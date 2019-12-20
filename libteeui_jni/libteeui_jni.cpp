/*
 *
 * Copyright 2019, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jni.h>

/*
 * JTypeTraits provides hints for JArray on how to access and free the array elements and how
 * to get the array size. The traits allow JArray to be used with jbyteArray, jintArray,
 * and jstring.
 */
template <typename T> struct JTypeTraits;

template <> struct JTypeTraits<jintArray> {
    using element_type = jint;
    using array_type = jintArray;

    static constexpr element_type* (JNIEnv::*const getArrayElements)(array_type, jboolean*) =
        &JNIEnv::GetIntArrayElements;
    static constexpr void (JNIEnv::*const releaseArrayElements)(array_type, element_type*, jint) =
        &JNIEnv::ReleaseIntArrayElements;
    static constexpr jsize (JNIEnv::*const getArrayLength)(jarray) = &JNIEnv::GetArrayLength;
};

template <> struct JTypeTraits<jbyteArray> {
    using element_type = jbyte;
    using array_type = jbyteArray;

    static constexpr element_type* (JNIEnv::*const getArrayElements)(array_type, jboolean*) =
        &JNIEnv::GetByteArrayElements;
    static constexpr void (JNIEnv::*const releaseArrayElements)(array_type, element_type*, jint) =
        &JNIEnv::ReleaseByteArrayElements;
    static constexpr jsize (JNIEnv::*const getArrayLength)(jarray) = &JNIEnv::GetArrayLength;
};

template <> struct JTypeTraits<jstring> {
    using element_type = const char;
    using array_type = jstring;

    static constexpr element_type* (JNIEnv::*const getArrayElements)(array_type, jboolean*) =
        &JNIEnv::GetStringUTFChars;
    static constexpr void (JNIEnv::*const releaseArrayElements)(array_type, element_type*) =
        &JNIEnv::ReleaseStringUTFChars;
    static constexpr jsize (JNIEnv::*const getArrayLength)(array_type) =
        &JNIEnv::GetStringUTFLength;
};

/*
 * JArray can wrap various types of primitive Java arrays through a common typical C++ interface.
 * It tracks ownership. It can only be moved but not copied. It provides iterators through begin()
 * and end() so that it can be used in range for loops.
 */
template <typename T> class JArray {
  private:
    JNIEnv* env_;
    using element_type = typename JTypeTraits<T>::element_type;
    using array_type = typename JTypeTraits<T>::array_type;
    array_type jarray_;
    element_type* buffer_;

  public:
    JArray(JNIEnv* env, array_type jarray) : env_(env), jarray_(jarray), buffer_(nullptr) {
        if (env_ == nullptr) return;
        buffer_ = (env_->*JTypeTraits<T>::getArrayElements)(jarray, nullptr);
    }
    JArray(const JArray&) = delete;
    JArray(JArray&& rhs) : env_(rhs.env_), jarray_(rhs.jarray_), buffer_(rhs.buffer_) {
        rhs.env_ = nullptr;
        rhs.buffer_ = nullptr;
    }
    ~JArray();

    JArray& operator=(const JArray&) = delete;
    JArray& operator=(JArray&& rhs) {
        if (&rhs != this) {
            env_ = rhs.env_;
            jarray_ = rhs.jarray_;
            buffer_ = rhs.buffer_;
            rhs.env_ = nullptr;
            rhs.buffer_ = nullptr;
        }
        return *this;
    }

    operator bool() const { return buffer_ != nullptr; }
    const jint& operator[](size_t offset) const { return buffer_[offset]; }
    element_type& operator[](size_t offset) { return buffer_[offset]; }
    size_t size() const { return (env_->*JTypeTraits<T>::getArrayLength)(jarray_); }
    element_type* begin() { return buffer_; }
    element_type* end() { return buffer_ + size(); }
    const element_type* begin() const { return buffer_; }
    const element_type* end() const { return buffer_ + size(); }
};

template <typename T> JArray<T>::~JArray() {
    if (env_ == nullptr) return;
    if (buffer_ == nullptr) return;
    (env_->*JTypeTraits<T>::releaseArrayElements)(jarray_, buffer_, 0);
}

template <> JArray<jstring>::~JArray() {
    if (env_ == nullptr) return;
    if (buffer_ == nullptr) return;
    (env_->*JTypeTraits<jstring>::releaseArrayElements)(jarray_, buffer_);
}

using JIntArray = JArray<jintArray>;
using JByteArray = JArray<jbyteArray>;
using JString = JArray<jstring>;

/*
 * Class:     com_android_framebufferizer_NativeRenderer
 * Method:    setDeviceInfo
 * Signature: (IIIDD)I
 */
extern "C" JNIEXPORT jint JNICALL Java_com_android_framebufferizer_NativeRenderer_setDeviceInfo(
    JNIEnv*, jclass, jint width, jint height, jint colormodel, jdouble dp2px, jdouble mm2px) {
    return setDeviceInfo(width, height, colormodel, dp2px, mm2px);
}

/*
 * Class:     com_android_framebufferizer_NativeRenderer
 * Method:    renderBuffer
 * Signature: (IIIII[I)I
 */
extern "C" JNIEXPORT jint JNICALL Java_com_android_framebufferizer_NativeRenderer_renderBuffer(
    JNIEnv* env, jclass, jint x, jint y, jint width, jint height, jint lineStride,
    jintArray jbuffer) {
    JIntArray buffer(env, jbuffer);
    if (!buffer) return 1;
    return renderUIIntoBuffer((uint32_t)x, (uint32_t)y, (uint32_t)width, (uint32_t)height,
                              (uint32_t)lineStride, (uint32_t*)buffer.begin(), buffer.size());
}