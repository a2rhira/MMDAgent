#include <jni.h>
/* DO NOT EDIT THIS FILE - it is machine generated */
/* Header for class TomeiTestJNI */

#ifndef _Included_MainNativeActivity
#define _Included_MainNativeActivity
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Java_com_example_mmdagent_MainNativeActivity_onStartToJNI
( JNIEnv* env, jobject thiz);

JNIEXPORT jint JNICALL Java_com_example_mmdagent_MainNativeActivity_onStopToJNI
( JNIEnv* env, jobject thiz);

JNIEXPORT jint JNICALL Java_com_example_mmdagent_MainNativeActivity_sendMessageToJNI
( JNIEnv* env, jobject thiz, jstring srcj);

JNIEXPORT jobject JNICALL Java_com_example_mmdagent_MainNativeActivity_receiveMessageFromJNI
( JNIEnv* env, jobject thiz);

#ifdef __cplusplus
}
#endif
#endif
