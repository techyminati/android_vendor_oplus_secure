
package com.silead.factorytest;

import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.app.Activity;
import android.os.Bundle;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerResult;

public class SelfTestActivity extends BaseActivity implements View.OnClickListener {
    private static final String FILE_TAG = "SelfTest";
    private FingerManager mFingerManager;

    private Button mBackBtn;
    private TextView mResultTextview;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.test_selftest_main);

        mResultTextview = (TextView) findViewById(R.id.test_selftest_result);

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
            if (!FactoryTestConst.FP_UI_BACK_BTN_SUPPORT) {
                mBackBtn.setVisibility(View.GONE);
            }
        }

        mFingerManager = FingerManager.getDefault(this);
        setDefaultTimeOut();
        mFingerManager.selfTest(mTestCmdCallback);
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_SELF_TEST && result instanceof FingerResult) {
                int result_pass = -1;
                FingerResult rsp = (FingerResult)result;
                Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] test result: " + rsp.getErrorCode());

                if (rsp.getErrorCode() == FingerManager.TEST_RESULT_OK) {
                    mResultTextview.setText(R.string.globle_test_result_pass);
                    mResultTextview.setTextAppearance(SelfTestActivity.this, R.style.TestResultPassStyle);
                    result_pass = 0;
                } else {
                    mResultTextview.setText(R.string.globle_test_result_fail);
                    mResultTextview.setTextAppearance(SelfTestActivity.this, R.style.TestResultFailStyle);
                }

                checkAndSetFinishWithResult(result_pass);
            }
        }
    };

    @Override
    void onTimeOut() {
        Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] timeout");
        mResultTextview.setText(R.string.globle_test_result_fail);
        mResultTextview.setTextAppearance(SelfTestActivity.this, R.style.TestResultFailStyle);
        checkAndSetFinishWithResult(-1);
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        }
    }
}
