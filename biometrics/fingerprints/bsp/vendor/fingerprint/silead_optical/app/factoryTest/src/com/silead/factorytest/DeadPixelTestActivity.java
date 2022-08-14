
package com.silead.factorytest;

import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.app.Activity;
import android.os.Bundle;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerDeadPixelResult;

public class DeadPixelTestActivity extends BaseActivity implements View.OnClickListener {
    private static final String FILE_TAG = "DeadPixelTest";
    private FingerManager mFingerManager;

    private Button mBackBtn;
    private TextView mDesTextview;
    private TextView mResultTextview;
    private TextView mResultDesTextview;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.test_deadpixel_main);

        mResultTextview = (TextView) findViewById(R.id.test_dead_pixel_result);
        mResultDesTextview = (TextView) findViewById(R.id.test_dead_pixel_result_des);

        mDesTextview = (TextView) findViewById(R.id.ui_bottom_des);
        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
            if (!FactoryTestConst.FP_UI_BACK_BTN_SUPPORT) {
                mBackBtn.setVisibility(View.GONE);
            }
        }
        if (mDesTextview != null) {
            mDesTextview.setVisibility(View.VISIBLE);
            mDesTextview.setText(R.string.test_item_dead_pixel_des);
        }

        mFingerManager = FingerManager.getDefault(this);
        setDefaultTimeOut();
        mFingerManager.testDeadPixel(mTestCmdCallback);
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_DEAD_PIXEL && result instanceof FingerDeadPixelResult) {
                int result_pass = -1;
                FingerDeadPixelResult rsp = (FingerDeadPixelResult)result;
                if (rsp.getErrorCode() != FingerManager.TEST_RESULT_OK) {
                    Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] test err: " + rsp.getErrorCode());
                    mResultTextview.setText(R.string.globle_test_result_fail);
                    mResultTextview.setTextAppearance(DeadPixelTestActivity.this, R.style.TestResultFailStyle);
                } else {
                    if (rsp.getResult() == 0) {
                        mResultTextview.setText(R.string.globle_test_result_pass);
                        mResultTextview.setTextAppearance(DeadPixelTestActivity.this, R.style.TestResultPassStyle);
                        result_pass = 0;
                    } else {
                        mResultTextview.setText(R.string.globle_test_result_fail);
                        mResultTextview.setTextAppearance(DeadPixelTestActivity.this, R.style.TestResultFailStyle);
                    }

                    Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] badpix: " + rsp.getDeadPixelNum() + ", badline: " + rsp.getBadlineNum());
                    mResultDesTextview.setText(getString(R.string.test_item_dead_pixel_result_des, rsp.getDeadPixelNum(), rsp.getBadlineNum()));
                }
                checkAndSetFinishWithResult(result_pass);
            }
        }
    };

    @Override
    void onTimeOut() {
        Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] timeout");
        mResultTextview.setText(R.string.globle_test_result_fail);
        mResultTextview.setTextAppearance(DeadPixelTestActivity.this, R.style.TestResultFailStyle);
        checkAndSetFinishWithResult(-1);
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        }
    }
}
