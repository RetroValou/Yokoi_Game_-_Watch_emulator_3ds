package com.retrovalou.yokoi.display;

import android.app.Activity;
import android.app.Presentation;
import android.content.Context;
import android.hardware.display.DisplayManager;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Display;

import com.retrovalou.yokoi.AppModes;
import com.retrovalou.yokoi.gl.MenuUiTextureBuilder;
import com.retrovalou.yokoi.gl.TextureInfo;
import com.retrovalou.yokoi.gl.TextureLoader;
import com.retrovalou.yokoi.nativebridge.YokoiNative;
import com.retrovalou.yokoi.prefs.AppPrefs;

public final class SecondScreenController {
    public interface OnExternalDisplayPresent {
        void run();
    }

    private final Activity activity;
    private final DisplayManager displayManager;
    private final OnExternalDisplayPresent onExternalDisplayPresent;

    private SecondScreenPresentation presentation;
    private GLSurfaceView secondGlView;
    private volatile boolean dualDisplayEnabled;
    // When true, the activity display shows the TOP panel and the secondary display shows the BOTTOM panel
    // (needed on devices such as the AYN Thor where the primary display is the physical top screen).
    private volatile boolean swapScreens;

    public SecondScreenController(Activity activity, OnExternalDisplayPresent onExternalDisplayPresent) {
        this.activity = activity;
        this.onExternalDisplayPresent = onExternalDisplayPresent;
        this.displayManager = (DisplayManager) activity.getSystemService(Context.DISPLAY_SERVICE);
        this.swapScreens = AppPrefs.getSwapScreens(activity);
    }

    public boolean isSwapScreens() {
        return swapScreens;
    }

    public void setSwapScreens(boolean swap) {
        swapScreens = swap;
        AppPrefs.setSwapScreens(activity, swap);
        if (dualDisplayEnabled) {
            YokoiNative.nativeSetEmulationDriverPanel(swap ? 0 : 1);
        }
    }

    public boolean isDualDisplayEnabled() {
        return dualDisplayEnabled;
    }

    public boolean hasExternalDisplayConnected() {
        if (displayManager == null) {
            return false;
        }
        Display defaultDisplay = activity.getWindowManager().getDefaultDisplay();
        int defaultId = defaultDisplay != null ? defaultDisplay.getDisplayId() : Display.DEFAULT_DISPLAY;
        for (Display d : displayManager.getDisplays()) {
            if (d != null && d.getDisplayId() != defaultId) {
                return true;
            }
        }
        return false;
    }

    public void tryStartSecondDisplay() {
        if (displayManager == null || presentation != null) {
            return;
        }

        Display defaultDisplay = activity.getWindowManager().getDefaultDisplay();
        int defaultId = defaultDisplay != null ? defaultDisplay.getDisplayId() : Display.DEFAULT_DISPLAY;

        Display candidate = null;
        for (Display d : displayManager.getDisplays()) {
            if (d != null && d.getDisplayId() != defaultId) {
                candidate = d;
                break;
            }
        }

        if (candidate == null) {
            dualDisplayEnabled = false;
            return;
        }

        if (onExternalDisplayPresent != null) {
            onExternalDisplayPresent.run();
        }

        presentation = new SecondScreenPresentation(activity, candidate);
        try {
            presentation.show();
            dualDisplayEnabled = true;
            // Main (touch) display renders panel 1 (or panel 0 when swapped), so make it the emulation driver.
            YokoiNative.nativeSetEmulationDriverPanel(swapScreens ? 0 : 1);
        } catch (RuntimeException e) {
            dualDisplayEnabled = false;
            presentation = null;
            YokoiNative.nativeSetEmulationDriverPanel(0);
        }
    }

    public void stopSecondDisplay() {
        dualDisplayEnabled = false;
        YokoiNative.nativeSetEmulationDriverPanel(0);
        if (secondGlView != null) {
            try {
                secondGlView.onPause();
            } catch (RuntimeException ignored) {
            }
            secondGlView = null;
        }
        if (presentation != null) {
            try {
                presentation.dismiss();
            } catch (RuntimeException ignored) {
            }
            presentation = null;
        }
    }

    public void onPause() {
        stopSecondDisplay();
    }

    public void onResume() {
        if (secondGlView != null) {
            try {
                secondGlView.onResume();
            } catch (RuntimeException ignored) {
            }
        } else {
            tryStartSecondDisplay();
        }
    }

    private final class SecondScreenPresentation extends Presentation {
        private int secondTextureGeneration;
        private int secondUiTexId;
        private int uiLastGen;
        private int uiLastMode;
        private int uiLastChoice;

        SecondScreenPresentation(Context outerContext, Display display) {
            super(outerContext, display);
        }

