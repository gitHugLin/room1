#include <jni.h>
#include "string.h"
#include "assert.h"
#include "log.h"

#ifndef MY_JNI_MYJNI_H
#define MY_JNI_MYJNI_H
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL updateTexture
(JNIEnv *, jobject,jlong);

JNIEXPORT void JNICALL grayImage
        (JNIEnv *, jobject,jobjectArray,jintArray,jint,jint);

JNIEXPORT jlong JNICALL addPicture
        (JNIEnv *env, jobject obj);

JNIEXPORT jboolean JNICALL gray
        (JNIEnv *env, jobject obj,jintArray,jint,jint);

JNIEXPORT jint JNICALL getRunTime
        (JNIEnv *env, jobject obj);

JNIEXPORT void JNICALL init(JNIEnv * env, jobject obj,  jint width, jint height);
JNIEXPORT void JNICALL step(JNIEnv * env, jobject obj);

#ifdef __cplusplus
}
#endif

#endif //MY_JNI_MYJNI_H
