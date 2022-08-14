
package com.silead.factorytest;

import android.text.method.ScrollingMovementMethod;
import android.app.Activity;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerSpiResult;

public class SpiStressTestActivity extends BaseActivity implements View.OnClickListener {
    private static final String FILE_TAG = "SpiStressTest";
    private FingerManager mFingerManager;

    private Button mBackBtn;
    private Button mPauseContinueBtn;
    private TextView mResultTextview;
    private TextView mResultDesTextview;

    private int mStressTestTotal = -1;
    private int mStressTestCount = 0;
    private long mStartTime = 0;
    private boolean mPause = false;

    public static final int RESULT_LINE_SHOW_COUNT = 1000;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.test_spi_stress_main);

        Bundle bundle = this.getIntent().getExtras();
        if(null != bundle) {
            mStressTestTotal = bundle.getInt(FactoryTestConst.LAUNCH_COUNT_KEY);
            Log.v(FactoryTestConst.LOG_TAG, "mStressTestTotal: " + mStressTestTotal);
        }

        mResultTextview = (TextView) findViewById(R.id.test_spi_result);
        mResultDesTextview = (TextView) findViewById(R.id.test_spi_des);
        mResultDesTextview.setMovementMethod(ScrollingMovementMethod.getInstance());

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
            if (!FactoryTestConst.FP_UI_BACK_BTN_SUPPORT) {
                mBackBtn.setVisibility(View.GONE);
            }
        }

        mPauseContinueBtn = (Button) findViewById(R.id.ui_bottom_btn_2);
        if (mPauseContinueBtn != null) {
            mPauseContinueBtn.setOnClickListener(this);
        }

        mFingerManager = FingerManager.getDefault(this);
    }
    
    @Override
    public void onResume() {
        super.onResume();

        initUI();
        testSpi();
    }

    @Override
    public void onPause() {
        super.onPause();

        updatePauseContinueBtn(true);
        finish();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    private void testSpi() {
        if (FactoryTestConst.LOG_DBG) {
            Log.v(FactoryTestConst.LOG_TAG, "testSpi");
        }
        setDefaultTimeOut();
        mStartTime = SystemClock.uptimeMillis();
        if (mFingerManager != null) {
            mFingerManager.testSpi(mTestCmdCallback);
        }
    }

    private void initUI() {
        mStressTestCount = 0;
        mResultTextview.setText(String.valueOf(mStressTestCount));
        mResultDesTextview.setText("");
        
        updatePauseContinueBtn(false);
    }

    private void updatePauseContinueBtn(boolean pause) {
        mPause = pause;
        if (mPauseContinueBtn != null) {
            if (mPause) {
                mPauseContinueBtn.setText(R.string.globle_continue_btn_title);
            } else {
                mPauseContinueBtn.setText(R.string.globle_pause_btn_title);
            }
        }
    }

    private void updateUI(int result) {
        removeTimeOut();

        mStressTestCount++;
        mResultTextview.setText(String.valueOf(mStressTestCount));

        if ((mStressTestCount % RESULT_LINE_SHOW_COUNT) == 0) {
            mResultDesTextview.setText("");
            mResultDesTextview.scrollTo(0, 0);
        }

        long time = SystemClock.uptimeMillis();
        if (result == 0) {
            if ((mStressTestTotal <= 0) || (mStressTestCount < mStressTestTotal)) {
                mResultDesTextview.append(mStressTestCount + ": " + (time - mStartTime) + " PASS\n");
            } else {
                mResultDesTextview.append(mStressTestCount + ": " + (time - mStartTime) + " PASS (finish)\n");
                updatePauseContinueBtn(true);
            }
        } else if (result > 0) {
            mResultDesTextview.append(mStressTestCount + ": " + (time - mStartTime) + " FAILED\n");
            updatePauseContinueBtn(true);
        } else {
            mResultDesTextview.append(mStressTestCount + ": " + (time - mStartTime) + " TIMEOUT\n");
            updatePauseContinueBtn(true);
        }

        int offset = mResultDesTextview.getLineCount() * mResultDesTextview.getLineHeight();
        if (offset > mResultDesTextview.getHeight()) {
            mResultDesTextview.scrollTo(0, offset - mResultDesTextview.getHeight());
        }
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_SPI && result instanceof FingerSpiResult) {
                FingerSpiResult rsp = (FingerSpiResult)result;
                String strChipId = rsp.getChipId().toString();
                if (strChipId != null && !strChipId.equals("unknow")) { // pass
                    updateUI(0);
                    Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] strChipId: " + strChipId + " " + mStressTestCount);

                    if ((mStressTestTotal <= 0) || (mStressTestCount < mStressTestTotal)) {
                        if (!mPause) {
                            testSpi();
                        }
                    } else {
                        mStressTestCount = 0;
                        checkAndSetFinishWithResult(0);
                    }
                } else { // failed
                    Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] failed");
                    updateUI(1);
                    checkAndSetFinishWithResult(-1);
                }
            }
        }
    };

    @Override
    void onTimeOut() {
        Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] timeout");
        updateUI(-1);
        checkAndSetFinishWithResult(-1);
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        } else if (v == mPauseContinueBtn) {
            if (mPause) {
                updatePauseContinueBtn(false);
                testSpi();
            } else {
                updatePauseContinueBtn(true);
            }
        }
    }
}