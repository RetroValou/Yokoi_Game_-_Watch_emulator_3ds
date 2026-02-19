package com.retrovalou.yokoi;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public final class RomPackImporter {
    public static final int REQ_IMPORT_ROMPACK = 1001;
    private static final String TAG = "RomPackImporter";

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
        Log.i(TAG, "Launching file picker for ROM pack");
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        activity.startActivityForResult(intent, REQ_IMPORT_ROMPACK);
    }

    /**
     * @return true if this result was handled by the importer.
     */
    public boolean handleActivityResult(int requestCode, int resultCode, Intent data) {
        Log.i(TAG, "handleActivityResult called with requestCode=" + requestCode + ", resultCode=" + resultCode);

        if (requestCode != REQ_IMPORT_ROMPACK) {
            Log.d(TAG, "Not a ROM pack request, ignoring");
            return false;
        }
        if (resultCode != Activity.RESULT_OK || data == null) {
            Log.w(TAG, "Import cancelled or intent data is null");
            listener.onRomPackImportFailed("Import cancelled");
            return true;
        }

        Uri uri = data.getData();
        if (uri == null) {
            Log.w(TAG, "No file selected, uri is null");
            listener.onRomPackImportFailed("No file selected");
            return true;
        }

        Log.i(TAG, "Selected file URI: " + uri);

        try {
            // Assure que le dossier existe
            File parent = destFile.getParentFile();
            if (!parent.exists()) {
                boolean created = parent.mkdirs();
                Log.i(TAG, "Created parent dirs: " + created + " (" + parent.getAbsolutePath() + ")");
            }

            try (InputStream in = activity.getContentResolver().openInputStream(uri);
                 OutputStream out = new FileOutputStream(destFile)) {

                if (in == null) {
                    Log.e(TAG, "Unable to open InputStream for URI: " + uri);
                    listener.onRomPackImportFailed("Unable to read selected file");
                    return true;
                }

                Log.i(TAG, "Copying file to " + destFile.getAbsolutePath());
                byte[] buf = new byte[64 * 1024];
                int n;
                long total = 0;
                while ((n = in.read(buf)) > 0) {
                    out.write(buf, 0, n);
                    total += n;
                }
                Log.i(TAG, "Copy finished, total bytes: " + total);
            }

        } catch (Exception e) {
            Log.e(TAG, "Exception while copying file", e);
            listener.onRomPackImportFailed("Import failed: " + e.getMessage());
            return true;
        }

        // Vérifie que le fichier existe et a une taille non nulle
        long size = destFile.length();
        Log.i(TAG, "Dest file size after copy: " + size);
        if (size == 0) {
            listener.onRomPackImportFailed("RomPack file is empty after copy!");
            return true;
        }

        // Appelle le loader natif
        Log.i(TAG, "Calling loader.load() for file: " + destFile.getAbsolutePath());
        boolean ok = loader.load(destFile.getAbsolutePath());
        Log.i(TAG, "Loader returned: " + ok);

        listener.onRomPackImported(destFile, ok);
        return true;
    }
}
