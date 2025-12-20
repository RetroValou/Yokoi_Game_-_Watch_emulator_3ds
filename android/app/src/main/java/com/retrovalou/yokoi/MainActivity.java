package com.retrovalou.yokoi;

import android.app.Activity;
import android.app.Presentation;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Paint;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.os.Bundle;
import android.hardware.display.DisplayManager;
import android.view.Display;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Spinner;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;

import java.io.IOException;
import java.io.InputStream;


public final class MainActivity extends Activity {
    static {
        System.loadLibrary("yokoi");
    }

    private GLSurfaceView glView;
    private AudioTrack audioTrack;
    private Thread audioThread;
    private volatile boolean audioRunning;

    private static native void nativeSetAssetManager(AssetManager assetManager);
    private static native void nativeSetStorageRoot(String path);
    private static native void nativeInit();
    private static native String[] nativeGetTextureAssetNames();
    private static native void nativeSetTextures(
            int segmentTex, int segmentW, int segmentH,
            int backgroundTex, int backgroundW, int backgroundH,
            int consoleTex, int consoleW, int consoleH);
        private static native void nativeSetUiTexture(int uiTex, int uiW, int uiH);
        private static native int nativeGetAppMode();
        private static native int nativeGetMenuLoadChoice();
        private static native boolean nativeMenuHasSaveState();
        private static native String[] nativeGetSelectedGameInfo();
        private static native void nativeAutoSaveState();
        private static native void nativeReturnToMenu();
        private static native void nativeSetPaused(boolean paused);
    private static native void nativeResize(int width, int height);
    private static native void nativeRender();
    private static native void nativeRenderPanel(int panel);
    private static native void nativeTouch(float x, float y, int action);
    private static native void nativeSetTouchSurfaceSize(int width, int height);
    private static native void nativeSetEmulationDriverPanel(int panel);
    private static native void nativeSetControllerMask(int mask);
    private static native int nativeGetTextureGeneration();
    private static native int nativeGetAudioSampleRate();
    private static native int nativeAudioRead(short[] pcm, int frames);
    private static native boolean nativeConsumeTextureReloadRequest();

    private static native int nativeGetBackgroundColor();
    private static native void nativeSetBackgroundColor(int rgb);
    private static native int nativeGetSegmentMarkingAlpha();
    private static native void nativeSetSegmentMarkingAlpha(int alpha);

    private int lastSegTexId;
    private int lastBgTexId;
    private int lastCsTexId;
    private int lastUiTexId;

    private DisplayManager displayManager;
    private SecondScreenPresentation secondPresentation;
    private GLSurfaceView secondGlView;
    private volatile boolean dualDisplayEnabled;

    // Must match native AppMode enum in yokoi_jni.cpp
    private static final int MODE_MENU_SELECT = 0;
    private static final int MODE_MENU_LOAD_PROMPT = 1;
    private static final int MODE_GAME = 2;

    // Controller bitmask must match native ControllerBits in yokoi_jni.cpp
    private static final int CTL_DPAD_UP = 1 << 0;
    private static final int CTL_DPAD_DOWN = 1 << 1;
    private static final int CTL_DPAD_LEFT = 1 << 2;
    private static final int CTL_DPAD_RIGHT = 1 << 3;
    private static final int CTL_A = 1 << 4;
    private static final int CTL_B = 1 << 5;
    private static final int CTL_X = 1 << 6;
    private static final int CTL_Y = 1 << 7;
    private static final int CTL_START = 1 << 8;
    private static final int CTL_SELECT = 1 << 9;
    private static final int CTL_L1 = 1 << 10;

    private int controllerMask;
    private long lastDpadKeyEventUptimeMs;

    private android.app.Dialog settingsDialog;

    private static final String[] BG_PRESET_NAMES = new String[]{
            "Light Yellow",
            "Light Blue",
            "Greyish",
            "White",
            "Light Green",
            "Cream",
    };

    // Colors are 0xRRGGBB to match native settings.
    private static final int[] BG_PRESET_COLORS = new int[]{
            0xdbe2bb,
            0xE0F0FF,
            0x98A09C,
            0xFFFFFF,
            0xE0FFE0,
            0xFFF8DC,
    };

        private static final String[] SEG_PRESET_NAMES = new String[]{
            "None",
            "Light",
            "Dark",
        };

