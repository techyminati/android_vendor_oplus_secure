
package com.silead.factorytest;

import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerSpiResult;

public class SpiTestActivity extends BaseActivity implements View.OnClickListener {
    private static final String FILE_TAG = "SpiTest";
    private FingerManager mFingerManager;

    private Button mBackBtn;
    private TextView mResultTextview;
    private TextView mResultDesTextview;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.test_spi_main);

        mResultTextview = (TextView) findViewById(R.id.test_spi_result);
        mResultDesTextview = (TextView) findViewById(R.id.test_spi_des);

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
            if (!FactoryTestConst.FP_UI_BACK_BTN_SUPPORT) {
                mBackBtn.setVisibility(View.GONE);
            }
        }

        mFingerManager = FingerManager.getDefault(this);
        setDefaultTimeOut();
        mFingerManager.testSpi(mTestCmdCallback);
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_SPI && result instanceof FingerSpiResult) {
                int result_pass = -1;
                FingerSpiResult rsp = (FingerSpiResult)result;
                String strChipId = rsp.getChipId().toString();
                mResultDesTextview.setText(strChipId);
                if (strChipId != null && !strChipId.equals("unknow")) {
                    mResultTextview.setText(R.string.globle_test_result_pass);
                    mResultTextview.setTextAppearance(SpiTestActivity.this, R.style.TestResultPassStyle);
                    result_pass = 0;
                } else {
                    mResultTextview.setText(R.string.globle_test_result_fail);
                    mResultTextview.setTextAppearance(SpiTestActivity.this, R.style.TestResultFailStyle);
                }
                Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] strChipId: " + strChipId);

                checkAndSetFinishWithResult(result_pass);
            }
        }
    };

    @Override
    void onTimeOut() {
        Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] timeout");
        mResultTextview.setText(R.string.globle_test_result_fail);
        mResultTextview.setTextAppearance(SpiTestActivity.this, R.style.TestResultFailStyle);
        checkAndSetFinishWithResult(-1);
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        }
    }
}
