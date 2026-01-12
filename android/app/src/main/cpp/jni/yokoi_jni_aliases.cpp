#include <jni.h>

// Forward declarations: existing entrypoints still live in yokoi_jni.cpp.
extern "C" {
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetAssetManager(JNIEnv* env, jclass clazz, jobject assetManager);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetStorageRoot(JNIEnv* env, jclass clazz, jstring path);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeInit(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeShutdown(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeStartAaudio(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeStopAaudio(JNIEnv* env, jclass clazz);
JNIEXPORT jobjectArray JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeGetTextureAssetNames(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetTextures(
        JNIEnv* env, jclass clazz,
        jint segmentTex, jint segmentW, jint segmentH,
        jint backgroundTex, jint backgroundW, jint backgroundH,
        jint consoleTex, jint consoleW, jint consoleH);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetUiTexture(JNIEnv* env, jclass clazz, jint uiTex, jint uiW, jint uiH);
JNIEXPORT jint JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeGetAppMode(JNIEnv* env, jclass clazz);
JNIEXPORT jint JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeGetMenuLoadChoice(JNIEnv* env, jclass clazz);
JNIEXPORT jboolean JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeMenuHasSaveState(JNIEnv* env, jclass clazz);
JNIEXPORT jobjectArray JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeGetSelectedGameInfo(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeAutoSaveState(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeReturnToMenu(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetPaused(JNIEnv* env, jclass clazz, jboolean paused);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeResize(JNIEnv* env, jclass clazz, jint width, jint height);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeRender(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeRenderPanel(JNIEnv* env, jclass clazz, jint panel);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeTouch(JNIEnv* env, jclass clazz, jfloat x, jfloat y, jint action);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetTouchSurfaceSize(JNIEnv* env, jclass clazz, jint width, jint height);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetEmulationDriverPanel(JNIEnv* env, jclass clazz, jint panel);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetControllerMask(JNIEnv* env, jclass clazz, jint mask);
JNIEXPORT jint JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeGetTextureGeneration(JNIEnv* env, jclass clazz);
JNIEXPORT jint JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeGetAudioSampleRate(JNIEnv* env, jclass clazz);
JNIEXPORT jint JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeAudioRead(JNIEnv* env, jclass clazz, jshortArray pcm, jint frames);
JNIEXPORT jboolean JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeConsumeTextureReloadRequest(JNIEnv* env, jclass clazz);
JNIEXPORT jint JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeGetBackgroundColor(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetBackgroundColor(JNIEnv* env, jclass clazz, jint rgb);
JNIEXPORT jint JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeGetSegmentMarkingAlpha(JNIEnv* env, jclass clazz);
JNIEXPORT void JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeSetSegmentMarkingAlpha(JNIEnv* env, jclass clazz, jint alpha);

JNIEXPORT jboolean JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeLoadRomPack(JNIEnv* env, jclass clazz, jstring path);
JNIEXPORT jbyteArray JNICALL Java_com_retrovalou_yokoi_MainActivity_nativeGetPackFileBytes(JNIEnv* env, jclass clazz, jstring name);
}

// -----------------------------------------------------------------------------
// JNI aliases for the refactor: YokoiNative forwards to MainActivity entrypoints.
// -----------------------------------------------------------------------------

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetAssetManager(JNIEnv* env, jclass clazz, jobject assetManager) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetAssetManager(env, clazz, assetManager);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetStorageRoot(JNIEnv* env, jclass clazz, jstring path) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetStorageRoot(env, clazz, path);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeInit(JNIEnv* env, jclass clazz) {
    Java_com_retrovalou_yokoi_MainActivity_nativeInit(env, clazz);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeShutdown(JNIEnv* env, jclass clazz) {
    Java_com_retrovalou_yokoi_MainActivity_nativeShutdown(env, clazz);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeStartAaudio(JNIEnv* env, jclass clazz) {
    Java_com_retrovalou_yokoi_MainActivity_nativeStartAaudio(env, clazz);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeStopAaudio(JNIEnv* env, jclass clazz) {
    Java_com_retrovalou_yokoi_MainActivity_nativeStopAaudio(env, clazz);
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeGetTextureAssetNames(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeGetTextureAssetNames(env, clazz);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeLoadRomPack(JNIEnv* env, jclass clazz, jstring path) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeLoadRomPack(env, clazz, path);
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeGetPackFileBytes(JNIEnv* env, jclass clazz, jstring name) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeGetPackFileBytes(env, clazz, name);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetTextures(
        JNIEnv* env, jclass clazz,
        jint segmentTex, jint segmentW, jint segmentH,
        jint backgroundTex, jint backgroundW, jint backgroundH,
        jint consoleTex, jint consoleW, jint consoleH) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetTextures(
            env, clazz,
            segmentTex, segmentW, segmentH,
            backgroundTex, backgroundW, backgroundH,
            consoleTex, consoleW, consoleH);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetUiTexture(JNIEnv* env, jclass clazz, jint uiTex, jint uiW, jint uiH) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetUiTexture(env, clazz, uiTex, uiW, uiH);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeGetAppMode(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeGetAppMode(env, clazz);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeGetMenuLoadChoice(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeGetMenuLoadChoice(env, clazz);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeMenuHasSaveState(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeMenuHasSaveState(env, clazz);
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeGetSelectedGameInfo(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeGetSelectedGameInfo(env, clazz);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeAutoSaveState(JNIEnv* env, jclass clazz) {
    Java_com_retrovalou_yokoi_MainActivity_nativeAutoSaveState(env, clazz);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeReturnToMenu(JNIEnv* env, jclass clazz) {
    Java_com_retrovalou_yokoi_MainActivity_nativeReturnToMenu(env, clazz);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetPaused(JNIEnv* env, jclass clazz, jboolean paused) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetPaused(env, clazz, paused);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeResize(JNIEnv* env, jclass clazz, jint width, jint height) {
    Java_com_retrovalou_yokoi_MainActivity_nativeResize(env, clazz, width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeRender(JNIEnv* env, jclass clazz) {
    Java_com_retrovalou_yokoi_MainActivity_nativeRender(env, clazz);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeRenderPanel(JNIEnv* env, jclass clazz, jint panel) {
    Java_com_retrovalou_yokoi_MainActivity_nativeRenderPanel(env, clazz, panel);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeTouch(JNIEnv* env, jclass clazz, jfloat x, jfloat y, jint action) {
    Java_com_retrovalou_yokoi_MainActivity_nativeTouch(env, clazz, x, y, action);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetTouchSurfaceSize(JNIEnv* env, jclass clazz, jint width, jint height) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetTouchSurfaceSize(env, clazz, width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetEmulationDriverPanel(JNIEnv* env, jclass clazz, jint panel) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetEmulationDriverPanel(env, clazz, panel);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetControllerMask(JNIEnv* env, jclass clazz, jint mask) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetControllerMask(env, clazz, mask);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeGetTextureGeneration(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeGetTextureGeneration(env, clazz);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeGetAudioSampleRate(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeGetAudioSampleRate(env, clazz);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeAudioRead(JNIEnv* env, jclass clazz, jshortArray pcm, jint frames) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeAudioRead(env, clazz, pcm, frames);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeConsumeTextureReloadRequest(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeConsumeTextureReloadRequest(env, clazz);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeGetBackgroundColor(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeGetBackgroundColor(env, clazz);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetBackgroundColor(JNIEnv* env, jclass clazz, jint rgb) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetBackgroundColor(env, clazz, rgb);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeGetSegmentMarkingAlpha(JNIEnv* env, jclass clazz) {
    return Java_com_retrovalou_yokoi_MainActivity_nativeGetSegmentMarkingAlpha(env, clazz);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_nativebridge_YokoiNative_nativeSetSegmentMarkingAlpha(JNIEnv* env, jclass clazz, jint alpha) {
    Java_com_retrovalou_yokoi_MainActivity_nativeSetSegmentMarkingAlpha(env, clazz, alpha);
}
