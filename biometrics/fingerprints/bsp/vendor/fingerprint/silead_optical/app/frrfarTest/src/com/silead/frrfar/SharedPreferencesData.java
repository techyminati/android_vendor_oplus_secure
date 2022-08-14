
package com.silead.frrfar;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

public class SharedPreferencesData {
    public static void saveDataString(Context context, String file, String name, String value) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_PRIVATE);
        Editor editor = sp.edit();
        editor.putString(name, value);
        editor.commit();
    }

    public static String loadDataString(Context context, String file, String name) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_PRIVATE);
        return sp.getString(name, "");
    }

    public static void saveDataInt(Context context, String file, String name, int value) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_PRIVATE);
        Editor editor = sp.edit();
        editor.putInt(name, value);
        editor.commit();
    }

    public static int loadDataInt(Context context, String file, String name, int def) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_PRIVATE);
        return sp.getInt(name, def);
    }

    public static void saveDataBoolean(Context context, String file, String name, Boolean value) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_PRIVATE);
        Editor editor = sp.edit();
        editor.putBoolean(name, value);
        editor.commit();
    }

    public static Boolean loadDataBoolean(Context context, String file, String name, Boolean def) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_PRIVATE);
        return sp.getBoolean(name, def);
    }

    public static void clearData(Context context, String file) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_PRIVATE);
        Editor editor = sp.edit();
        editor.clear();
        editor.commit();
    }
}
