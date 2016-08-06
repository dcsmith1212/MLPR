//
// Created by derek on 8/4/16.
//

#include <jni.h>
#include "derek_licrecndk4_MainActivity.h"

#include <opencv2/core/core.hpp>

JNIEXPORT jstring JNICALL Java_derek_licrecndk4_MainActivity_hello
  (JNIEnv * env, jobject obj){
    return (*env)->NewStringUTF(env, "Hello from C++ file!");
  }