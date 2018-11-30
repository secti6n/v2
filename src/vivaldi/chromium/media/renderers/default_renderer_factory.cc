// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/renderers/default_renderer_factory.h"

#include <memory>
#include <utility>

#include "base/bind.h"
#include "media/base/decoder_factory.h"
#include "media/renderers/audio_renderer_impl.h"
#include "media/renderers/renderer_impl.h"
#include "media/renderers/video_renderer_impl.h"
#include "media/video/gpu_memory_buffer_video_frame_pool.h"
#include "media/video/gpu_video_accelerator_factories.h"

#if defined(USE_SYSTEM_PROPRIETARY_CODECS)
#include "platform_media/renderer/decoders/ipc_demuxer.h"
#include "platform_media/renderer/decoders/pass_through_audio_decoder.h"
#include "platform_media/renderer/decoders/pass_through_video_decoder.h"
#if defined(OS_MACOSX)
#include "platform_media/renderer/decoders/mac/at_audio_decoder.h"
#endif
#if defined(OS_WIN)
#include "platform_media/renderer/decoders/win/wmf_audio_decoder.h"
#include "platform_media/renderer/decoders/win/wmf_video_decoder.h"
#endif
#endif

namespace media {

DefaultRendererFactory::DefaultRendererFactory(
    MediaLog* media_log,
    DecoderFactory* decoder_factory,
    const GetGpuFactoriesCB& get_gpu_factories_cb)
    : media_log_(media_log),
      decoder_factory_(decoder_factory),
      get_gpu_factories_cb_(get_gpu_factories_cb) {
  DCHECK(decoder_factory_);
}

DefaultRendererFactory::~DefaultRendererFactory() = default;

std::vector<std::unique_ptr<AudioDecoder>>
DefaultRendererFactory::CreateAudioDecoders(
    const scoped_refptr<base::SingleThreadTaskRunner>& media_task_runner,
    bool use_platform_media_pipeline) {
  // Create our audio decoders and renderer.
  std::vector<std::unique_ptr<AudioDecoder>> audio_decoders;

#if defined(USE_SYSTEM_PROPRIETARY_CODECS)
  if (use_platform_media_pipeline) {
    audio_decoders.push_back(
        std::make_unique<PassThroughAudioDecoder>(media_task_runner));
  } else {
#if defined(OS_MACOSX)
    audio_decoders.push_back(
        std::make_unique<ATAudioDecoder>(media_task_runner));
#elif defined(OS_WIN)
    audio_decoders.push_back(
        std::make_unique<WMFAudioDecoder>(media_task_runner));
#endif
  }
#endif

  decoder_factory_->CreateAudioDecoders(media_task_runner, media_log_,
                                        &audio_decoders);
  return audio_decoders;
}

std::vector<std::unique_ptr<VideoDecoder>>
DefaultRendererFactory::CreateVideoDecoders(
    const scoped_refptr<base::SingleThreadTaskRunner>& media_task_runner,
    const RequestOverlayInfoCB& request_overlay_info_cb,
    const gfx::ColorSpace& target_color_space,
    GpuVideoAcceleratorFactories* gpu_factories,
    bool use_platform_media_pipeline) {
  // Create our video decoders and renderer.
  std::vector<std::unique_ptr<VideoDecoder>> video_decoders;

#if defined(USE_SYSTEM_PROPRIETARY_CODECS)
  if (use_platform_media_pipeline) {
    video_decoders.push_back(
        std::make_unique<PassThroughVideoDecoder>(media_task_runner));
  } else {
#endif

  // TODO(pgraszka): When chrome fixes the dropping frames issue in the
  // GpuVideoDecoder, we should make it our first choice on the list of video
  // decoders, for more details see: DNA-36050,
  // https://code.google.com/p/chromium/issues/detail?id=470466.
  decoder_factory_->CreateVideoDecoders(media_task_runner, gpu_factories,
                                        media_log_, request_overlay_info_cb,
                                        target_color_space, &video_decoders);

#if defined(USE_SYSTEM_PROPRIETARY_CODECS)
  }
#endif

#if defined(USE_SYSTEM_PROPRIETARY_CODECS)
#if defined(OS_WIN)
    video_decoders.push_back(std::make_unique<WMFVideoDecoder>(media_task_runner));
#endif
#endif

  return video_decoders;
}

std::unique_ptr<Renderer> DefaultRendererFactory::CreateRenderer(
    const scoped_refptr<base::SingleThreadTaskRunner>& media_task_runner,
    const scoped_refptr<base::TaskRunner>& worker_task_runner,
    AudioRendererSink* audio_renderer_sink,
    VideoRendererSink* video_renderer_sink,
    const RequestOverlayInfoCB& request_overlay_info_cb,
    const gfx::ColorSpace& target_color_space,
    bool use_platform_media_pipeline) {
  DCHECK(audio_renderer_sink);

  std::unique_ptr<AudioRenderer> audio_renderer(new AudioRendererImpl(
      media_task_runner, audio_renderer_sink,
      // Unretained is safe here, because the RendererFactory is guaranteed to
      // outlive the RendererImpl. The RendererImpl is destroyed when WMPI
      // destructor calls pipeline_controller_.Stop() -> PipelineImpl::Stop() ->
      // RendererWrapper::Stop -> RendererWrapper::DestroyRenderer(). And the
      // RendererFactory is owned by WMPI and gets called after WMPI destructor
      // finishes.
      base::Bind(&DefaultRendererFactory::CreateAudioDecoders,
                 base::Unretained(this), media_task_runner, use_platform_media_pipeline),
      media_log_));

  GpuVideoAcceleratorFactories* gpu_factories = nullptr;
  if (!get_gpu_factories_cb_.is_null())
    gpu_factories = get_gpu_factories_cb_.Run();

  std::unique_ptr<GpuMemoryBufferVideoFramePool> gmb_pool;
  if (gpu_factories && gpu_factories->ShouldUseGpuMemoryBuffersForVideoFrames(
                           false /* for_media_stream */)) {
    gmb_pool = std::make_unique<GpuMemoryBufferVideoFramePool>(
        std::move(media_task_runner), std::move(worker_task_runner),
        gpu_factories);
  }

  std::unique_ptr<VideoRenderer> video_renderer(new VideoRendererImpl(
      media_task_runner, video_renderer_sink,
      // Unretained is safe here, because the RendererFactory is guaranteed to
      // outlive the RendererImpl. The RendererImpl is destroyed when WMPI
      // destructor calls pipeline_controller_.Stop() -> PipelineImpl::Stop() ->
      // RendererWrapper::Stop -> RendererWrapper::DestroyRenderer(). And the
      // RendererFactory is owned by WMPI and gets called after WMPI destructor
      // finishes.
      base::Bind(&DefaultRendererFactory::CreateVideoDecoders,
                 base::Unretained(this), media_task_runner,
                 request_overlay_info_cb, target_color_space, gpu_factories, use_platform_media_pipeline),
      true, media_log_, std::move(gmb_pool)));

  return std::make_unique<RendererImpl>(
      media_task_runner, std::move(audio_renderer), std::move(video_renderer));
}

}  // namespace media