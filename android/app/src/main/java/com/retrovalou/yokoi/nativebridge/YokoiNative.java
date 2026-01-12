package com.retrovalou.yokoi.nativebridge;

import android.content.res.AssetManager;

public final class YokoiNative {
    static {
        System.loadLibrary("yokoi");
    }

    private YokoiNative() {
    }

    public static native void nativeSetAssetManager(AssetManager assetManager);
    public static native void nativeSetStorageRoot(String path);
    public static native void nativeInit();
    public static native void nativeShutdown();

    public static native void nativeStartAaudio();
    public static native void nativeStopAaudio();

    public static native String[] nativeGetTextureAssetNames();

    public static native boolean nativeLoadRomPack(String path);
    public static native byte[] nativeGetPackFileBytes(String name);

    public static native void nativeSetTextures(
            int segmentTex, int segmentW, int segmentH,
            int backgroundTex, int backgroundW, int backgroundH,
            int consoleTex, int consoleW, int consoleH);

    public static native void nativeSetUiTexture(int uiTex, int uiW, int uiH);

    public static native int nativeGetAppMode();
    public static native int nativeGetMenuLoadChoice();
    public static native boolean nativeMenuHasSaveState();
    public static native String[] nativeGetSelectedGameInfo();

    public static native void nativeAutoSaveState();
    public static native void nativeReturnToMenu();
    public static native void nativeSetPaused(boolean paused);

    public static native void nativeResize(int width, int height);
    public static native void nativeRender();
    public static native void nativeRenderPanel(int panel);

    public static native void nativeTouch(float x, float y, int action);
    public static native void nativeSetTouchSurfaceSize(int width, int height);

    public static native void nativeSetEmulationDriverPanel(int panel);
    public static native void nativeSetControllerMask(int mask);

    public static native int nativeGetTextureGeneration();

    public static native int nativeGetAudioSampleRate();
    public static native int nativeAudioRead(short[] pcm, int frames);

    public static native boolean nativeConsumeTextureReloadRequest();

    public static native int nativeGetBackgroundColor();
    public static native void nativeSetBackgroundColor(int rgb);

    public static native int nativeGetSegmentMarkingAlpha();
    public static native void nativeSetSegmentMarkingAlpha(int alpha);
}
