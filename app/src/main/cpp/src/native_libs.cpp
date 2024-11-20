#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>

#include "FFLog.h"
#include "FFThread.h"
#include "FFCodecHandler.h"
#include "FFEglEnvironment.h"
#include "FFVideoPlay.h"

FFCodecHandler codecHandler;

void testThread(JNIEnv *env, jobject invoker) {
    FFVideoPlay videoPlay;
}

void setUrl(JNIEnv *env, jobject invoker, jstring url) {
    const char *path = env->GetStringUTFChars(url, JNI_FALSE);
    codecHandler.SetMediaPath(path);
    env->ReleaseStringUTFChars(url, path);
}

void init(JNIEnv *env, jobject invoker) {
    codecHandler.UnInitCodec();
    codecHandler.InitCodec();
    codecHandler.StartPlayVideo();
}

void release(JNIEnv *env, jobject invoker) {
    codecHandler.UnInitCodec();
}

void start(JNIEnv *env, jobject invoker) {

}

void pause(JNIEnv *env, jobject invoker) {

}

void stop(JNIEnv *env, jobject invoker) {

}

void seek(JNIEnv *env, jobject invoker, jfloat percent) {

}

void setDisplaySurface(JNIEnv *env, jobject invoker, jobject surface) {
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    //videoPlay.setDisplayWindow(window);
}

void removeDisplaySurface(JNIEnv *env, jobject invoker) {
    //videoPlay.removeDisplayWindow();
}

void resizeDisplaySurface(JNIEnv *env, jobject invoker, int w, int h) {
    //videoPlay.resizeDisplayWindow(w, h);
}

static JNINativeMethod methods[] = {
        {"testThread",           "()V",                                   (void *) testThread},
        {"init",                 "()V",                                   (void *) init},
        {"release",              "()V",                                   (void *) release},
        {"setUrl",               "(Ljava/lang/String;)V",                 (void *) setUrl},
        {"start",                "()V",                                   (void *) start},
        {"pause",                "()V",                                   (void *) pause},
        {"stop",                 "()V",                                   (void *) stop},
        {"seek",                 "(F)V",                                  (void *) seek},
        {"setDisplaySurface",    "(Landroid/view/Surface;)V",             (void *) setDisplaySurface},
        {"removeDisplaySurface", "()V",                                   (void *) removeDisplaySurface},
        {"resizeDisplaySurface", "(II)V",                                 (void *) resizeDisplaySurface},
};

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6)) {
        return JNI_ERR;
    }
    jclass playerClz = env->FindClass("com/example/ndkpractice/core/FFPlayer");
    if (playerClz == nullptr) {
        return JNI_ERR;
    }
    if (env->RegisterNatives(playerClz, methods, sizeof(methods) / sizeof(JNINativeMethod))) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {

}

