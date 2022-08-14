
package com.silead.factorytest;

import android.content.Intent;
import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;

public class BaseActivity extends Activity {
    private String mLaunchMode = "";

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        Bundle bundle = this.getIntent().getExtras();
        if(null != bundle) {
            mLaunchMode = bundle.getString(FactoryTestConst.LAUNCH_MODE_KEY);
            Log.d(FactoryTestConst.LOG_TAG, "bundle: MODE " + mLaunchMode);
        } else {
            Log.d(FactoryTestConst.LOG_TAG, "bundle is null");
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        removeTimeOut();
    }

    protected void checkAndSetFinishWithResult(int result) {
        removeTimeOut();

        if (FactoryTestConst.isPCBAMode(mLaunchMode)) {
            Log.d(FactoryTestConst.LOG_TAG, "get result " + result);
            Intent intent = this.getIntent();
            intent.putExtra("ErrorCode", result);
            setResult(result, intent);
            finish();
        }
    }

    void onTimeOut() {
    }

    private Handler handler = new Handler();
    Runnable runnable = new Runnable(){
        @Override
        public void run() {
            onTimeOut();
        }
    };

    protected void setTimeOut(int ms) {
        handler.postDelayed(runnable, ms);
    }

    protected void setDefaultTimeOut() {
        handler.postDelayed(runnable, FactoryTestConst.CMD_TIMEOUT_MS);
    }

    protected void removeTimeOut() {
        handler.removeCallbacks(runnable);
    }
}
