package com.retrovalou.yokoi;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public final class RomPackImporter {
    public static final int REQ_IMPORT_ROMPACK = 1001;

    public interface Loader {
        boolean load(String path);
    }

    public interface Listener {
        void onRomPackImported(File dest, boolean loadOk);

        void onRomPackImportFailed(String message);
    }

    private final Activity activity;
    private final File destFile;
    private final Loader loader;
    private final Listener listener;

    public RomPackImporter(Activity activity, File destFile, Loader loader, Listener listener) {
        this.activity = activity;
        this.destFile = destFile;
        this.loader = loader;
        this.listener = listener;
    }

    public void launchPicker() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        activity.startActivityForResult(intent, REQ_IMPORT_ROMPACK);
    }

    /**
     * @return true if this result was handled by the importer.
     */
    public boolean handleActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode != REQ_IMPORT_ROMPACK) {
            return false;
        }
        if (resultCode != Activity.RESULT_OK || data == null) {
            listener.onRomPackImportFailed("Import cancelled");
            return true;
        }

        Uri uri = data.getData();
        if (uri == null) {
            listener.onRomPackImportFailed("No file selected");
            return true;
        }

        try (InputStream in = activity.getContentResolver().openInputStream(uri);
             OutputStream out = new FileOutputStream(destFile)) {
            if (in == null) {
                listener.onRomPackImportFailed("Unable to read selected file");
                return true;
            }
            byte[] buf = new byte[64 * 1024];
            int n;
            while ((n = in.read(buf)) > 0) {
                out.write(buf, 0, n);
            }
        } catch (Exception e) {
            listener.onRomPackImportFailed("Import failed: " + e.getMessage());
            return true;
        }

        boolean ok = loader.load(destFile.getAbsolutePath());
        listener.onRomPackImported(destFile, ok);
        return true;
    }
}
