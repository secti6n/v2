// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/variations/variations_url_constants.h"

#include "sync/vivaldi_sync_urls.h"

namespace variations {

// Default server of Variations seed info.
const char kDefaultServerUrl[] =
    SYNC_URL("/chrome-variations/seed");

const char kDefaultInsecureServerUrl[] =
    "http://clientservices.googleapis.com/chrome-variations/seed";

}  // namespace variations