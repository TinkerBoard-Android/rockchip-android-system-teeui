/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.android.framebufferizer;

import com.android.framebufferizer.utils.DeviceInfo;

public class NativeRenderer {
    static {
         System.loadLibrary("teeui_jni");
    }

    public static native int setDeviceInfo(DeviceInfo deviceInfo, boolean magnified, boolean inverted, boolean touchLayout);
    public static native int renderBuffer(int x, int y, int width, int height, int lineStride, int[] buffer);
    public static native void setLanguage(String language_id);
    public static native String[] getLanguageIdList();
    public static native void setConfimationMessage(String confimationMessage);
    public static native int onEvent(int x, int y, int event);
}