        @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            secondGlView = new GLSurfaceView(getContext());
            secondGlView.setEGLContextClientVersion(3);
            secondGlView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
            secondGlView.setPreserveEGLContextOnPause(true);
            secondGlView.setRenderer(new GLSurfaceView.Renderer() {
                @Override
                public void onSurfaceCreated(javax.microedition.khronos.opengles.GL10 gl, javax.microedition.khronos.egl.EGLConfig config) {
                    YokoiNative.nativeInit();

                    // Load textures in this GL context too (dual-display devices commonly do NOT share textures).
                    String[] names = YokoiNative.nativeGetTextureAssetNames();
                    String segName = (names != null && names.length > 0) ? names[0] : "";
                    String bgName = (names != null && names.length > 1) ? names[1] : "";
                    String csName = (names != null && names.length > 2) ? names[2] : "";

                    TextureInfo seg = TextureLoader.loadTextureFromPngBytesOrAsset(activity.getAssets(), segName, YokoiNative.nativeGetPackFileBytes(segName));
                    TextureInfo bg = TextureLoader.loadTextureFromPngBytesOrAsset(activity.getAssets(), bgName, YokoiNative.nativeGetPackFileBytes(bgName));
                    TextureInfo cs = TextureLoader.loadTextureFromPngBytesOrAsset(activity.getAssets(), csName, YokoiNative.nativeGetPackFileBytes(csName));

                    YokoiNative.nativeSetTextures(
                            seg.id, seg.width, seg.height,
                            bg.id, bg.width, bg.height,
                            cs.id, cs.width, cs.height);

                    TextureInfo ui = MenuUiTextureBuilder.buildMenuUiTexture();
                    secondUiTexId = ui.id;
                    YokoiNative.nativeSetUiTexture(ui.id, ui.width, ui.height);

                    secondTextureGeneration = YokoiNative.nativeGetTextureGeneration();
                    uiLastGen = secondTextureGeneration;
                    uiLastMode = YokoiNative.nativeGetAppMode();
                    uiLastChoice = YokoiNative.nativeGetMenuLoadChoice();
                }

                @Override
                public void onSurfaceChanged(javax.microedition.khronos.opengles.GL10 gl, int width, int height) {
                    YokoiNative.nativeResize(width, height);
                }

                @Override
                public void onDrawFrame(javax.microedition.khronos.opengles.GL10 gl) {
                    if (!dualDisplayEnabled) {
                        return;
                    }
                    int gen = YokoiNative.nativeGetTextureGeneration();
                    if (gen != secondTextureGeneration) {
                        String[] names = YokoiNative.nativeGetTextureAssetNames();
                        String segName = (names != null && names.length > 0) ? names[0] : "";
                        String bgName = (names != null && names.length > 1) ? names[1] : "";
                        String csName = (names != null && names.length > 2) ? names[2] : "";

                        TextureInfo seg = TextureLoader.loadTextureFromPngBytesOrAsset(activity.getAssets(), segName, YokoiNative.nativeGetPackFileBytes(segName));
                        TextureInfo bg = TextureLoader.loadTextureFromPngBytesOrAsset(activity.getAssets(), bgName, YokoiNative.nativeGetPackFileBytes(bgName));
                        TextureInfo cs = TextureLoader.loadTextureFromPngBytesOrAsset(activity.getAssets(), csName, YokoiNative.nativeGetPackFileBytes(csName));

                        YokoiNative.nativeSetTextures(
                                seg.id, seg.width, seg.height,
                                bg.id, bg.width, bg.height,
                                cs.id, cs.width, cs.height);

                        if (secondUiTexId != 0) {
                            int[] t = new int[]{secondUiTexId};
                            GLES30.glDeleteTextures(1, t, 0);
                            secondUiTexId = 0;
                        }
                        TextureInfo ui = MenuUiTextureBuilder.buildMenuUiTexture();
                        secondUiTexId = ui.id;
                        YokoiNative.nativeSetUiTexture(ui.id, ui.width, ui.height);
                        secondTextureGeneration = gen;
                        uiLastGen = gen;
                    }

                    int mode = YokoiNative.nativeGetAppMode();
                    int choice = YokoiNative.nativeGetMenuLoadChoice();
                    if (mode != AppModes.MODE_GAME && (mode != uiLastMode || choice != uiLastChoice || gen != uiLastGen)) {
                        if (secondUiTexId != 0) {
                            int[] t = new int[]{secondUiTexId};
                            GLES30.glDeleteTextures(1, t, 0);
                            secondUiTexId = 0;
                        }
                        TextureInfo ui = MenuUiTextureBuilder.buildMenuUiTexture();
                        secondUiTexId = ui.id;
                        YokoiNative.nativeSetUiTexture(ui.id, ui.width, ui.height);
                        uiLastMode = mode;
                        uiLastChoice = choice;
                        uiLastGen = gen;
                    }

                    // Secondary display: render the TOP panel (or the BOTTOM panel when swapped).
                    YokoiNative.nativeRenderPanel(swapScreens ? 1 : 0);
                }
            });
            secondGlView.setOnTouchListener((v, event) -> {
                // Only handle touch here when swapped, i.e. when this display shows the BOTTOM panel.
                if (!dualDisplayEnabled || !swapScreens) {
                    return true;
                }
                YokoiNative.nativeSetTouchSurfaceSize(v.getWidth(), v.getHeight());
                YokoiNative.nativeTouch(event.getX(), event.getY(), event.getActionMasked());
                return true;
            });
            setContentView(secondGlView);
        }

        @Override
        public void onDisplayRemoved() {
            super.onDisplayRemoved();
            dualDisplayEnabled = false;
        }
    }
}