        private static final int[] SEG_PRESET_ALPHA = new int[]{
            0,
            5,
            10,
        };

    private static int clampInt(int v, int lo, int hi) {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    private int findClosestBgPresetIndex(int rgb) {
        rgb &= 0x00FFFFFF;
        for (int i = 0; i < BG_PRESET_COLORS.length; i++) {
            if ((BG_PRESET_COLORS[i] & 0x00FFFFFF) == rgb) {
                return i;
            }
        }
        // If not an exact match, default to first preset.
        return 0;
    }

    private int findClosestSegPresetIndex(int alpha) {
        alpha = clampInt(alpha, 0, 255);
        for (int i = 0; i < SEG_PRESET_ALPHA.length; i++) {
            if (SEG_PRESET_ALPHA[i] == alpha) {
                return i;
            }
        }
        // Default to Light (matches old default 0x05).
        return 1;
    }

    private void showSettingsMenu() {
        if (settingsDialog != null && settingsDialog.isShowing()) {
            return;
        }

        final int mode = nativeGetAppMode();
        final boolean inGame = (mode == MODE_GAME);

        if (inGame) {
            nativeSetPaused(true);
        }

        final int initialAlpha = clampInt(nativeGetSegmentMarkingAlpha(), 0, 255);
        final int initialBg = nativeGetBackgroundColor() & 0x00FFFFFF;

        final int pad = (int) (16 * getResources().getDisplayMetrics().density);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(pad, pad, pad, pad);
        root.setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));

        TextView segLabel = new TextView(this);
        segLabel.setText("Segments");
        root.addView(segLabel);

