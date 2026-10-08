package com.retrovalou.yokoi.ui;

import android.app.Activity;
import android.app.Dialog;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import com.retrovalou.yokoi.AppModes;
import com.retrovalou.yokoi.input.overlay.OverlayControls;
import com.retrovalou.yokoi.nativebridge.YokoiNative;
import com.retrovalou.yokoi.prefs.AppPrefs;

public final class SettingsMenu {
    private final Activity activity;
    private Dialog dialog;

    public interface BooleanGetter {
        boolean get();
    }

    public interface BooleanSetter {
        void set(boolean value);
    }

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

    public SettingsMenu(Activity activity) {
        this.activity = activity;
    }

    public boolean isShowing() {
        return dialog != null && dialog.isShowing();
    }

    public void dismissIfShowing() {
        if (dialog != null && dialog.isShowing()) {
            dialog.dismiss();
        }
    }

    public void show(
            OverlayControls overlayControls,
            BooleanGetter hasExternalDisplayConnected,
            BooleanGetter isSingleScreenTopOnly,
            BooleanSetter setSingleScreenTopOnly,
            BooleanGetter isSwapScreens,
            BooleanSetter setSwapScreens,
            Runnable quitToMenu
    ) {
        if (dialog != null && dialog.isShowing()) {
            return;
        }

        final int mode = YokoiNative.nativeGetAppMode();
        final boolean inGame = (mode == AppModes.MODE_GAME);
        final boolean hasExternalDisplay = hasExternalDisplayConnected != null && hasExternalDisplayConnected.get();

        if (inGame) {
            YokoiNative.nativeSetPaused(true);
        }

        final int initialAlpha = clampInt(YokoiNative.nativeGetSegmentMarkingAlpha(), 0, 255);
        final int initialBg = YokoiNative.nativeGetBackgroundColor() & 0x00FFFFFF;

        final int pad = (int) (16 * activity.getResources().getDisplayMetrics().density);
        LinearLayout root = new LinearLayout(activity);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(pad, pad, pad, pad);
        root.setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));

        TextView segLabel = new TextView(activity);
        segLabel.setText("Segments");
        root.addView(segLabel);

        Spinner segSpinner = new Spinner(activity);
        ArrayAdapter<String> segAdapter = new ArrayAdapter<>(activity, android.R.layout.simple_spinner_item, SEG_PRESET_NAMES);
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
                YokoiNative.nativeSetSegmentMarkingAlpha(SEG_PRESET_ALPHA[idx]);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        TextView bgLabel = new TextView(activity);
        bgLabel.setText("Background color");
        LinearLayout.LayoutParams bgLp = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        bgLp.topMargin = pad / 2;
        bgLabel.setLayoutParams(bgLp);
        root.addView(bgLabel);

        Spinner bgSpinner = new Spinner(activity);
        ArrayAdapter<String> bgAdapter = new ArrayAdapter<>(activity, android.R.layout.simple_spinner_item, BG_PRESET_NAMES);
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
                YokoiNative.nativeSetBackgroundColor(BG_PRESET_COLORS[idx]);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        TextView overlayLabel = new TextView(activity);
        overlayLabel.setText("On-screen controls");
        LinearLayout.LayoutParams overlayLabelLp = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        overlayLabelLp.topMargin = pad / 2;
        overlayLabel.setLayoutParams(overlayLabelLp);
        root.addView(overlayLabel);

        Switch overlaySwitch = new Switch(activity);
        overlaySwitch.setChecked(overlayControls != null ? overlayControls.isEnabled() : AppPrefs.getShowOverlayControls(activity));
        overlaySwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (overlayControls != null) {
                overlayControls.setEnabled(isChecked);
            } else {
                AppPrefs.setShowOverlayControls(activity, isChecked);
            }
        });
        root.addView(overlaySwitch);

        if (hasExternalDisplay && isSwapScreens != null && setSwapScreens != null) {
            TextView swapLabel = new TextView(activity);
            swapLabel.setText("Swap screens");
            LinearLayout.LayoutParams swapLabelLp = new LinearLayout.LayoutParams(
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            swapLabelLp.topMargin = pad / 2;
            swapLabel.setLayoutParams(swapLabelLp);
            root.addView(swapLabel);

            Switch swapSwitch = new Switch(activity);
            swapSwitch.setChecked(isSwapScreens.get());
            swapSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> setSwapScreens.set(isChecked));
            root.addView(swapSwitch);
        }

        if (inGame) {
            LinearLayout.LayoutParams btnLp = new LinearLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            btnLp.topMargin = pad;

            Button quitBtn = new Button(activity);
            quitBtn.setLayoutParams(btnLp);
            quitBtn.setText("Quit Game");
            quitBtn.setOnClickListener(v -> {
                dismissIfShowing();
                if (setSingleScreenTopOnly != null) {
                    setSingleScreenTopOnly.set(false);
                }
                if (quitToMenu != null) {
                    quitToMenu.run();
                }
            });
            root.addView(quitBtn);

            if (!hasExternalDisplay) {
                LinearLayout.LayoutParams btnLp3 = new LinearLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT);
                btnLp3.topMargin = pad / 2;

                Button topOnlyBtn = new Button(activity);
                topOnlyBtn.setLayoutParams(btnLp3);
                boolean topOnly = isSingleScreenTopOnly != null && isSingleScreenTopOnly.get();
                topOnlyBtn.setText(topOnly ? "Show both screens" : "Top screen only");
                topOnlyBtn.setOnClickListener(v -> {
                    boolean current = isSingleScreenTopOnly != null && isSingleScreenTopOnly.get();
                    if (setSingleScreenTopOnly != null) {
                        setSingleScreenTopOnly.set(!current);
                    }
                    // Ensure the emulation-driver panel is TOP when in top-only mode.
                    YokoiNative.nativeSetEmulationDriverPanel(0);
                    dismissIfShowing();
                });
                root.addView(topOnlyBtn);
            }
        }

        LinearLayout.LayoutParams btnLp2 = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        btnLp2.topMargin = inGame ? (pad / 2) : pad;

        Button closeBtn = new Button(activity);
        closeBtn.setLayoutParams(btnLp2);
        closeBtn.setText("Close");
        closeBtn.setOnClickListener(v -> dismissIfShowing());
        root.addView(closeBtn);

        MaterialAlertDialogBuilder b = new MaterialAlertDialogBuilder(activity);
        b.setTitle("Settings");
        b.setView(root);
        b.setCancelable(true);

        dialog = b.create();
        dialog.setOnDismissListener(dlg -> {
            dialog = null;
            if (YokoiNative.nativeGetAppMode() == AppModes.MODE_GAME) {
                YokoiNative.nativeSetPaused(false);
            }
        });
        dialog.show();
    }

    private static int clampInt(int v, int lo, int hi) {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    private static int findClosestBgPresetIndex(int rgb) {
        rgb &= 0x00FFFFFF;
        for (int i = 0; i < BG_PRESET_COLORS.length; i++) {
            if ((BG_PRESET_COLORS[i] & 0x00FFFFFF) == rgb) {
                return i;
            }
        }
        return 0;
    }

    private static int findClosestSegPresetIndex(int alpha) {
        alpha = clampInt(alpha, 0, 255);
        for (int i = 0; i < SEG_PRESET_ALPHA.length; i++) {
            if (SEG_PRESET_ALPHA[i] == alpha) {
                return i;
            }
        }
        return 1;
    }
}
