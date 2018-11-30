// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/profiling_host/background_profiling_triggers.h"

#include "base/stl_util.h"
#include "base/task_scheduler/post_task.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/metrics/chrome_metrics_service_accessor.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiling_host/profiling_process_host.h"
#include "components/heap_profiling/supervisor.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/process_type.h"
#include "services/resource_coordinator/public/cpp/memory_instrumentation/memory_instrumentation.h"

namespace heap_profiling {

namespace {

#if defined(OS_ANDROID)
// Check memory usage every 5 minutes.
const int kRepeatingCheckMemoryDelayInMinutes = 5;

const size_t kBrowserProcessMallocTriggerKb = 100 * 1024;    // 100 MB
const size_t kGPUProcessMallocTriggerKb = 40 * 1024;         // 40 MB
const size_t kRendererProcessMallocTriggerKb = 125 * 1024;   // 125 MB

// If memory usage has increased by 50MB since the last report, send another.
const uint32_t kHighWaterMarkThresholdKb = 50 * 1024;  // 50 MB
#else
// Check memory usage every 15 minutes.
const int kRepeatingCheckMemoryDelayInMinutes = 15;

const size_t kBrowserProcessMallocTriggerKb = 400 * 1024;    // 400 MB
const size_t kGPUProcessMallocTriggerKb = 400 * 1024;        // 400 MB
const size_t kRendererProcessMallocTriggerKb = 500 * 1024;   // 500 MB

// If memory usage has increased by 500MB since the last report, send another.
const uint32_t kHighWaterMarkThresholdKb = 500 * 1024;  // 500 MB
#endif  // OS_ANDROID

int GetContentProcessType(
    const memory_instrumentation::mojom::ProcessType& type) {
  using memory_instrumentation::mojom::ProcessType;

  switch (type) {
    case ProcessType::BROWSER:
      return content::ProcessType::PROCESS_TYPE_BROWSER;

    case ProcessType::RENDERER:
      return content::ProcessType::PROCESS_TYPE_RENDERER;

    case ProcessType::GPU:
      return content::ProcessType::PROCESS_TYPE_GPU;

    case ProcessType::UTILITY:
      return content::ProcessType::PROCESS_TYPE_UTILITY;

    case ProcessType::PLUGIN:
      return content::ProcessType::PROCESS_TYPE_PLUGIN_DEPRECATED;

    case ProcessType::OTHER:
      return content::ProcessType::PROCESS_TYPE_UNKNOWN;
  }

  NOTREACHED();
  return content::ProcessType::PROCESS_TYPE_UNKNOWN;
}

}  // namespace

BackgroundProfilingTriggers::BackgroundProfilingTriggers(
    ProfilingProcessHost* host)
    : host_(host), weak_ptr_factory_(this) {
  DCHECK(host_);
}

BackgroundProfilingTriggers::~BackgroundProfilingTriggers() {}

void BackgroundProfilingTriggers::StartTimer() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  // Register a repeating timer to check memory usage periodically.
  timer_.Start(
      FROM_HERE,
      base::TimeDelta::FromMinutes(kRepeatingCheckMemoryDelayInMinutes),
      base::BindRepeating(
          &BackgroundProfilingTriggers::PerformMemoryUsageChecks,
          weak_ptr_factory_.GetWeakPtr()));
}

bool BackgroundProfilingTriggers::IsAllowedToUpload() const {
  return ChromeMetricsServiceAccessor::IsMetricsAndCrashReportingEnabled();
}

bool BackgroundProfilingTriggers::IsOverTriggerThreshold(
    int content_process_type,
    uint32_t private_footprint_kb) {
  switch (content_process_type) {
    case content::ProcessType::PROCESS_TYPE_BROWSER:
      return private_footprint_kb > kBrowserProcessMallocTriggerKb;

    case content::ProcessType::PROCESS_TYPE_GPU:
      return private_footprint_kb > kGPUProcessMallocTriggerKb;

    case content::ProcessType::PROCESS_TYPE_RENDERER:
      return private_footprint_kb > kRendererProcessMallocTriggerKb;

    default:
      return false;
  }
}

void BackgroundProfilingTriggers::PerformMemoryUsageChecks() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  if (!IsAllowedToUpload()) {
    return;
  }

  auto callback = base::BindOnce(
      [](base::WeakPtr<BackgroundProfilingTriggers> weak_ptr,
         std::vector<base::ProcessId> result) {
        memory_instrumentation::MemoryInstrumentation::GetInstance()
            ->RequestPrivateMemoryFootprint(
                base::kNullProcessId,
                base::Bind(&BackgroundProfilingTriggers::OnReceivedMemoryDump,
                           std::move(weak_ptr), std::move(result)));
      },
      weak_ptr_factory_.GetWeakPtr());
  Supervisor::GetInstance()->GetProfiledPids(std::move(callback));
}

void BackgroundProfilingTriggers::OnReceivedMemoryDump(
    std::vector<base::ProcessId> profiled_pids,
    bool success,
    std::unique_ptr<memory_instrumentation::GlobalMemoryDump> dump) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  if (!success) {
    return;
  }

  bool should_send_report = false;
  for (const auto& proc : dump->process_dumps()) {
    if (!base::ContainsValue(profiled_pids, proc.pid()))
      continue;

    uint32_t private_footprint_kb = proc.os_dump().private_footprint_kb;
    auto it = pmf_at_last_upload_.find(proc.pid());
    if (it != pmf_at_last_upload_.end()) {
      if (private_footprint_kb > it->second + kHighWaterMarkThresholdKb) {
        should_send_report = true;
        it->second = private_footprint_kb;
      }
      continue;
    }

    // No high water mark exists yet, check the trigger threshold.
    if (IsOverTriggerThreshold(GetContentProcessType(proc.process_type()),
                               private_footprint_kb)) {
      should_send_report = true;
      pmf_at_last_upload_[proc.pid()] = private_footprint_kb;
    }
  }

  if (should_send_report) {
    // Clear the watermark for all non-profiled pids.
    for (auto it = pmf_at_last_upload_.begin();
         it != pmf_at_last_upload_.end();) {
      if (base::ContainsValue(profiled_pids, it->first)) {
        ++it;
      } else {
        it = pmf_at_last_upload_.erase(it);
      }
    }

    TriggerMemoryReport();
  }
}

void BackgroundProfilingTriggers::TriggerMemoryReport() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  host_->RequestProcessReport("MEMLOG_BACKGROUND_TRIGGER");
}

}  // namespace heap_profiling