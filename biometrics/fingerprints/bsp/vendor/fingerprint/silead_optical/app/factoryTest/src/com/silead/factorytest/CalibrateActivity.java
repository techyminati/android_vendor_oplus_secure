
package com.silead.factorytest;

import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;
import android.view.MotionEvent;
import android.widget.Button;
import android.widget.TextView;
import android.app.Activity;
import android.os.Bundle;

import android.content.Context;
import android.view.Gravity;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.MotionEvent;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.view.WindowManager;
import android.widget.ImageView;

import com.silead.manager.FingerManager;
import com.silead.manager.CalibrateStepResult;

public class CalibrateActivity extends Activity implements View.OnClickListener, View.OnTouchListener {
    private static final String FILE_TAG = "Calibrate";
    private FingerManager mFingerManager;

    private Button mBackBtn;
    private Button mCalibrateStartBtn;
    private Button mCalibrateSnrBtn;
    private TextView mStepTipsTextview;
    private TextView mResultTextview;

    private int mCalStep = 1;
    private String mStrCalResult = "";

    private final boolean UI_SNR_BTN_DISPLAY = false;

    private WindowManager.LayoutParams lp;
    private ImageView mFingerpintImage;
    private boolean mIsOptic;
    private int mFingerprintImageWidth;
    private int mFingerprintImageHeight;
    private int mFingerprintImageCenter;
    private String mFingerprintImageName = null;

    private int[] mStepTipsId = new int[] {
        R.string.cal_item_step_white_tips,
        R.string.cal_item_step_black_tips,
        R.string.cal_item_step_black_tips,
        R.string.cal_item_step_snr_tips,
        R.string.cal_item_step_finish_tips
    };

    private String mLaunchMode = "";
    private final long TIMEOUT_MS = 20 * 1000;
    private final String PCBAFilePath = Environment.getExternalStorageDirectory().getPath() + "/vendor/silead/cali.txt";

    private final int PCBA_MOULD_ACTION_DOWN = 1;
    private final int PCBA_MOULD_ACTION_UP = 2;

    private final int MSG_OPTIC_STEP_DOWN_READY = 1;
    private final int MSG_OPTIC_STEP_UP_READY   = 2;
    private final int MSG_OPTIC_STEP_TIMEOUT    = 101;
    private final int MSG_OPTIC_STEP_FAIL       = 102;
    private final int MSG_OPTIC_STEP_SUCCESS    = 103;

    public static final String OPTIC_PCBA_CMD_SUCCESS = "SUCCESS";
    public static final String OPTIC_PCBA_CMD_FAIL = "FAIL";
    public static final String OPTIC_PCBA_CMD_TIMEOUT = "TIMEOUT";

    private static final String[] OPTIC_PCBA_CMD_STEP_ACTION = new String[] {
        "FLESH_DOWN",
        "FLESH_UP",
        "DARK_DOWN",
        "",
        "",
        "DARK_UP",
        "CHART_DOWN",
        "CHART_UP"
    };
    private static final String[] OPTIC_PCBA_CMD_STEP_ACTION_READY = new String[] {
        "FLESH_DOWN_READY",
        "FLESH_UP_READY",
        "DARK_DOWN_READY",
        "",
        "",
        "DARK_UP_READY",
        "CHART_DOWN_READY",
        "CHART_UP_READY"
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.test_calibrate_main);

        // adb shell am start -n com.silead.factorytest/.CalibrateActivity --es MODE "PCBA"
        Bundle bundle = this.getIntent().getExtras();
        if(null != bundle) {
            mLaunchMode = bundle.getString(FactoryTestConst.LAUNCH_MODE_KEY);
            Log.d(FactoryTestConst.LOG_TAG, "bundle: MODE " + mLaunchMode);
        } else {
            Log.d(FactoryTestConst.LOG_TAG, "bundle is null");
        }

        Context sharecontext = this;
        try {
            sharecontext = createPackageContext(FactoryTestConst.FP_SETTINGS_PACKAGE_NAME, Context.CONTEXT_IGNORE_SECURITY);
        } catch (PackageManager.NameNotFoundException e) {
        }
        mIsOptic = SharedPreferencesData.loadDataBoolean(sharecontext, FactoryTestConst.FP_SETTINGS_CONFIG,
                FactoryTestConst.FP_SETTINGS_IS_OPTIC_NAME, FactoryTestConst.FP_UI_OPTIC_MODE_DEFAULT);

        if(!mIsOptic) {
            finish();
            return;
        }

