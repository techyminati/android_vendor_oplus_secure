
package com.silead.factorytest;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

public class SharedPreferencesData {
    public static String loadDataString(Context context, String file, String name) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_MULTI_PROCESS);
        return sp.getString(name, "");
    }

    public static int loadDataInt(Context context, String file, String name, int def) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_MULTI_PROCESS);
        return sp.getInt(name, def);
    }

    public static Boolean loadDataBoolean(Context context, String file, String name, Boolean def) {
        SharedPreferences sp = context.getSharedPreferences(file, Context.MODE_MULTI_PROCESS);
        return sp.getBoolean(name, def);
    }
}
