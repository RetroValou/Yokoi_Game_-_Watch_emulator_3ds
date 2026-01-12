package com.retrovalou.yokoi;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import java.io.File;
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
    private View packOverlay;
    private RomPackImporter romPackImporter;

    private boolean isPackOnlyGated() {
        return BuildConfig.ROMPACK_ONLY && (glView == null);
    }

    private File getDefaultRomPackPathPreferExternal() {
        // Prefer app-specific external storage (typically on internal shared storage):
        //   /storage/emulated/0/Android/data/<package>/files/
        File dir = getExternalFilesDir(null);
        if (dir == null) {
            dir = getFilesDir();
        }
        return new File(dir, "yokoi_pack_rgds.ykp");
    }

    private File getDefaultRomPackPathInternal() {
        return new File(getFilesDir(), "yokoi_pack_rgds.ykp");
    }

    private boolean tryLoadRomPackFromDefaultLocations() {
        // Try external-files location first, then internal-files location.
        File p1 = getDefaultRomPackPathPreferExternal();
        if (YokoiNative.nativeLoadRomPack(p1.getAbsolutePath())) {
            return true;
        }
        File p2 = getDefaultRomPackPathInternal();
        if (!p2.getAbsolutePath().equals(p1.getAbsolutePath())) {
            return YokoiNative.nativeLoadRomPack(p2.getAbsolutePath());
        }
        return false;
    }

    // Intentionally no startup toast: pack loading should be silent.
    // For pack-only builds, we show a blocking overlay only when a pack is missing/incompatible.

    private void launchRomPackPicker() {
        if (romPackImporter != null) {
            romPackImporter.launchPicker();
        }
    }

    private View buildPackOverlay() {
        FrameLayout overlay = new FrameLayout(this);
        overlay.setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        overlay.setBackgroundColor(0xFF000000);
        overlay.setClickable(true);
        overlay.setFocusable(true);

        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(32, 32, 32, 32);
        panel.setBackgroundColor(0xFF111111);

        TextView title = new TextView(this);
        title.setText("ROM pack missing or outdated");
        title.setTextColor(Color.WHITE);
        title.setTextSize(20);
        panel.addView(title);

        TextView body = new TextView(this);
        body.setText(
                "This build loads games from a .ykp ROM pack.\n\n" +
                "Tap Import/Update and select yokoi_pack_rgds.ykp from Downloads (or any folder).\n\n" +
                "The app will copy it into internal storage and load it.");
        body.setTextColor(Color.WHITE);
        body.setTextSize(14);
        body.setPadding(0, 16, 0, 16);
        panel.addView(body);

        Button importPack = new Button(this);
        importPack.setText("Import/Update ROM pack");
        importPack.setOnClickListener(v -> launchRomPackPicker());
        panel.addView(importPack);

        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                Gravity.CENTER);
        panel.setLayoutParams(lp);
        panel.setClickable(true);
        panel.setFocusable(true);

        overlay.addView(panel);
        return overlay;
    }

    private void showPackGateScreen() {
        rootView = new FrameLayout(this);
        rootView.setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        packOverlay = buildPackOverlay();
        rootView.addView(packOverlay);
        setContentView(rootView);
    }

    private void startMainUi() {
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
                try {
                    YokoiNative.nativeInit();

                    // Textures must be created on the GL thread.
                    String[] names = YokoiNative.nativeGetTextureAssetNames();
                    String segName = (names != null && names.length > 0) ? names[0] : "";
                    String bgName = (names != null && names.length > 1) ? names[1] : "";
                    String csName = (names != null && names.length > 2) ? names[2] : "";

                    TextureInfo seg = loadTextureByName(segName);
                    TextureInfo bg = loadTextureByName(bgName);
                    TextureInfo cs = loadTextureByName(csName);

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
                } catch (Throwable t) {
                    t.printStackTrace();
                    runOnUiThread(() -> Toast.makeText(MainActivity.this, "Render init failed: " + t.getMessage(), Toast.LENGTH_LONG).show());
                }
            }

            @Override
            public void onSurfaceChanged(javax.microedition.khronos.opengles.GL10 gl, int width, int height) {
                YokoiNative.nativeResize(width, height);
                YokoiNative.nativeSetTouchSurfaceSize(width, height);
            }

            @Override
            public void onDrawFrame(javax.microedition.khronos.opengles.GL10 gl) {
                try {
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

                        TextureInfo seg = loadTextureByName(segName);
                        TextureInfo bg = loadTextureByName(bgName);
                        TextureInfo cs = loadTextureByName(csName);

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
                } catch (Throwable t) {
                    t.printStackTrace();
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

        // Attempt to load an external ROM pack from default locations.
        // If not present, the user can import it via the in-app button.
        boolean packOk = tryLoadRomPackFromDefaultLocations();

        romPackImporter = new RomPackImporter(
                this,
                getDefaultRomPackPathInternal(),
                YokoiNative::nativeLoadRomPack,
                new RomPackImporter.Listener() {
                    @Override
                    public void onRomPackImported(File dest, boolean loadOk) {
                        if (loadOk) {
                            // If we started in "gated" mode (no GL/UI yet), bring up the main UI now.
                            if (BuildConfig.ROMPACK_ONLY && glView == null) {
                                startMainUi();
                            } else if (packOverlay != null) {
                                packOverlay.setVisibility(View.GONE);
                            }
                            Toast.makeText(MainActivity.this, "ROM pack loaded: " + dest.getAbsolutePath(), Toast.LENGTH_LONG).show();
                        } else {
                            Toast.makeText(MainActivity.this, "ROM pack import OK but load failed (see logs)", Toast.LENGTH_LONG).show();
                        }
                    }

                    @Override
                    public void onRomPackImportFailed(String message) {
                        // Keep it non-fatal; user can retry.
                        Toast.makeText(MainActivity.this, message, Toast.LENGTH_LONG).show();
                    }
                }
        );

        // In pack-only builds, do not create/render the emulator UI until we have a valid pack.
        // This prevents the menu/overlays from appearing behind the import screen.
        if (BuildConfig.ROMPACK_ONLY && !packOk) {
            showPackGateScreen();
            return;
        }

        startMainUi();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (romPackImporter != null && romPackImporter.handleActivityResult(requestCode, resultCode, data)) {
            return;
        }
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
        if (glView != null) {
            glView.onPause();
        }
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
        if (glView != null) {
            glView.onResume();
        }
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

    private TextureInfo loadTextureByName(String assetName) {
        byte[] bytes = YokoiNative.nativeGetPackFileBytes(assetName);
        return TextureLoader.loadTextureFromPngBytesOrAsset(getAssets(), assetName, bytes);
    }
}
