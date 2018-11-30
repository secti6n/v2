// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.crash.browser;

import org.chromium.base.Log;
import org.chromium.base.ThreadUtils;
import org.chromium.base.annotations.CalledByNative;

import java.io.File;

/**
 * A Java-side bridge for continuing the upload processing for child process crash minidumps.
 */
public class CrashDumpManager {
    private static final String TAG = "CrashDumpManager";

    /**
     * An interface for providing a callback that will try to upload a minidump. The callback should
     * be registered on, and will be run on, the UI thread.
     */
    public interface UploadMinidumpCallback { public void tryToUploadMinidump(File minidump); }

    /**
     * The globally registered callback for uploading minidumps, or null if no callback has been
     * registered yet.
     */
    private static UploadMinidumpCallback sCallback;

    /**
     * Registers a callback for uploading minidumps. May be called at most once, and only on the UI
     * thread.
     *
     * @param callback The callback to trigger when a new minidump is generated by a child process.
     */
    public static void registerUploadCallback(UploadMinidumpCallback callback) {
        ThreadUtils.assertOnUiThread();
        assert sCallback == null;
        sCallback = callback;
    }

    /**
     * Attempts to upload the specified child process minidump (or a no-op if no observer has been
     * registered).
     *
     * @param minidumpPath The file path for the generated minidump.
     */
    @CalledByNative
    public static void tryToUploadMinidump(String minidumpPath) {
        // The C++ code that calls into this method should be running on a background thread. It's
        // important to be off the UI thread for the file operations done below.
        ThreadUtils.assertOnBackgroundThread();

        if (minidumpPath == null) {
            Log.e(TAG, "Minidump path should be non-null! Bailing...");
            return;
        }

        final File minidump = new File(minidumpPath);
        if (!minidump.exists() || !minidump.isFile()) {
            Log.e(TAG,
                    "Minidump path '" + minidumpPath
                            + "' should describe a file that exists! Bailing...");
            return;
        }

        // The callback should only be accessed on the UI thread.
        ThreadUtils.postOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (sCallback == null) {
                    Log.w(TAG, "Ignoring crash observed before a callback was registered...");
                    return;
                }
                sCallback.tryToUploadMinidump(minidump);
            }
        });
    }
}