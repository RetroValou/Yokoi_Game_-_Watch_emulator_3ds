package com.retrovalou.yokoi;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.Button;
import android.widget.LinearLayout;
import com.retrovalou.yokoi.audio.AudioDriver;
import com.retrovalou.yokoi.display.SecondScreenController;
import com.retrovalou.yokoi.gl.MenuUiTextureBuilder;
import com.retrovalou.yokoi.gl.TextureInfo;
import com.retrovalou.yokoi.gl.TextureLoader;
import com.retrovalou.yokoi.input.ControllerInputRouter;
import com.retrovalou.yokoi.input.overlay.OverlayControls;
import com.retrovalou.yokoi.nativebridge.YokoiNative;

import com.retrovalou.yokoi.ui.SettingsMenu;


public final class MainActivity extends Activity {
    private GLSurfaceView glView;
    private final AudioDriver audioDriver = new AudioDriver();

    private final SettingsMenu settingsMenu = new SettingsMenu(this);

    private int lastSegTexId;
    private int lastBgTexId;
    private int lastCsTexId;
    private int lastUiTexId;

    private SecondScreenController secondScreenController;

    // Single-display convenience: render the top panel only while in-game.
    private volatile boolean singleScreenTopOnly;

    private FrameLayout rootView;
    private ControllerInputRouter controllerRouter;
    private OverlayControls overlayControls;
    private void showSettingsMenu() {
        settingsMenu.show(
                overlayControls,
                () -> secondScreenController != null && secondScreenController.hasExternalDisplayConnected(),
                () -> singleScreenTopOnly,
                v -> singleScreenTopOnly = v,
                YokoiNative::nativeReturnToMenu
        );
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        YokoiNative.nativeSetAssetManager(getAssets());
        YokoiNative.nativeSetStorageRoot(getFilesDir().getAbsolutePath());

        secondScreenController = new SecondScreenController(this, () -> singleScreenTopOnly = false);

        glView = new GLSurfaceView(this);
        glView.setEGLContextClientVersion(3);
        glView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        glView.setPreserveEGLContextOnPause(true);
        glView.setRenderer(new GLSurfaceView.Renderer() {
            private int textureGeneration;
            private int uiLastGen = -1;
            private int uiLastMode = -1;
            private int uiLastChoice = -1;

            @Override
            public void onSurfaceCreated(javax.microedition.khronos.opengles.GL10 gl, javax.microedition.khronos.egl.EGLConfig config) {
                YokoiNative.nativeInit();

                // Textures must be created on the GL thread.
                String[] names = YokoiNative.nativeGetTextureAssetNames();
                String segName = (names != null && names.length > 0) ? names[0] : "";
                String bgName = (names != null && names.length > 1) ? names[1] : "";
                String csName = (names != null && names.length > 2) ? names[2] : "";

                TextureInfo seg = loadTextureFromAsset(segName);
                TextureInfo bg = loadTextureFromAsset(bgName);
                TextureInfo cs = loadTextureFromAsset(csName);

                lastSegTexId = seg.id;
                lastBgTexId = bg.id;
                lastCsTexId = cs.id;

                YokoiNative.nativeSetTextures(
                        seg.id, seg.width, seg.height,
                        bg.id, bg.width, bg.height,
                        cs.id, cs.width, cs.height);

                TextureInfo ui = MenuUiTextureBuilder.buildMenuUiTexture();
                lastUiTexId = ui.id;
                YokoiNative.nativeSetUiTexture(ui.id, ui.width, ui.height);
                audioDriver.startIfNeeded();

                textureGeneration = YokoiNative.nativeGetTextureGeneration();
                uiLastGen = textureGeneration;
                uiLastMode = YokoiNative.nativeGetAppMode();
                uiLastChoice = YokoiNative.nativeGetMenuLoadChoice();

                // Attempt to start a second physical display if present.
                    runOnUiThread(() -> {
                        if (secondScreenController != null) {
                            secondScreenController.tryStartSecondDisplay();
                        }
                    });
            }

            @Override
            public void onSurfaceChanged(javax.microedition.khronos.opengles.GL10 gl, int width, int height) {
                YokoiNative.nativeResize(width, height);
                YokoiNative.nativeSetTouchSurfaceSize(width, height);
            }

            @Override
            public void onDrawFrame(javax.microedition.khronos.opengles.GL10 gl) {
                int gen = YokoiNative.nativeGetTextureGeneration();
                if (gen != textureGeneration) {
                    int[] ids = new int[]{lastSegTexId, lastBgTexId, lastCsTexId, lastUiTexId};
                    for (int id : ids) {
                        if (id != 0) {
                            int[] t = new int[]{id};
                            GLES30.glDeleteTextures(1, t, 0);
                        }
                    }
                    lastSegTexId = 0;
                    lastBgTexId = 0;
                    lastCsTexId = 0;
                    lastUiTexId = 0;

                    String[] names = YokoiNative.nativeGetTextureAssetNames();
                    String segName = (names != null && names.length > 0) ? names[0] : "";
                    String bgName = (names != null && names.length > 1) ? names[1] : "";
                    String csName = (names != null && names.length > 2) ? names[2] : "";

                    TextureInfo seg = loadTextureFromAsset(segName);
                    TextureInfo bg = loadTextureFromAsset(bgName);
                    TextureInfo cs = loadTextureFromAsset(csName);

                    lastSegTexId = seg.id;
                    lastBgTexId = bg.id;
                    lastCsTexId = cs.id;

                        YokoiNative.nativeSetTextures(
                            seg.id, seg.width, seg.height,
                            bg.id, bg.width, bg.height,
                            cs.id, cs.width, cs.height);

                        TextureInfo ui = MenuUiTextureBuilder.buildMenuUiTexture();
                        lastUiTexId = ui.id;
                        YokoiNative.nativeSetUiTexture(ui.id, ui.width, ui.height);

                    audioDriver.stop();
                    audioDriver.startIfNeeded();

                    textureGeneration = gen;
                    uiLastGen = gen;
                }

                int mode = YokoiNative.nativeGetAppMode();
                if (mode != AppModes.MODE_GAME) {
                    // Leaving the game always returns to the default two-panel layout.
                    singleScreenTopOnly = false;
                }
                int choice = YokoiNative.nativeGetMenuLoadChoice();
                if (mode != AppModes.MODE_GAME && (mode != uiLastMode || choice != uiLastChoice || gen != uiLastGen)) {
                    if (lastUiTexId != 0) {
                        int[] t = new int[]{lastUiTexId};
                        GLES30.glDeleteTextures(1, t, 0);
                        lastUiTexId = 0;
                    }
                    TextureInfo ui = MenuUiTextureBuilder.buildMenuUiTexture();
                    lastUiTexId = ui.id;
                    YokoiNative.nativeSetUiTexture(ui.id, ui.width, ui.height);
                    uiLastMode = mode;
                    uiLastChoice = choice;
                    uiLastGen = gen;
                }

                if (secondScreenController != null && secondScreenController.isDualDisplayEnabled()) {
                    // Default (physical bottom / touch) display: render the BOTTOM panel.
                    YokoiNative.nativeRenderPanel(1);
                } else if (singleScreenTopOnly && !(secondScreenController != null && secondScreenController.hasExternalDisplayConnected())) {
                    // Single-display convenience mode.
                    YokoiNative.nativeRenderPanel(0);
                } else {
                    YokoiNative.nativeRender();
                }
            }
        });

        glView.setFocusableInTouchMode(true);
        glView.requestFocus();

        glView.setOnTouchListener((v, event) -> {
            YokoiNative.nativeTouch(event.getX(), event.getY(), event.getActionMasked());
            return true;
        });

        rootView = new FrameLayout(this);
        rootView.setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        rootView.addView(glView);

        controllerRouter = new ControllerInputRouter();
        overlayControls = new OverlayControls(this, controllerRouter);
        rootView.addView(overlayControls.getView());

        setContentView(rootView);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (controllerRouter != null && controllerRouter.dispatchKeyEvent(event)) {
            return true;
        }
        return super.dispatchKeyEvent(event);
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        if (controllerRouter != null && controllerRouter.onGenericMotionEvent(event)) {
            return true;
        }
        return super.onGenericMotionEvent(event);
    }

