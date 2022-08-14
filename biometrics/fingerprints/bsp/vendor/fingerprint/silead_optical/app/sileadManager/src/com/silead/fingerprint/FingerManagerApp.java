
package com.silead.fingerprint;

import android.app.Application;

public class FingerManagerApp extends Application {
    private FingerService mService;

    public FingerManagerApp() {
    }

    @Override
    public void onCreate() {
        mService = new FingerService(this);
    }
}
