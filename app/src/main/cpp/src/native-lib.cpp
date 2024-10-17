#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "ALog.h"

#include "libavcodec/avcodec.h"

#include "IPlayer.h"
#include "FFPlayBuilder.h"
#include "XThread.h"

static JavaVM *globalVm = nullptr;
static jobject globalPlayerObj = nullptr;

static FFPlayBuilder builder;
static IPlayer *player = builder.build(0);

jstring getFFPlayerVersionInfo(JNIEnv *env, jobject obj) {
    return env->NewStringUTF("");
}

void onPlayPositionChanged(double position) {
    if (!globalVm) {
        return;
    }
    JNIEnv *env;
    if (globalVm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        LOGE("onPlayPositionChanged acquire jni env failed.");
        globalVm->DetachCurrentThread();
        return;
    }
    if (!globalPlayerObj) {
        return;
    }
    jclass clz = env->GetObjectClass(globalPlayerObj);
    if (!clz) {
        LOGE("onPlayPositionChanged acquire clz failed.");
        globalVm->DetachCurrentThread();
        return;
    }
    jmethodID mId = env->GetMethodID(clz, "onPlayPositionChanged", "(D)V");
    if (!mId) {
        LOGE("onPlayPositionChanged method not found.");
        globalVm->DetachCurrentThread();
        return;
    }
    env->CallVoidMethod(globalPlayerObj, mId, position);

    globalVm->DetachCurrentThread();
}

bool startPlay(JNIEnv *env, jobject obj, jstring url) {
    const char *urlPath = env->GetStringUTFChars(url, JNI_FALSE);

    env->DeleteGlobalRef(globalPlayerObj);
    globalPlayerObj = env->NewGlobalRef(obj);

    player->open(urlPath);
    player->start();
    player->callback = onPlayPositionChanged;
    env->ReleaseStringUTFChars(url, urlPath);
    return true;
}

bool setDisplayWindow(JNIEnv *env, jobject obj, jobject surface) {
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    int width = ANativeWindow_getWidth(window);
    int height = ANativeWindow_getHeight(window);

    player->setSurface(window);
    return true;
}

bool removeDisplayWindow(JNIEnv *env, jobject obj, jobject surface) {
    LOGI("remove egl window ret %d.");
    return false;
}

bool seek(JNIEnv *env, jobject obj, double position) {
    return player->seek(position);
}

void pause(JNIEnv *env, jobject obj) {
    if (!player->isPause())
        player->pause();
}

void resume(JNIEnv *env, jobject obj) {
    if (player->isPause())
        player->resume();
}

bool isPause(JNIEnv *env, jobject obj) {
    return player->isPause();
}


static JNINativeMethod nativeMethods[]{
        {"getPlayerVersion",     "()Ljava/lang/String;",      (void *) getFFPlayerVersionInfo},
        {"setDisplaySurface",    "(Landroid/view/Surface;)Z", (void *) setDisplayWindow},
        {"removeDisplaySurface", "(Landroid/view/Surface;)Z", (void *) removeDisplayWindow},
        {"startPlay",            "(Ljava/lang/String;)Z",     (void *) startPlay},
        {"seek",                 "(D)Z",                      (void *) seek},
        {"pause",                "()V",                       (void *) pause},
        {"resume",               "()V",                       (void *) resume},
        {"isPaused",             "()Z",                       (void *) isPause},
};

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    if (!globalVm) {
        globalVm = vm;
    }
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("JNI_OnLoad get env failed.");
        return JNI_ERR;
    }
    jclass playerClz = env->FindClass("com/example/ndkpractice/core/FFPlayer");
    if (playerClz == nullptr) {
        LOGE("JNI_OnLoad find clz failed.");
        return JNI_ERR;
    }
    if (env->RegisterNatives(playerClz, nativeMethods,
                             sizeof(nativeMethods) / sizeof(JNINativeMethod)) != JNI_OK) {
        LOGE("JNI_OnLoad RegisterNatives failed.");
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
    globalVm = nullptr;
}