// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/device/bluetooth/le/le_scan_manager_impl.h"

#include <algorithm>
#include <utility>

#include "base/stl_util.h"
#include "chromecast/base/bind_to_task_runner.h"
#include "chromecast/device/bluetooth/bluetooth_util.h"
#include "chromecast/public/cast_media_shlib.h"

#define RUN_ON_IO_THREAD(method, ...)                       \
  io_task_runner_->PostTask(                                \
      FROM_HERE, base::BindOnce(&LeScanManagerImpl::method, \
                                weak_factory_.GetWeakPtr(), ##__VA_ARGS__));

#define MAKE_SURE_IO_THREAD(method, ...)            \
  DCHECK(io_task_runner_);                          \
  if (!io_task_runner_->BelongsToCurrentThread()) { \
    RUN_ON_IO_THREAD(method, ##__VA_ARGS__)         \
    return;                                         \
  }

#define EXEC_CB_AND_RET(cb, ret, ...)        \
  do {                                       \
    if (cb) {                                \
      std::move(cb).Run(ret, ##__VA_ARGS__); \
    }                                        \
    return;                                  \
  } while (0)

namespace chromecast {
namespace bluetooth {

namespace {

const int kMaxMessagesInQueue = 5;

}  // namespace

class LeScanManagerImpl::ScanHandleImpl : public LeScanManager::ScanHandle {
 public:
  explicit ScanHandleImpl(LeScanManagerImpl* manager, int32_t id)
      : on_destroyed_(BindToCurrentSequence(
            base::BindOnce(&LeScanManagerImpl::NotifyScanHandleDestroyed,
                           manager->weak_factory_.GetWeakPtr(),
                           id))) {}
  ~ScanHandleImpl() override { std::move(on_destroyed_).Run(); }

 private:
  base::OnceClosure on_destroyed_;
};

LeScanManagerImpl::LeScanManagerImpl(
    bluetooth_v2_shlib::LeScannerImpl* le_scanner)
    : le_scanner_(le_scanner),
      observers_(new base::ObserverListThreadSafe<Observer>()),
      weak_factory_(this) {}

LeScanManagerImpl::~LeScanManagerImpl() = default;

void LeScanManagerImpl::Initialize(
    scoped_refptr<base::SingleThreadTaskRunner> io_task_runner) {
  io_task_runner_ = std::move(io_task_runner);
}

void LeScanManagerImpl::Finalize() {}

void LeScanManagerImpl::AddObserver(Observer* observer) {
  observers_->AddObserver(observer);
}

void LeScanManagerImpl::RemoveObserver(Observer* observer) {
  observers_->RemoveObserver(observer);
}

void LeScanManagerImpl::RequestScan(RequestScanCallback cb) {
  MAKE_SURE_IO_THREAD(RequestScan, BindToCurrentSequence(std::move(cb)));
  if (scan_handle_ids_.empty()) {
    if (!le_scanner_->StartScan()) {
      LOG(ERROR) << "Failed to enable scanning";
      std::move(cb).Run(nullptr);
      return;
    }
    observers_->Notify(FROM_HERE, &Observer::OnScanEnableChanged, true);
  }

  int32_t id = next_scan_handle_id_++;
  auto handle = std::make_unique<ScanHandleImpl>(this, id);
  scan_handle_ids_.insert(id);

  std::move(cb).Run(std::move(handle));
}

void LeScanManagerImpl::GetScanResults(GetScanResultsCallback cb,
                                       base::Optional<ScanFilter> scan_filter) {
  MAKE_SURE_IO_THREAD(GetScanResults, BindToCurrentSequence(std::move(cb)),
                      std::move(scan_filter));
  std::move(cb).Run(GetScanResultsInternal(std::move(scan_filter)));
}

void LeScanManagerImpl::ClearScanResults() {
  MAKE_SURE_IO_THREAD(ClearScanResults);
  addr_to_scan_results_.clear();
}

void LeScanManagerImpl::OnScanResult(
    const bluetooth_v2_shlib::LeScanner::ScanResult& scan_result_shlib) {
  LeScanResult scan_result;
  if (!scan_result.SetAdvData(scan_result_shlib.adv_data)) {
    // Error logged.
    return;
  }
  scan_result.addr = scan_result_shlib.addr;
  scan_result.rssi = scan_result_shlib.rssi;

  // Remove results with the same data as the current result to avoid duplicate
  // messages in the queue
  auto& previous_scan_results = addr_to_scan_results_[scan_result.addr];
  previous_scan_results.remove_if([&scan_result](const auto& previous_result) {
    return previous_result.adv_data == scan_result.adv_data;
  });

  previous_scan_results.push_front(scan_result);
  if (previous_scan_results.size() > kMaxMessagesInQueue) {
    previous_scan_results.pop_back();
  }

  // Update observers.
  observers_->Notify(FROM_HERE, &Observer::OnNewScanResult, scan_result);
}

// Returns a list of all scan results. The results are sorted by RSSI.
std::vector<LeScanResult> LeScanManagerImpl::GetScanResultsInternal(
    base::Optional<ScanFilter> scan_filter) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  std::vector<LeScanResult> results;
  for (const auto& pair : addr_to_scan_results_) {
    for (const auto& scan_result : pair.second) {
      if (!scan_filter || scan_filter->Matches(scan_result)) {
        results.push_back(scan_result);
      }
    }
  }

  std::sort(results.begin(), results.end(),
            [](const LeScanResult& d1, const LeScanResult& d2) {
              return d1.rssi > d2.rssi;
            });

  return results;
}

void LeScanManagerImpl::NotifyScanHandleDestroyed(int32_t id) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());

  size_t num_removed = scan_handle_ids_.erase(id);
  DCHECK_EQ(num_removed, 1u);
  if (scan_handle_ids_.empty()) {
    if (!le_scanner_->StopScan()) {
      LOG(ERROR) << "Failed to disable scanning";
    } else {
      observers_->Notify(FROM_HERE, &Observer::OnScanEnableChanged, false);
    }
  }
}

}  // namespace bluetooth
}  // namespace chromecast