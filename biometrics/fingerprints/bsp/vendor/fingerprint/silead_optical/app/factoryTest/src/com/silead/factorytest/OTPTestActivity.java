
package com.silead.factorytest;

import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerOTPResult;

public class OTPTestActivity extends BaseActivity implements View.OnClickListener {
    private static final String FILE_TAG = "OTPTest";
    private FingerManager mFingerManager;

    private Button mBackBtn;
    private TextView mResultTextview;
    private TextView mResultDesTextview;
    private TextView mResultDetailTextview;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.test_otp_main);

        mResultTextview = (TextView) findViewById(R.id.test_otp_result);
        mResultDesTextview = (TextView) findViewById(R.id.test_otp_info);
        mResultDetailTextview = (TextView) findViewById(R.id.test_otp_info_detail);

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
            if (!FactoryTestConst.FP_UI_BACK_BTN_SUPPORT) {
                mBackBtn.setVisibility(View.GONE);
            }
        }

        mFingerManager = FingerManager.getDefault(this);
        setDefaultTimeOut();
        mFingerManager.testOTP(mTestCmdCallback);
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_TEST_OTP && result instanceof FingerOTPResult) {
                int result_pass = -1;
                FingerOTPResult rsp = (FingerOTPResult)result;
                Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] test result: " + rsp.getErrorCode());

                if (rsp.getErrorCode() == FingerManager.TEST_RESULT_OK) {
                    mResultTextview.setText(R.string.globle_test_result_pass);
                    mResultTextview.setTextAppearance(OTPTestActivity.this, R.style.TestResultPassStyle);
                    mResultDesTextview.setText(rsp.toString());
                    mResultDetailTextview.setText(rsp.getOTPDetails());
                    result_pass = 0;
                } else {
                    mResultTextview.setText(R.string.globle_test_result_fail);
                    mResultTextview.setTextAppearance(OTPTestActivity.this, R.style.TestResultFailStyle);
                    mResultDesTextview.setText("");
                }

                checkAndSetFinishWithResult(result_pass);
            }
        }
    };

    @Override
    void onTimeOut() {
        Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] timeout");
        mResultTextview.setText(R.string.globle_test_result_fail);
        mResultTextview.setTextAppearance(OTPTestActivity.this, R.style.TestResultFailStyle);
        checkAndSetFinishWithResult(-1);
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        }
    }
}
