package com.retrovalou.yokoi.gl;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES30;
import android.opengl.GLUtils;

import java.io.IOException;
import java.io.InputStream;

public final class TextureLoader {
    private TextureLoader() {
    }

    public static TextureInfo loadTextureFromPngBytesOrAsset(AssetManager assets, String assetName, byte[] pngBytes) {
        if (pngBytes != null && pngBytes.length > 0) {
            TextureInfo info = loadTextureFromPngBytes(pngBytes);
            if (info != null && info.id != 0) {
                return info;
            }
        }
        return loadTextureFromAsset(assets, assetName);
    }

    public static TextureInfo loadTextureFromPngBytes(byte[] pngBytes) {
        if (pngBytes == null || pngBytes.length == 0) {
            return new TextureInfo(0, 0, 0);
        }

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;

        Bitmap bitmap = BitmapFactory.decodeByteArray(pngBytes, 0, pngBytes.length, options);
        return loadTextureFromBitmap(bitmap);
    }

    public static TextureInfo loadTextureFromAsset(AssetManager assets, String assetName) {
        if (assets == null || assetName == null || assetName.isEmpty()) {
            return new TextureInfo(0, 0, 0);
        }

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;

        Bitmap bitmap;
        try (InputStream is = assets.open(assetName)) {
            bitmap = BitmapFactory.decodeStream(is, null, options);
        } catch (IOException e) {
            e.printStackTrace();
            return new TextureInfo(0, 0, 0);
        }

        return loadTextureFromBitmap(bitmap);
    }

    public static TextureInfo loadTextureFromBitmap(Bitmap bitmap) {
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
}
