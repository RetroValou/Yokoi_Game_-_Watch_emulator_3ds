package com.retrovalou.yokoi;

import android.app.Activity;
import android.app.Presentation;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
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
import android.view.MotionEvent;

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
    private static native void nativeResize(int width, int height);
    private static native void nativeRender();
    private static native void nativeRenderPanel(int panel);
    private static native void nativeTouch(float x, float y, int action);
    private static native void nativeSetTouchSurfaceSize(int width, int height);
    private static native void nativeSetEmulationDriverPanel(int panel);
    private static native int nativeGetTextureGeneration();
    private static native int nativeGetAudioSampleRate();
    private static native int nativeAudioRead(short[] pcm, int frames);
    private static native boolean nativeConsumeTextureReloadRequest();

    private int lastSegTexId;
    private int lastBgTexId;
    private int lastCsTexId;

    private DisplayManager displayManager;
    private SecondScreenPresentation secondPresentation;
    private GLSurfaceView secondGlView;
    private volatile boolean dualDisplayEnabled;

    private final class SecondScreenPresentation extends Presentation {
        private int secondTextureGeneration;

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

                    secondTextureGeneration = nativeGetTextureGeneration();
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
                        secondTextureGeneration = gen;
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
                startAudioIfNeeded();

                textureGeneration = nativeGetTextureGeneration();

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
                    int[] ids = new int[]{lastSegTexId, lastBgTexId, lastCsTexId};
                    for (int id : ids) {
                        if (id != 0) {
                            int[] t = new int[]{id};
                            GLES30.glDeleteTextures(1, t, 0);
                        }
                    }
                    lastSegTexId = 0;
                    lastBgTexId = 0;
                    lastCsTexId = 0;

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

                    stopAudio();
                    startAudioIfNeeded();

                    textureGeneration = gen;
                }

                if (dualDisplayEnabled) {
                    // Default (physical bottom / touch) display: render the BOTTOM panel.
                    nativeRenderPanel(1);
                } else {
                    nativeRender();
                }
            }
        });

        glView.setOnTouchListener((v, event) -> {
            nativeTouch(event.getX(), event.getY(), event.getActionMasked());
            return true;
        });

        setContentView(glView);
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
        int bufferBytes = Math.max(minBytes, sampleRate / 2); // ~0.5s

        audioTrack = new AudioTrack(
                new AudioAttributes.Builder()
                        .setLegacyStreamType(AudioManager.STREAM_MUSIC)
                        .setUsage(AudioAttributes.USAGE_GAME)
                        .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                        .build(),
                new AudioFormat.Builder()
                        .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                        .setSampleRate(sampleRate)
                        .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                        .build(),
                bufferBytes,
                AudioTrack.MODE_STREAM,
                AudioManager.AUDIO_SESSION_ID_GENERATE);

        if (audioTrack.getState() != AudioTrack.STATE_INITIALIZED) {
            audioTrack.release();
            audioTrack = null;
            return;
        }

        audioTrack.play();
        audioRunning = true;

        final int outputRate = sampleRate;
        final int framesPerWrite = 256;
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
        glView.onPause();
        if (secondGlView != null) {
            secondGlView.onPause();
        }
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
        super.onDestroy();
    }
}
