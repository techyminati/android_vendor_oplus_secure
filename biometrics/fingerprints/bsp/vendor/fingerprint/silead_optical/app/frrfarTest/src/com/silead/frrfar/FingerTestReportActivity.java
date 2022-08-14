
package com.silead.frrfar;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

public class FingerTestReportActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "FingerFarTest";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.empty);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onClick(View v) {
    }
}