        mCalibrateStartBtn = (Button) findViewById(R.id.cal_item_start_btn);
        mCalibrateSnrBtn = (Button) findViewById(R.id.cal_item_snr_btn);
        mBackBtn = (Button) findViewById(R.id.cal_item_back_btn);
        mStepTipsTextview = (TextView) findViewById(R.id.cal_item_test_tips);
        mResultTextview = (TextView) findViewById(R.id.cal_item_test_result);

        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
            if (!FactoryTestConst.FP_UI_BACK_BTN_SUPPORT) {
                mBackBtn.setVisibility(View.GONE);
            }
        }

        if (mCalibrateStartBtn != null) {
            if(FactoryTestConst.isPCBAMode(mLaunchMode)) {
                mCalibrateStartBtn.setOnTouchListener(this);
            } else {
                mCalibrateStartBtn.setOnClickListener(this);
            }
        }
        if (mCalibrateSnrBtn != null) {
            if (UI_SNR_BTN_DISPLAY) {
                mCalibrateSnrBtn.setVisibility(View.VISIBLE);
            }
            mCalibrateSnrBtn.setOnClickListener(this);
        }

        mFingerManager = FingerManager.getDefault(this);

        mFingerprintImageWidth = SharedPreferencesData.loadDataInt(sharecontext, FactoryTestConst.OPTIC_SETTINGS_CONFIG,
                FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_WIDTH_NAME, FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_WIDTH_DEFAULT);
        mFingerprintImageHeight = SharedPreferencesData.loadDataInt(sharecontext, FactoryTestConst.OPTIC_SETTINGS_CONFIG,
                FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_HEIGHT_NAME, FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_HEIGHT_DEFAULT);
        mFingerprintImageCenter = SharedPreferencesData.loadDataInt(sharecontext, FactoryTestConst.OPTIC_SETTINGS_CONFIG,
                FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_Y_NAME, FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_Y_DEFAULT);
        mFingerprintImageName = SharedPreferencesData.loadDataString(sharecontext, FactoryTestConst.OPTIC_SETTINGS_CONFIG,
                FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_NAME);
        if (mFingerprintImageName == null || mFingerprintImageName.isEmpty()) {
            mFingerprintImageName = FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT;
        }

        if(FactoryTestConst.isPCBAMode(mLaunchMode)) {
            notifyPCBACalibrtMouldAction(1, PCBA_MOULD_ACTION_DOWN);
            waitPCBACalibrtMouldActionReady(1, PCBA_MOULD_ACTION_DOWN);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        initIcon();
        showIcon();

        mCalStep = 1;
        updateUI(mCalStep, false);
    }

    @Override
    public void onPause() {
        super.onPause();
        hideIcon();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private FingerManager.TestCmdCallback mOpticCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_OPTIC_CALIBRATE_STEP && result instanceof CalibrateStepResult) {
                CalibrateStepResult rsp = (CalibrateStepResult)result;
                Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] test result:" + rsp.toString());
                if (rsp.getErrorCode() != FingerManager.TEST_RESULT_OK) {
                    mStrCalResult += getString(R.string.cal_item_step_result_failed, mCalStep, rsp.getErrorCode()) + "\n";
                    if (rsp.hasData()) {
                        mStrCalResult += rsp.getDataStr() + "\n";
                    }
                    mCalStep = 1;
                    updateUI(mCalStep, false);
                    if(FactoryTestConst.isPCBAMode(mLaunchMode)) {
                        if (mHandler != null) {
                            mHandler.obtainMessage(MSG_OPTIC_STEP_FAIL, rsp.getStep()).sendToTarget();
                        }
                    }
                } else {
                    mStrCalResult += getString(R.string.cal_item_step_result_success, mCalStep) + "\n";
                    if (rsp.hasData()) {
                        mStrCalResult += rsp.getDataStr() + "\n";
                    }

                    if (FactoryTestConst.isPCBAMode(mLaunchMode)) {
                        if (rsp.getStep() >= OPTIC_PCBA_CMD_STEP_ACTION.length / 2) {
                            if (mHandler != null) {
                                mHandler.obtainMessage(MSG_OPTIC_STEP_SUCCESS, rsp.getStep()).sendToTarget();
                            }
                        } else {
                            notifyPCBACalibrtMouldAction(rsp.getStep(), PCBA_MOULD_ACTION_UP);
                            waitPCBACalibrtMouldActionReady(rsp.getStep(), PCBA_MOULD_ACTION_UP);
                        }
                    } else {
                        mCalStep++;
                        if (mCalStep == 3) {
                            mFingerManager.calibrateStep(mCalStep, mOpticCmdCallback);
                            updateUI(mCalStep, true);
                        } else {
                            updateUI(mCalStep, false);
                        }
                    }
                }
            }
        }
    };

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        } else if (v == mCalibrateSnrBtn) {
            mCalStep = 4;
            updateUI(mCalStep, true);
            mFingerManager.calibrateStep(mCalStep, mOpticCmdCallback);
        } else if (v == mCalibrateStartBtn) {
            updateUI(mCalStep, true);
            mFingerManager.calibrateStep(mCalStep, mOpticCmdCallback);
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (v == mCalibrateStartBtn) {
            int cmdIndex = -1;
            if (event.getAction() == MotionEvent.ACTION_UP) {
                cmdIndex = (mCalStep - 1) * 2 + 1;
            } else if (event.getAction() == MotionEvent.ACTION_DOWN) {
                cmdIndex = (mCalStep - 1) * 2;
            }
            if (cmdIndex >= 0 && cmdIndex < OPTIC_PCBA_CMD_STEP_ACTION_READY.length) {
                FactoryTestConst.writeFile(PCBAFilePath, OPTIC_PCBA_CMD_STEP_ACTION_READY[cmdIndex], false);
            }
        }
        return false;
    }

    private void updateUI(int step, boolean running) {
        if ((step == 1) && running) {
            mStrCalResult = "";
        }

        updateCalTips(step);
        updateResult();
        updataCalibrateBtn(!running);
    }

    private void updateResult() {
        if (mStrCalResult == null || mStrCalResult.isEmpty()) {
            mResultTextview.setText("");
        } else {
            mResultTextview.setText(mStrCalResult);
        }
    }

    private void updateCalTips(int step) {
        int index = step - 1;
        if (index >= 0 && index < mStepTipsId.length) {
            mStepTipsTextview.setText(getString(mStepTipsId[index]));
        }
    }

    private void updataCalibrateBtn(boolean enable) {
        mBackBtn.setEnabled(enable);
        mCalibrateStartBtn.setEnabled(enable);
        mCalibrateSnrBtn.setEnabled(enable);
    }

    private void initIcon() {
        lp = new WindowManager.LayoutParams();
        lp.width = mFingerprintImageWidth;
        lp.height = mFingerprintImageHeight;
        lp.y = mFingerprintImageCenter - mFingerprintImageHeight / 2;
        lp.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
        lp.format = PixelFormat.TRANSPARENT;
        lp.type = WindowManager.LayoutParams.TYPE_VOLUME_OVERLAY;
        lp.windowAnimations = -1;
        lp.screenBrightness = 1;
        lp.flags = lp.flags
                | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                | WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH
                | WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                | WindowManager.LayoutParams.FLAG_FULLSCREEN;
        Log.d(FactoryTestConst.LOG_TAG, "initIcon");
        mFingerpintImage = new FingerprintTestView(getApplicationContext());
    }

    private void showIcon() {
        Log.d(FactoryTestConst.LOG_TAG, "showIcon");
        WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
        if (wm != null) {
            wm.addView(mFingerpintImage, lp);
        }
    }

    private void hideIcon() {
        WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
        if (wm != null) {
            wm.removeView(mFingerpintImage);
        }
    }

    public class FingerprintTestView extends ImageView {
        private Context mContext;

        public FingerprintTestView(Context context) {
            this(context, null);
        }

        public FingerprintTestView(Context context, AttributeSet attrs) {
            this(context, attrs, 0);
        }

        public FingerprintTestView(Context context, AttributeSet attrs, int defStyleAttr) {
            super(context, attrs, defStyleAttr);
            mContext = context;

            Drawable drawable = null;
            try {
                Context contextShare = context.createPackageContext(FactoryTestConst.FP_SETTINGS_PACKAGE_NAME, Context.CONTEXT_IGNORE_SECURITY);
                int drawableId = contextShare.getResources().getIdentifier(mFingerprintImageName, "drawable", FactoryTestConst.FP_SETTINGS_PACKAGE_NAME);
                drawable = contextShare.getDrawable(drawableId);
            } catch (PackageManager.NameNotFoundException e) {
            } catch (android.content.res.Resources.NotFoundException e) {
            }

            if (drawable == null) {
                int drawableId = getResources().getIdentifier(FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT, "drawable", getPackageName());
                drawable = context.getDrawable(drawableId);
            }
            setImageDrawable(drawable);
        }

        @Override
        public boolean onTouchEvent(MotionEvent mv) {
            return true;
        }
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int step = ((Integer)msg.obj).intValue();
            Log.d(FactoryTestConst.LOG_TAG, "[" + FILE_TAG + "] handleMessage: step=" + step + ", what=" + msg.what);

            switch (msg.what) {
                case MSG_OPTIC_STEP_DOWN_READY: {
                    mFingerManager.calibrateStep(step, mOpticCmdCallback);
                    break;
                }
                case MSG_OPTIC_STEP_UP_READY: {
                    mCalStep = step + 1;
                    updateUI(mCalStep, false);
                    notifyPCBACalibrtMouldAction(step + 1, PCBA_MOULD_ACTION_DOWN);
                    waitPCBACalibrtMouldActionReady(step + 1, PCBA_MOULD_ACTION_DOWN);
                    break;
                }
                case MSG_OPTIC_STEP_TIMEOUT: {
                    mStrCalResult += getString(R.string.cal_item_step_result_timeout, step) + "\n";
                    FactoryTestConst.writeFile(PCBAFilePath, OPTIC_PCBA_CMD_TIMEOUT, true);
                    finish();
                    break;
                }
                case MSG_OPTIC_STEP_FAIL: {
                    FactoryTestConst.writeFile(PCBAFilePath, OPTIC_PCBA_CMD_FAIL, true);
                    finish();
                    break;
                }
                case MSG_OPTIC_STEP_SUCCESS: {
                    int cmdIndex = (step - 1) * 2 + 1;
                    if (cmdIndex > OPTIC_PCBA_CMD_STEP_ACTION.length) {
                        cmdIndex = OPTIC_PCBA_CMD_STEP_ACTION.length - 1;
                    }
                    FactoryTestConst.writeFile(PCBAFilePath, OPTIC_PCBA_CMD_STEP_ACTION[cmdIndex], false);
                    FactoryTestConst.writeFile(PCBAFilePath, OPTIC_PCBA_CMD_SUCCESS, true);
                    finish();
                    break;
                }
                default: {
                    break;
                }
            }
        }
    };

    private void notifyPCBACalibrtMouldAction(final int step, final int action) {
        Thread thread = new Thread() {
            @Override
            public void run() {
                int cmdIndex = -1;
                String CMD = "";

                if (PCBA_MOULD_ACTION_DOWN == action) {
                    cmdIndex = (step - 1) * 2;
                } else if(PCBA_MOULD_ACTION_UP == action) {
                    cmdIndex = (step - 1) * 2 + 1;
                }

                if (cmdIndex >= 0 && cmdIndex < OPTIC_PCBA_CMD_STEP_ACTION.length) {
                    CMD = OPTIC_PCBA_CMD_STEP_ACTION[cmdIndex];
                }

                if (CMD != null && !CMD.isEmpty()) {
                    FactoryTestConst.writeFile(PCBAFilePath, CMD, false);
                }
            }
        };
        thread.start();
    }

    private void waitPCBACalibrtMouldActionReady(final int step, final int action) {
        Thread thread = new Thread() {
            @Override
            public void run() {
                int cmdIndex = -1;
                int msgID = 0;
                if (PCBA_MOULD_ACTION_DOWN == action) {
                    cmdIndex = (step - 1) * 2;
                    msgID = MSG_OPTIC_STEP_DOWN_READY;
                } else if(PCBA_MOULD_ACTION_UP == action) {
                    cmdIndex = (step - 1) * 2 + 1;
                    msgID = MSG_OPTIC_STEP_UP_READY;
                }

                String CMD = "";
                if (cmdIndex >= 0 && cmdIndex < OPTIC_PCBA_CMD_STEP_ACTION_READY.length) {
                    CMD = OPTIC_PCBA_CMD_STEP_ACTION_READY[cmdIndex];
                }

                if (CMD == null || CMD.isEmpty()) {
                    Log.d(FactoryTestConst.LOG_TAG, "waitReady: not need wait");
                    if (mHandler != null) {
                        mHandler.obtainMessage(msgID, step).sendToTarget();
                    }
                } else {
                    try {
                        String buff = "";
                        long expiredTimeMs = SystemClock.elapsedRealtime() + TIMEOUT_MS;
                        while (!buff.equals(CMD)) {
                            if (SystemClock.elapsedRealtime() > expiredTimeMs) {
                                Log.e(FactoryTestConst.LOG_TAG, "waitReady: " + CMD +" Time out (" + step + " " + expiredTimeMs + ":" + SystemClock.elapsedRealtime() + ")");

                                if (mHandler != null) {
                                    mHandler.obtainMessage(MSG_OPTIC_STEP_TIMEOUT, step).sendToTarget();
                                }
                                break;
                            }

                            buff = String.valueOf(FactoryTestConst.readFile(PCBAFilePath, "UTF-8"));
                            Log.d(FactoryTestConst.LOG_TAG, "waitReady: get strBuilder " + buff);
                            sleep(100);
                        }
                        if (buff.equals(CMD) && mHandler != null) {
                            mHandler.obtainMessage(msgID, step).sendToTarget();
                        }
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        };
        thread.start();
    }
}
