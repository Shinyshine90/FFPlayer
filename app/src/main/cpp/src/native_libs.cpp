#include <jni.h>
#include <android/asset_manager_jni.h>
#include <string>

#include "FFCodecHandler.h"
#include "FFLog.h"

FFCodecHandler codecHandler;

void glInit(JNIEnv *env, jobject invoker, jobject assetManagerObj) {
    AAssetManager *assetManager = AAssetManager_fromJava(env, assetManagerObj);

}

void glResize(JNIEnv *env, jobject invoker, jint width, jint height) {

}

void glDraw(JNIEnv *env, jobject invoker) {

}

void setUrl(JNIEnv *env, jobject invoker, jstring url) {
    const char* path = env->GetStringUTFChars(url, JNI_FALSE);
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

static JNINativeMethod methods[] = {
        {"glInit",   "(Landroid/content/res/AssetManager;)V", (void *) glInit},
        {"glResize", "(II)V",                                 (void *) glResize},
        {"glDraw",   "()V",                                   (void *) glDraw},

        {"init",     "()V",                                   (void *) init},
        {"release",  "()V",                                   (void *) release},
        {"setUrl",   "(Ljava/lang/String;)V",                 (void *) setUrl},
        {"start",    "()V",                                   (void *) start},
        {"pause",    "()V",                                   (void *) pause},
        {"stop",     "()V",                                   (void *) stop},
        {"seek",     "(F)V",                                  (void *) seek},
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