    @Override
    protected void onPause() {
        super.onPause();
        // Pause the native emulation loop when the app is not visible.
        YokoiNative.nativeSetPaused(true);
        YokoiNative.nativeAutoSaveState();
        glView.onPause();
        // Always dismiss the second display Presentation when backgrounding (Home/recents)
        // so it doesn't linger showing the last frame.
        if (secondScreenController != null) {
            secondScreenController.onPause();
        }
        settingsMenu.dismissIfShowing();
        // Avoid stuck controller bits if a device disconnects or stops sending events.
        if (controllerRouter != null) {
            controllerRouter.clearAll();
        }
        audioDriver.stop();
    }

    @Override
    protected void onResume() {
        super.onResume();
        YokoiNative.nativeSetPaused(false);
        glView.onResume();
        if (secondScreenController != null) {
            secondScreenController.onResume();
        }

        // `onSurfaceCreated()` may not run again when resuming.
        audioDriver.startIfNeeded();
    }

    @Override
    protected void onDestroy() {
        if (secondScreenController != null) {
            secondScreenController.stopSecondDisplay();
        }
        audioDriver.stop();
        YokoiNative.nativeShutdown();
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (settingsMenu.isShowing()) {
            settingsMenu.dismissIfShowing();
            return;
        }
        showSettingsMenu();
    }

    private TextureInfo loadTextureFromAsset(String assetName) {
        return TextureLoader.loadTextureFromAsset(getAssets(), assetName);
    }
}
