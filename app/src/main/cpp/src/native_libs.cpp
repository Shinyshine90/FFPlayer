#include <jni.h>
#include <string>

#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <unordered_map>
#include <memory>
#include <functional>

#include "FFLog.h"
#include "FFThread.h"
#include "FFCodecHandler.h"
#include "FFEglEnvironment.h"
#include "FFVideoRender.h"
#include "PcmTest.h"

extern "C" {
#include "libavcodec/jni.h"
}

FFCodecHandler* getFFCodecHandler(JNIEnv *env, jobject invoker) {
    jclass clz = env->GetObjectClass(invoker);
    jfieldID fieldId = env->GetFieldID(clz, "nativeHandle", "J");
    jlong handle = env->GetLongField(invoker, fieldId);
    return reinterpret_cast<FFCodecHandler*>(handle);
}

void performAction(JNIEnv *env, jobject invoker, const char* action, std::function<void(FFCodecHandler*)> run) {
    FFCodecHandler* handler = getFFCodecHandler(env, invoker);
    if (handler) {
        run(handler);
    } else {
        LOGE("jni perform %s failed, player released already.", action);
    }
}

void initGlobal(JNIEnv *env, jobject invoker) {
    av_register_all();
    avformat_network_init();

    //sdcard/out.pcm
    //testPcm();
}

void setupNative(JNIEnv *env, jobject invoker) {
    FFCodecHandler* player = new FFCodecHandler();
    jclass clz = env->GetObjectClass(invoker);
    jfieldID fieldId = env->GetFieldID(clz, "nativeHandle", "J");
    env->SetLongField(invoker, fieldId, reinterpret_cast<long>(player));
}

void setUrl(JNIEnv *env, jobject invoker, jstring url) {
    performAction(env, invoker, "setUrl", [&](FFCodecHandler* handler){
        const char *path = env->GetStringUTFChars(url, JNI_FALSE);
        handler->SetMediaPath(path);
        env->ReleaseStringUTFChars(url, path);
    });
}

void prepare(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "prepare", [](FFCodecHandler* handler){
        handler->InitCodec();
    });
}

void start(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "start", [](FFCodecHandler* handler){
        handler->StartPlayVideo();
    });
}

void pause(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "start", [](FFCodecHandler* handler){
        handler->PausePlayVideo();
    });
}

void stop(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "stop", [](FFCodecHandler* handler){
        handler->StopPlayVideo();
    });
}

void seek(JNIEnv *env, jobject invoker, jfloat percent) {
    performAction(env, invoker, "seek", [percent](FFCodecHandler* handler){
        handler->Seek(percent);
    });
}

void setDisplaySurface(JNIEnv *env, jobject invoker, jobject surface) {
    performAction(env, invoker, "setDisplaySurface", [&](FFCodecHandler* handler){
        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
        handler->videoRender.setDisplayWindow(window);
    });
}

void removeDisplaySurface(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "removeDisplaySurface", [](FFCodecHandler* handler){
        handler->videoRender.removeDisplayWindow();
    });
}

void resizeDisplaySurface(JNIEnv *env, jobject invoker, int w, int h) {
    performAction(env, invoker, "removeDisplaySurface", [w, h](FFCodecHandler* handler){
        handler->videoRender.resizeDisplayWindow(w, h);
    });
}

void release(JNIEnv *env, jobject invoker) {
    performAction(env, invoker, "removeDisplaySurface", [&](FFCodecHandler* handler){
        handler->UnInitCodec();
        delete handler;

        jclass clz = env->GetObjectClass(invoker);
        jfieldID fieldId = env->GetFieldID(clz, "nativeHandle", "J");
        env->SetLongField(invoker, fieldId, 0L);
    });
}

static JNINativeMethod methods[] = {
        {"initGlobal",           "()V",                                   (void *) initGlobal},
        {"setupNative",           "()V",                                   (void *) setupNative},
        {"prepare",              "()V",                                   (void *) prepare},
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
    av_jni_set_java_vm(vm, nullptr);
    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {

}

