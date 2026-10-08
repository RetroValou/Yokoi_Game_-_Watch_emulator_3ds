package com.retrovalou.yokoi.prefs;

import android.content.Context;

public final class AppPrefs {
    private static final String PREFS_NAME = "yokoi_prefs";
    private static final String KEY_SHOW_OVERLAY_CONTROLS = "show_overlay_controls";
    private static final String KEY_SWAP_SCREENS = "swap_screens";

    private AppPrefs() {
    }

    public static boolean getShowOverlayControls(Context context) {
        return context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                .getBoolean(KEY_SHOW_OVERLAY_CONTROLS, false);
    }

    public static boolean getSwapScreens(Context context) {
        return context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                .getBoolean(KEY_SWAP_SCREENS, false);
    }

    public static void setSwapScreens(Context context, boolean swap) {
        context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                .edit()
                .putBoolean(KEY_SWAP_SCREENS, swap)
                .apply();
    }

    public static void setShowOverlayControls(Context context, boolean enabled) {
        context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
                .edit()
                .putBoolean(KEY_SHOW_OVERLAY_CONTROLS, enabled)
                .apply();
    }
}
