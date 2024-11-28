#include <jni.h>
#include <string>

#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <memory>
#include <functional>

#include "FFLog.h"
#include "FFThread.h"
#include "FFPlayer.h"
#include "FFEglEnvironment.h"
#include "FFVideoRender.h"
#include "PcmTest.h"

extern "C" {
#include "libavcodec/jni.h"
}

#define TAG "native-lib"

std::mutex actionMutex;

FFPlayer* getFFCodecHandler(JNIEnv *env, jobject invoker) {
    jclass clz = env->GetObjectClass(invoker);
    jfieldID fieldId = env->GetFieldID(clz, "nativeHandle", "J");
    jlong handle = env->GetLongField(invoker, fieldId);
    return reinterpret_cast<FFPlayer*>(handle);
}

void performAction(JNIEnv *env, jobject invoker, const char* action, std::function<void(FFPlayer*)> run) {
    std::lock_guard<std::mutex> lock(actionMutex);
    FFPlayer* handler = getFFCodecHandler(env, invoker);
    if (handler) {
        run(handler);
    } else {
        LOGE(TAG, "jni perform %s failed, player released already.", action);
    }
}

void initGlobal(JNIEnv *env, jobject invoker) {
    av_register_all();
    avformat_network_init();

    testDemux();
}

void setupNative(JNIEnv *env, jobject invoker) {
    FFPlayer* player = new FFPlayer();
    jclass clz = env->GetObjectClass(invoker);
    jfieldID fieldId = env->GetFieldID(clz, "nativeHandle", "J");
    env->SetLongField(invoker, fieldId, reinterpret_cast<long>(player));
}

void setUrl(JNIEnv *env, jobject invoker, jstring url) {
    performAction(env, invoker, "setUrl", [&](FFPlayer* handler){
        const char *path = env->GetStringUTFChars(url, JNI_FALSE);
        handler->SetMediaPath(path);
        env->ReleaseStringUTFChars(url, path);
    });
}

void prepare(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "prepare", [](FFPlayer* handler){
        handler->InitCodec();
    });
}

void start(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "start", [](FFPlayer* handler){
        handler->StartPlayVideo();
    });
}

void pause(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "start", [](FFPlayer* handler){
        handler->PausePlayVideo();
    });
}

void stop(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "stop", [](FFPlayer* handler){
        handler->StopPlayVideo();
    });
}

void seek(JNIEnv *env, jobject invoker, jfloat percent) {
    performAction(env, invoker, "seek", [percent](FFPlayer* handler){
        handler->Seek(percent);
    });
}

void setDisplaySurface(JNIEnv *env, jobject invoker, jobject surface) {
    performAction(env, invoker, "setDisplaySurface", [&](FFPlayer* handler){
        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
        handler->videoRender.setDisplayWindow(window);
    });
}

void removeDisplaySurface(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "removeDisplaySurface", [](FFPlayer* handler){
        handler->videoRender.removeDisplayWindow();
    });
}

void resizeDisplaySurface(JNIEnv *env, jobject invoker, int w, int h) {
    performAction(env, invoker, "removeDisplaySurface", [w, h](FFPlayer* handler){
        handler->videoRender.resizeDisplayWindow(w, h);
    });
}

void release(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "removeDisplaySurface", [&](FFPlayer* handler){
        handler->UnInitCodec();
        delete handler;

        jclass clz = env->GetObjectClass(invoker);
        jfieldID fieldId = env->GetFieldID(clz, "nativeHandle", "J");
        env->SetLongField(invoker, fieldId, 0L);
    });
}

static JNINativeMethod methods[] = {
        {"_initGlobal",           "()V",                                   (void *) initGlobal},
        {"_setupNative",           "()V",                                   (void *) setupNative},
        {"_prepare",              "()V",                                   (void *) prepare},
        {"_release",              "()V",                                   (void *) release},
        {"_setDataSource",               "(Ljava/lang/String;)V",                 (void *) setUrl},
        {"_start",                "()V",                                   (void *) start},
        {"_pause",                "()V",                                   (void *) pause},
        {"_stop",                 "()V",                                   (void *) stop},
        {"_seek",                 "(F)V",                                  (void *) seek},
        {"_setDisplaySurface",    "(Landroid/view/Surface;)V",             (void *) setDisplaySurface},
        {"_removeDisplaySurface", "()V",                                   (void *) removeDisplaySurface},
        {"_resizeDisplaySurface", "(II)V",                                 (void *) resizeDisplaySurface},
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
    av_jni_set_java_vm(vm, nullptr);
    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {

}