        Spinner segSpinner = new Spinner(this);
        ArrayAdapter<String> segAdapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, SEG_PRESET_NAMES);
        segAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        segSpinner.setAdapter(segAdapter);
        segSpinner.setSelection(findClosestSegPresetIndex(initialAlpha));
        root.addView(segSpinner);

        final boolean[] ignoreFirstSeg = new boolean[]{true};
        segSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (ignoreFirstSeg[0]) {
                    ignoreFirstSeg[0] = false;
                    return;
                }
                int idx = clampInt(position, 0, SEG_PRESET_ALPHA.length - 1);
                nativeSetSegmentMarkingAlpha(SEG_PRESET_ALPHA[idx]);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        TextView bgLabel = new TextView(this);
        bgLabel.setText("Background color");
        LinearLayout.LayoutParams bgLp = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        bgLp.topMargin = pad / 2;
        bgLabel.setLayoutParams(bgLp);
        root.addView(bgLabel);

        Spinner bgSpinner = new Spinner(this);
        ArrayAdapter<String> bgAdapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, BG_PRESET_NAMES);
        bgAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        bgSpinner.setAdapter(bgAdapter);
        bgSpinner.setSelection(findClosestBgPresetIndex(initialBg));
        root.addView(bgSpinner);

        final boolean[] ignoreFirstBg = new boolean[]{true};
        bgSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (ignoreFirstBg[0]) {
                    ignoreFirstBg[0] = false;
                    return;
                }
                int idx = clampInt(position, 0, BG_PRESET_COLORS.length - 1);
                nativeSetBackgroundColor(BG_PRESET_COLORS[idx]);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        // Actions (stacked vertically, similar width to dropdowns).
        // Only show Quit Game while in a game.
        if (inGame) {
            LinearLayout.LayoutParams btnLp = new LinearLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            btnLp.topMargin = pad;

            Button quitBtn = new Button(this);
            quitBtn.setLayoutParams(btnLp);
            quitBtn.setText("Quit Game");
            quitBtn.setOnClickListener(v -> {
                if (settingsDialog != null) {
                    settingsDialog.dismiss();
                }
                nativeReturnToMenu();
            });
            root.addView(quitBtn);
        }

        LinearLayout.LayoutParams btnLp2 = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        btnLp2.topMargin = inGame ? (pad / 2) : pad;

        Button closeBtn = new Button(this);
        closeBtn.setLayoutParams(btnLp2);
        closeBtn.setText("Close");
        closeBtn.setOnClickListener(v -> {
            if (settingsDialog != null) {
                settingsDialog.dismiss();
            }
        });
        root.addView(closeBtn);

        MaterialAlertDialogBuilder b = new MaterialAlertDialogBuilder(this);
        b.setTitle("Settings");
        b.setView(root);
        b.setCancelable(true);

        settingsDialog = b.create();
        settingsDialog.setOnDismissListener(dlg -> {
            settingsDialog = null;
            // Resume emulation if we dismissed the menu while still in-game.
            if (nativeGetAppMode() == MODE_GAME) {
                nativeSetPaused(false);
            }
        });
        settingsDialog.show();
    }

    private static float getCenteredAxis(MotionEvent event, InputDevice device, int axis) {
        if (device == null) {
            return 0.0f;
        }
        InputDevice.MotionRange range = device.getMotionRange(axis, event.getSource());
        if (range == null) {
            return 0.0f;
        }
        float value = event.getAxisValue(axis);
        float flat = range.getFlat();
        if (Math.abs(value) <= flat) {
            return 0.0f;
        }
        return value;
    }

    private void setControllerBit(int bit, boolean down) {
        int newMask = down ? (controllerMask | bit) : (controllerMask & ~bit);
        if (newMask != controllerMask) {
            controllerMask = newMask;
            nativeSetControllerMask(controllerMask);
        }
    }

    private static boolean isControllerEvent(KeyEvent event) {
        if (event == null) {
            return false;
        }
        int src = event.getSource();
        return ((src & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)
                || ((src & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)
                || ((src & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD);
    }

    private static boolean isJoystickEvent(MotionEvent event) {
        if (event == null) {
            return false;
        }
        int src = event.getSource();
        return ((src & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK);
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
                    nativeInit();

                    // Load textures in this GL context too (dual-display devices commonly do NOT share textures).
                    String[] names = nativeGetTextureAssetNames();
                    String segName = (names != null && names.length > 0) ? names[0] : "";
                    String bgName = (names != null && names.length > 1) ? names[1] : "";
                    String csName = (names != null && names.length > 2) ? names[2] : "";

                    TextureInfo seg = loadTextureFromAsset(segName);
                    TextureInfo bg = loadTextureFromAsset(bgName);
                    TextureInfo cs = loadTextureFromAsset(csName);

                    nativeSetTextures(
                            seg.id, seg.width, seg.height,
                            bg.id, bg.width, bg.height,
                            cs.id, cs.width, cs.height);

                        TextureInfo ui = buildMenuUiTexture();
                        secondUiTexId = ui.id;
                        nativeSetUiTexture(ui.id, ui.width, ui.height);

                    secondTextureGeneration = nativeGetTextureGeneration();
                        uiLastGen = secondTextureGeneration;
                        uiLastMode = nativeGetAppMode();
                        uiLastChoice = nativeGetMenuLoadChoice();
                }

                @Override
                public void onSurfaceChanged(javax.microedition.khronos.opengles.GL10 gl, int width, int height) {
                    nativeResize(width, height);
                }

                @Override
                public void onDrawFrame(javax.microedition.khronos.opengles.GL10 gl) {
                    if (!dualDisplayEnabled) {
                        return;
                    }
                    int gen = nativeGetTextureGeneration();
                    if (gen != secondTextureGeneration) {
                        // Reload textures in this context.
                        String[] names = nativeGetTextureAssetNames();
                        String segName = (names != null && names.length > 0) ? names[0] : "";
                        String bgName = (names != null && names.length > 1) ? names[1] : "";
                        String csName = (names != null && names.length > 2) ? names[2] : "";

                        TextureInfo seg = loadTextureFromAsset(segName);
                        TextureInfo bg = loadTextureFromAsset(bgName);
                        TextureInfo cs = loadTextureFromAsset(csName);

                        nativeSetTextures(
                                seg.id, seg.width, seg.height,
                                bg.id, bg.width, bg.height,
                                cs.id, cs.width, cs.height);

                        if (secondUiTexId != 0) {
                            int[] t = new int[]{secondUiTexId};
                            GLES30.glDeleteTextures(1, t, 0);
                            secondUiTexId = 0;
                        }
                        TextureInfo ui = buildMenuUiTexture();
                        secondUiTexId = ui.id;
                        nativeSetUiTexture(ui.id, ui.width, ui.height);
                        secondTextureGeneration = gen;
                        uiLastGen = gen;
                    }

                    int mode = nativeGetAppMode();
                    int choice = nativeGetMenuLoadChoice();
                    if (mode != MODE_GAME && (mode != uiLastMode || choice != uiLastChoice || gen != uiLastGen)) {
                        if (secondUiTexId != 0) {
                            int[] t = new int[]{secondUiTexId};
                            GLES30.glDeleteTextures(1, t, 0);
                            secondUiTexId = 0;
                        }
                        TextureInfo ui = buildMenuUiTexture();
                        secondUiTexId = ui.id;
                        nativeSetUiTexture(ui.id, ui.width, ui.height);
                        uiLastMode = mode;
                        uiLastChoice = choice;
                        uiLastGen = gen;
                    }

                    // Secondary (physical top) display: render the TOP panel.
                    nativeRenderPanel(0);
                }
            });
            setContentView(secondGlView);
        }

        @Override
        public void onDisplayRemoved() {
            super.onDisplayRemoved();
            dualDisplayEnabled = false;
        }
    }

    private static final class TextureInfo {
        final int id;
        final int width;
        final int height;

        TextureInfo(int id, int width, int height) {
            this.id = id;
            this.width = width;
            this.height = height;
        }
    }

    private TextureInfo loadTextureFromAsset(String assetName) {
        if (assetName == null || assetName.isEmpty()) {
            return new TextureInfo(0, 0, 0);
        }

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;

        Bitmap bitmap;
        try (InputStream is = getAssets().open(assetName)) {
            bitmap = BitmapFactory.decodeStream(is, null, options);
        } catch (IOException e) {
            e.printStackTrace();
            return new TextureInfo(0, 0, 0);
        }

        if (bitmap == null) {
            return new TextureInfo(0, 0, 0);
        }

        if (bitmap.getConfig() != Bitmap.Config.ARGB_8888) {
            Bitmap converted = bitmap.copy(Bitmap.Config.ARGB_8888, false);
            bitmap.recycle();
            bitmap = converted;
        }

        int[] tex = new int[1];
        GLES30.glGenTextures(1, tex, 0);
        int texId = tex[0];
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, texId);

        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE);

        GLES30.glPixelStorei(GLES30.GL_UNPACK_ALIGNMENT, 1);
        GLUtils.texImage2D(GLES30.GL_TEXTURE_2D, 0, bitmap, 0);

        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        bitmap.recycle();

        return new TextureInfo(texId, w, h);
    }

    private TextureInfo loadTextureFromBitmap(Bitmap bitmap) {
        if (bitmap == null) {
            return new TextureInfo(0, 0, 0);
        }

        if (bitmap.getConfig() != Bitmap.Config.ARGB_8888) {
            Bitmap converted = bitmap.copy(Bitmap.Config.ARGB_8888, false);
            bitmap.recycle();
            bitmap = converted;
        }

        int[] tex = new int[1];
        GLES30.glGenTextures(1, tex, 0);
        int texId = tex[0];
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, texId);

        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE);

        GLES30.glPixelStorei(GLES30.GL_UNPACK_ALIGNMENT, 1);
        GLUtils.texImage2D(GLES30.GL_TEXTURE_2D, 0, bitmap, 0);

        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        bitmap.recycle();

        return new TextureInfo(texId, w, h);
    }

    private TextureInfo buildMenuUiTexture() {
        // RGDS/Android: menu renders into a 640x480 logical panel.
        final int w = 640;
        final int h = 480;
        final float s = w / 400.0f;

        Bitmap bmp = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bmp);
        canvas.drawColor(Color.BLACK);

        String[] info = nativeGetSelectedGameInfo();
        String name = (info != null && info.length > 0) ? info[0] : "";
        String date = (info != null && info.length > 1) ? info[1] : "";

        Paint titlePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        titlePaint.setColor(Color.WHITE);
        titlePaint.setTextAlign(Paint.Align.CENTER);

        Paint subPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        subPaint.setColor(Color.LTGRAY);
        subPaint.setTextAlign(Paint.Align.CENTER);

        float titleSize = 34.0f * s;
        titlePaint.setTextSize(titleSize);
        while (titleSize > 18.0f * s && titlePaint.measureText(name) > (w - 20.0f * s)) {
            titleSize -= 2.0f * s;
            titlePaint.setTextSize(titleSize);
        }
        subPaint.setTextSize(18.0f * s);

        float cx = w * 0.5f;
        canvas.drawText(name, cx, 80.0f * s, titlePaint);
        if (!date.isEmpty()) {
            canvas.drawText(date, cx, 112.0f * s, subPaint);
        }

        int mode = nativeGetAppMode();
        if (mode == MODE_MENU_SELECT) {
            subPaint.setColor(Color.GRAY);
            canvas.drawText("Left/Right: select", cx, 170.0f * s, subPaint);
            canvas.drawText("Tap center or press A", cx, 200.0f * s, subPaint);
        } else if (mode == MODE_MENU_LOAD_PROMPT) {
            boolean hasSave = nativeMenuHasSaveState();
            int choice = nativeGetMenuLoadChoice();

            subPaint.setColor(Color.WHITE);
            canvas.drawText("Start:", cx, 160.0f * s, subPaint);

            Paint optPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            optPaint.setTextAlign(Paint.Align.CENTER);
            optPaint.setTextSize(20.0f * s);
            optPaint.setColor(Color.LTGRAY);

            String left = (choice == 0) ? "> Load fresh" : "Load fresh";
            String right = hasSave
                    ? ((choice == 1) ? "> Load savestate" : "Load savestate")
                    : "No savestate";

            canvas.drawText(left, w * 0.25f, 205.0f * s, optPaint);
            canvas.drawText(right, w * 0.75f, 205.0f * s, optPaint);

            subPaint.setColor(Color.GRAY);
            subPaint.setTextSize(16.0f * s);
            canvas.drawText("B: back", cx, 232.0f * s, subPaint);
        }

        return loadTextureFromBitmap(bmp);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        nativeSetAssetManager(getAssets());
        nativeSetStorageRoot(getFilesDir().getAbsolutePath());

        displayManager = (DisplayManager) getSystemService(Context.DISPLAY_SERVICE);

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
                nativeInit();

                // Textures must be created on the GL thread.
                String[] names = nativeGetTextureAssetNames();
                String segName = (names != null && names.length > 0) ? names[0] : "";
                String bgName = (names != null && names.length > 1) ? names[1] : "";
                String csName = (names != null && names.length > 2) ? names[2] : "";

                TextureInfo seg = loadTextureFromAsset(segName);
                TextureInfo bg = loadTextureFromAsset(bgName);
                TextureInfo cs = loadTextureFromAsset(csName);

                lastSegTexId = seg.id;
                lastBgTexId = bg.id;
                lastCsTexId = cs.id;

                nativeSetTextures(
                        seg.id, seg.width, seg.height,
                        bg.id, bg.width, bg.height,
                        cs.id, cs.width, cs.height);

                TextureInfo ui = buildMenuUiTexture();
                lastUiTexId = ui.id;
                nativeSetUiTexture(ui.id, ui.width, ui.height);
                startAudioIfNeeded();

                textureGeneration = nativeGetTextureGeneration();
                uiLastGen = textureGeneration;
                uiLastMode = nativeGetAppMode();
                uiLastChoice = nativeGetMenuLoadChoice();

                // Attempt to start a second physical display if present.
                    runOnUiThread(() -> {
                        tryStartSecondDisplay();
                    });
            }

            @Override
            public void onSurfaceChanged(javax.microedition.khronos.opengles.GL10 gl, int width, int height) {
                nativeResize(width, height);
                nativeSetTouchSurfaceSize(width, height);
            }

            @Override
            public void onDrawFrame(javax.microedition.khronos.opengles.GL10 gl) {
                int gen = nativeGetTextureGeneration();
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

                    String[] names = nativeGetTextureAssetNames();
                    String segName = (names != null && names.length > 0) ? names[0] : "";
                    String bgName = (names != null && names.length > 1) ? names[1] : "";
                    String csName = (names != null && names.length > 2) ? names[2] : "";

                    TextureInfo seg = loadTextureFromAsset(segName);
                    TextureInfo bg = loadTextureFromAsset(bgName);
                    TextureInfo cs = loadTextureFromAsset(csName);

                    lastSegTexId = seg.id;
                    lastBgTexId = bg.id;
                    lastCsTexId = cs.id;

                    nativeSetTextures(
                            seg.id, seg.width, seg.height,
                            bg.id, bg.width, bg.height,
                            cs.id, cs.width, cs.height);

                        TextureInfo ui = buildMenuUiTexture();
                        lastUiTexId = ui.id;
                        nativeSetUiTexture(ui.id, ui.width, ui.height);

                    stopAudio();
                    startAudioIfNeeded();

                    textureGeneration = gen;
                    uiLastGen = gen;
                }

                int mode = nativeGetAppMode();
                int choice = nativeGetMenuLoadChoice();
                if (mode != MODE_GAME && (mode != uiLastMode || choice != uiLastChoice || gen != uiLastGen)) {
                    if (lastUiTexId != 0) {
                        int[] t = new int[]{lastUiTexId};
                        GLES30.glDeleteTextures(1, t, 0);
                        lastUiTexId = 0;
                    }
                    TextureInfo ui = buildMenuUiTexture();
                    lastUiTexId = ui.id;
                    nativeSetUiTexture(ui.id, ui.width, ui.height);
                    uiLastMode = mode;
                    uiLastChoice = choice;
                    uiLastGen = gen;
                }

                if (dualDisplayEnabled) {
                    // Default (physical bottom / touch) display: render the BOTTOM panel.
                    nativeRenderPanel(1);
                } else {
                    nativeRender();
                }
            }
        });

        glView.setFocusableInTouchMode(true);
        glView.requestFocus();

        glView.setOnTouchListener((v, event) -> {
            nativeTouch(event.getX(), event.getY(), event.getActionMasked());
            return true;
        });

        setContentView(glView);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (isControllerEvent(event)) {
            int action = event.getAction();
            boolean down = action == KeyEvent.ACTION_DOWN;
            int keyCode = event.getKeyCode();
            switch (keyCode) {
                case KeyEvent.KEYCODE_DPAD_UP:
                    lastDpadKeyEventUptimeMs = android.os.SystemClock.uptimeMillis();
                    setControllerBit(CTL_DPAD_UP, down);
                    return true;
                case KeyEvent.KEYCODE_DPAD_DOWN:
                    lastDpadKeyEventUptimeMs = android.os.SystemClock.uptimeMillis();
                    setControllerBit(CTL_DPAD_DOWN, down);
                    return true;
                case KeyEvent.KEYCODE_DPAD_LEFT:
                    lastDpadKeyEventUptimeMs = android.os.SystemClock.uptimeMillis();
                    setControllerBit(CTL_DPAD_LEFT, down);
                    return true;
                case KeyEvent.KEYCODE_DPAD_RIGHT:
                    lastDpadKeyEventUptimeMs = android.os.SystemClock.uptimeMillis();
                    setControllerBit(CTL_DPAD_RIGHT, down);
                    return true;
                case KeyEvent.KEYCODE_BUTTON_A:
                    setControllerBit(CTL_A, down);
                    return true;
                case KeyEvent.KEYCODE_BUTTON_B:
                    setControllerBit(CTL_B, down);
                    return true;
                case KeyEvent.KEYCODE_BUTTON_X:
                    setControllerBit(CTL_X, down);
                    return true;
                case KeyEvent.KEYCODE_BUTTON_Y:
                    setControllerBit(CTL_Y, down);
                    return true;
                case KeyEvent.KEYCODE_BUTTON_START:
                    setControllerBit(CTL_START, down);
                    return true;
                case KeyEvent.KEYCODE_BUTTON_SELECT:
                    setControllerBit(CTL_SELECT, down);
                    return true;
                case KeyEvent.KEYCODE_BUTTON_L1:
                case KeyEvent.KEYCODE_BUTTON_L2:
                    setControllerBit(CTL_L1, down);
                    return true;
                default:
                    break;
            }
        }
        return super.dispatchKeyEvent(event);
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        if (isJoystickEvent(event)) {
            // If the device provides DPAD KeyEvents, prefer those; some controllers also emit
            // joystick hat values that can get "stuck" or noisy.
            long now = android.os.SystemClock.uptimeMillis();
            if (now - lastDpadKeyEventUptimeMs < 200) {
                return true;
            }

            final float dead = 0.35f;
            InputDevice dev = event.getDevice();

            float x = getCenteredAxis(event, dev, MotionEvent.AXIS_HAT_X);
            float y = getCenteredAxis(event, dev, MotionEvent.AXIS_HAT_Y);

            // Some devices don't provide HAT axes; fall back to left stick.
            if (x == 0.0f && y == 0.0f) {
                x = getCenteredAxis(event, dev, MotionEvent.AXIS_X);
                y = getCenteredAxis(event, dev, MotionEvent.AXIS_Y);
            }

            boolean left = x < -dead;
            boolean right = x > dead;
            boolean up = y < -dead;
            boolean down = y > dead;

            // Update the DPAD bits from axes.
            int newMask = controllerMask;
            newMask = left ? (newMask | CTL_DPAD_LEFT) : (newMask & ~CTL_DPAD_LEFT);
            newMask = right ? (newMask | CTL_DPAD_RIGHT) : (newMask & ~CTL_DPAD_RIGHT);
            newMask = up ? (newMask | CTL_DPAD_UP) : (newMask & ~CTL_DPAD_UP);
            newMask = down ? (newMask | CTL_DPAD_DOWN) : (newMask & ~CTL_DPAD_DOWN);

            if (newMask != controllerMask) {
                controllerMask = newMask;
                nativeSetControllerMask(controllerMask);
            }
            return true;
        }
        return super.onGenericMotionEvent(event);
    }

    private void tryStartSecondDisplay() {
        if (displayManager == null || secondPresentation != null) {
            return;
        }
        Display defaultDisplay = getWindowManager().getDefaultDisplay();
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

        secondPresentation = new SecondScreenPresentation(this, candidate);
        try {
            secondPresentation.show();
            dualDisplayEnabled = true;
            // Main (touch) display renders panel 1 in dual-display mode, so make it the emulation driver.
            nativeSetEmulationDriverPanel(1);
        } catch (RuntimeException e) {
            // If the display cannot be used, fall back to single-screen combined layout.
            dualDisplayEnabled = false;
            secondPresentation = null;
            nativeSetEmulationDriverPanel(0);
        }
    }

    private void stopSecondDisplay() {
        dualDisplayEnabled = false;
        nativeSetEmulationDriverPanel(0);
        if (secondGlView != null) {
            try {
                secondGlView.onPause();
            } catch (RuntimeException ignored) {
            }
            secondGlView = null;
        }
        if (secondPresentation != null) {
            try {
                secondPresentation.dismiss();
            } catch (RuntimeException ignored) {
            }
            secondPresentation = null;
        }
    }

    private void stopAudio() {
        audioRunning = false;
        if (audioThread != null) {
            try {
                audioThread.join(500);
            } catch (InterruptedException ignored) {
            }
            audioThread = null;
        }
        if (audioTrack != null) {
            try {
                audioTrack.pause();
                audioTrack.flush();
                audioTrack.stop();
            } catch (IllegalStateException ignored) {
            }
            audioTrack.release();
            audioTrack = null;
        }
    }

    private void startAudioIfNeeded() {
        if (audioTrack != null || audioThread != null) {
            return;
        }

        final int sourceRate = Math.max(1, nativeGetAudioSampleRate());
        final int framesPerWrite = 256;

        int sampleRate = sourceRate;
        int minBytes = AudioTrack.getMinBufferSize(
                sampleRate,
                AudioFormat.CHANNEL_OUT_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        if (minBytes <= 0) {
            // Fallback rates are widely supported.
            sampleRate = 48000;
            minBytes = AudioTrack.getMinBufferSize(
                    sampleRate,
                    AudioFormat.CHANNEL_OUT_MONO,
                    AudioFormat.ENCODING_PCM_16BIT);
            if (minBytes <= 0) {
                sampleRate = 44100;
                minBytes = AudioTrack.getMinBufferSize(
                        sampleRate,
                        AudioFormat.CHANNEL_OUT_MONO,
                        AudioFormat.ENCODING_PCM_16BIT);
            }
            if (minBytes <= 0) {
                // Give up quietly.
                return;
            }
        }
        // Keep the buffer small to reduce latency (the previous ~0.5s buffer adds noticeable lag).
        // Use a small multiple of minBytes and ensure it's at least a few writes worth.
        int bytesPerFrame = 2; // mono, PCM_16
        int minForWrites = framesPerWrite * bytesPerFrame * 4; // ~4 callbacks worth
        int bufferBytes = Math.max(minBytes * 2, minForWrites);

        audioTrack = new AudioTrack.Builder()
            .setAudioAttributes(new AudioAttributes.Builder()
                .setLegacyStreamType(AudioManager.STREAM_MUSIC)
                .setUsage(AudioAttributes.USAGE_GAME)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build())
            .setAudioFormat(new AudioFormat.Builder()
                .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                .setSampleRate(sampleRate)
                .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                .build())
            .setTransferMode(AudioTrack.MODE_STREAM)
            .setBufferSizeInBytes(bufferBytes)
            .setPerformanceMode(AudioTrack.PERFORMANCE_MODE_LOW_LATENCY)
            .build();

        if (audioTrack.getState() != AudioTrack.STATE_INITIALIZED) {
            audioTrack.release();
            audioTrack = null;
            return;
        }

        audioTrack.play();
        audioRunning = true;

        final int outputRate = sampleRate;
        audioThread = new Thread(() -> {
            short[] pcm = new short[framesPerWrite];

            // If outputRate != sourceRate, resample a mono PCM stream with linear interpolation.
            final boolean needResample = outputRate != sourceRate;
            final float step = needResample ? (sourceRate / (float) outputRate) : 1.0f;
            float srcPos = 0.0f;
            final short[] src = needResample ? new short[512] : null;
            int srcCount = 0;
            int srcIndex = 0;

            if (needResample) {
                srcCount = nativeAudioRead(src, src.length);
                srcIndex = 0;
                srcPos = 0.0f;
            }

            while (audioRunning) {
                if (!needResample) {
                    nativeAudioRead(pcm, framesPerWrite);
                } else {
                    for (int i = 0; i < framesPerWrite; i++) {
                        int i0 = srcIndex + (int) srcPos;
                        float frac = srcPos - (int) srcPos;
                        // Ensure we have i0 and i0+1.
                        while (i0 + 1 >= srcCount) {
                            // Shift remaining samples down.
                            int remain = Math.max(0, srcCount - srcIndex);
                            if (remain > 0) {
                                System.arraycopy(src, srcIndex, src, 0, remain);
                            }
                            int got = nativeAudioRead(src, src.length - remain);
                            srcCount = remain + Math.max(0, got);
                            srcIndex = 0;
                            srcPos = 0.0f;
                            i0 = 0;
                            frac = 0.0f;
                            if (srcCount <= 1) {
                                break;
                            }
                        }
                        short s0 = (srcCount > 0) ? src[i0] : 0;
                        short s1 = (srcCount > 1) ? src[i0 + 1] : s0;
                        pcm[i] = (short) (s0 + (s1 - s0) * frac);
                        srcPos += step;
                        int adv = (int) srcPos;
                        if (adv > 0) {
                            srcIndex += adv;
                            srcPos -= adv;
                        }
                    }
                }
                int wrote = audioTrack.write(pcm, 0, framesPerWrite);
                if (wrote <= 0) {
                    try {
                        Thread.sleep(5);
                    } catch (InterruptedException ignored) {
                    }
                }
            }
        }, "yokoi-audio");
        audioThread.setDaemon(true);
        audioThread.start();
    }
    @Override
    protected void onPause() {
        super.onPause();
        nativeAutoSaveState();
        glView.onPause();
        if (secondGlView != null) {
            secondGlView.onPause();
        }
        // Always dismiss the second display Presentation when backgrounding (Home/recents)
        // so it doesn't linger showing the last frame.
        stopSecondDisplay();
        if (settingsDialog != null && settingsDialog.isShowing()) {
            settingsDialog.dismiss();
        }
        // Avoid stuck controller bits if a device disconnects or stops sending events.
        controllerMask = 0;
        nativeSetControllerMask(0);
        stopAudio();
    }

    @Override
    protected void onResume() {
        super.onResume();
        glView.onResume();
        if (secondGlView != null) {
            secondGlView.onResume();
        } else {
            tryStartSecondDisplay();
        }
    }

    @Override
    protected void onDestroy() {
        stopSecondDisplay();
        stopAudio();
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (settingsDialog != null && settingsDialog.isShowing()) {
            settingsDialog.dismiss();
            return;
        }
        showSettingsMenu();
    }
}
