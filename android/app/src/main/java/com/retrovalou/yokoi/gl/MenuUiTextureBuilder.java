package com.retrovalou.yokoi.gl;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;

import com.retrovalou.yokoi.AppModes;
import com.retrovalou.yokoi.nativebridge.YokoiNative;

public final class MenuUiTextureBuilder {
    private MenuUiTextureBuilder() {
    }

    public static TextureInfo buildMenuUiTexture() {
        // RGDS/Android: menu renders into a 640x480 logical panel.
        final int w = 640;
        final int h = 480;
        final float s = w / 400.0f;

        Bitmap bmp = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bmp);
        canvas.drawColor(Color.BLACK);

        String[] info = YokoiNative.nativeGetSelectedGameInfo();
        String name = (info != null && info.length > 0) ? info[0] : "";
        String date = (info != null && info.length > 1) ? info[1] : "";
        String mfr = (info != null && info.length > 2) ? info[2] : "";

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

        // Manufacturer header.
        if (!mfr.isEmpty()) {
            Paint mfrPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mfrPaint.setColor(Color.GRAY);
            mfrPaint.setTextAlign(Paint.Align.CENTER);
            mfrPaint.setTextSize(16.0f * s);
            canvas.drawText(mfr, cx, 48.0f * s, mfrPaint);
        }

        canvas.drawText(name, cx, 90.0f * s, titlePaint);
        if (!date.isEmpty()) {
            canvas.drawText(date, cx, 122.0f * s, subPaint);
        }

        int mode = YokoiNative.nativeGetAppMode();
        if (mode == AppModes.MODE_MENU_SELECT) {
            subPaint.setColor(Color.GRAY);
            canvas.drawText("Up/Down: manufacturer", cx, 170.0f * s, subPaint);
            canvas.drawText("Left/Right: game", cx, 200.0f * s, subPaint);
            canvas.drawText("Tap center or press A", cx, 230.0f * s, subPaint);
        } else if (mode == AppModes.MODE_MENU_LOAD_PROMPT) {
            boolean hasSave = YokoiNative.nativeMenuHasSaveState();
            int choice = YokoiNative.nativeGetMenuLoadChoice();

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

        return TextureLoader.loadTextureFromBitmap(bmp);
    }
}
