
package com.silead.factorytest;

import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.app.Activity;
import android.os.Bundle;

import android.content.Context;
import android.content.pm.PackageManager;
import android.view.Gravity;
import android.graphics.drawable.Drawable;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.ImageView;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerSpeedResult;

public class SpeedTestActivity extends Activity implements View.OnClickListener {
    private FingerManager mFingerManager;

    private TextView mResultTextView;
    private TextView mQualityTextView;
    private TextView mAreaTextView;
    private TextView mCaptureTextView;
    private TextView mReduceNoiseTextView;
    private TextView mAuthTextView;
    private TextView mTplUpdTextView;

    private TextView mDesTextview;
    private Button mBackBtn;

    private int mImageQuality = 0;
    private int mEffectArea = 0;
    private int mCaptureTime = 0;
    private int mReduceNoiseTime = 0;
    private int mAuthTime = 0;
    private int mTplUpdTime = 0;
    private int mErrorCode = FingerManager.TEST_RESULT_OK;
    private int mAuthResult = 0;
    private boolean mFirstLoad = true;

    private WindowManager.LayoutParams lp;
    private ImageView mFingerpintImage;
    private boolean mIsOptic = FactoryTestConst.FP_UI_OPTIC_MODE_DEFAULT;
    private int mFingerprintImageWidth;
    private int mFingerprintImageHeight;
    private int mFingerprintImageCenter;
    private String mFingerprintImageName = null;
    private boolean mFingerprintImageTouchEnable = true;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        Context sharecontext = this;
        try {
            sharecontext = createPackageContext(FactoryTestConst.FP_SETTINGS_PACKAGE_NAME, Context.CONTEXT_IGNORE_SECURITY);
        } catch (PackageManager.NameNotFoundException e) {
        }
        mIsOptic = SharedPreferencesData.loadDataBoolean(sharecontext, FactoryTestConst.FP_SETTINGS_CONFIG,
                FactoryTestConst.FP_SETTINGS_IS_OPTIC_NAME, FactoryTestConst.FP_UI_OPTIC_MODE_DEFAULT);

        if (mIsOptic) {
            setContentView(R.layout.test_speed_main_optic);
        } else {
            setContentView(R.layout.test_speed_main);
        }
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mFingerManager = FingerManager.getDefault(this);

        mResultTextView  = (TextView) findViewById(R.id.test_speed_item_result);
        mQualityTextView  = (TextView) findViewById(R.id.test_speed_item_quality);
        mAreaTextView  = (TextView) findViewById(R.id.test_speed_item_area);
        mCaptureTextView  = (TextView) findViewById(R.id.test_speed_item_capture);
        mReduceNoiseTextView  = (TextView) findViewById(R.id.test_speed_item_reduce_noise);
        mAuthTextView  = (TextView) findViewById(R.id.test_speed_item_auth);
        mTplUpdTextView  = (TextView) findViewById(R.id.test_speed_item_tpl_upd);

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
            mDesTextview.setText(R.string.speed_item_test_desc);
        }

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
        mFingerprintImageTouchEnable = SharedPreferencesData.loadDataBoolean(sharecontext, FactoryTestConst.OPTIC_SETTINGS_CONFIG,
                FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_TOUCH_ENABLE_NAME, FactoryTestConst.OPTIC_SETTINGS_FINGER_IMAGE_TOUCH_ENABLE_DEFAULT);
    }

    @Override
    public void onResume() {
        super.onResume();

        initData();
        updateUI();

        if (mIsOptic) {
            initIcon();
            showIcon();
        }

        testSpeed();
    }

    @Override
    public void onPause() {
        super.onPause();

        if (mIsOptic) {
            hideIcon();
        }

        testFinish();
        finish();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        }
    }

    private void updateUI() {
        mQualityTextView.setText(String.valueOf(mImageQuality));
        mAreaTextView.setText(String.valueOf(mEffectArea));
        mCaptureTextView.setText(String.valueOf(mCaptureTime) + "ms");
        mReduceNoiseTextView.setText(String.valueOf(mReduceNoiseTime) + "ms");
        mAuthTextView.setText(String.valueOf(mAuthTime) + "ms");
        mTplUpdTextView.setText(String.valueOf(mTplUpdTime) + "ms");

        if (mErrorCode == FingerManager.TEST_RESULT_SERVICE_FAILED) {
            mResultTextView.setText(getString(R.string.globle_err_not_found_service));
        } else if (mErrorCode == FingerManager.TEST_RESULT_DATA_IMCOMPLITE) {
            mResultTextView.setText(getString(R.string.globle_err_data_invalide));
        } else if (mErrorCode == FingerManager.TEST_RESULT_BAD_PARAM) {
            mResultTextView.setText(getString(R.string.globle_err_bad_param));
        } else if (mErrorCode == FingerManager.TEST_RESULT_MOVE_TOO_FAST) {
            mResultTextView.setText(getString(R.string.globle_err_move_to_fast));
        } else if (mErrorCode == FingerManager.TEST_RESULT_NO_FINGER) {
            mResultTextView.setText(getString(R.string.globle_err_move_to_fast));
        } else if (mErrorCode == FingerManager.TEST_RESULT_ENROLL_FAKE_FINGER) {
            mResultTextView.setText(getString(R.string.globle_err_enroll_fake_finger));
        } else if (mErrorCode == FingerManager.TEST_RESULT_ENROLL_GAIN_IMPROVE_TIMEOUT) {
            mResultTextView.setText(getString(R.string.globle_err_enroll_gain_improve_timeout));
        } else if (mErrorCode == FingerManager.TEST_RESULT_CANCELED) {
            mResultTextView.setText(getString(R.string.globle_err_canceled));
        } else if (mErrorCode != FingerManager.TEST_RESULT_OK) {
            mResultTextView.setText(getString(R.string.globle_err_unknow));
        } else {
            if (!mFirstLoad) {
                if (mAuthResult > 0) {
                    mResultTextView.setText(getString(R.string.globle_err_auth_success));
                } else {
                    mResultTextView.setText(getString(R.string.globle_err_auth_failed));
                }
            }
        }
    }

    private void initData() {
        mImageQuality = 0;
        mEffectArea = 0;
        mCaptureTime = 0;
        mReduceNoiseTime = 0;
        mAuthTime = 0;
        mTplUpdTime = 0;
        mErrorCode = FingerManager.TEST_RESULT_OK;
        mAuthResult = 0;
    }

    private void testSpeed() {
        if (FactoryTestConst.LOG_DBG) {
            Log.v(FactoryTestConst.LOG_TAG, "testSpeed");
        }
        if (mFingerManager != null) {
            mFingerManager.testSpeed(mTestCmdCallback);
        }
    }

    private void testFinish() {
        if (FactoryTestConst.LOG_DBG) {
            Log.v(FactoryTestConst.LOG_TAG, "testFinish: finish");
        }
        if (mFingerManager != null) {
            mFingerManager.testFinish(mTestCmdCallback);
        }
    }

    private void testSendFingerDownMsg() {
        if (FactoryTestConst.LOG_DBG) {
            Log.v(FactoryTestConst.LOG_TAG, "testSendFingerDownMsg: ");
        }
        if (mFingerManager != null) {
            mFingerManager.testSendFingerDownMsg(mTestCmdCallback);
        }
    }

    public void testSendFingerUpMsg() {
        if (FactoryTestConst.LOG_DBG) {
            Log.v(FactoryTestConst.LOG_TAG, "testSendFingerUpMsg");
        }
        if (mFingerManager != null) {
            mFingerManager.testSendFingerUpMsg(mTestCmdCallback);
        }
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_SPEED_TEST && result instanceof FingerSpeedResult) {
                FingerSpeedResult rsp = (FingerSpeedResult)result;
                if (rsp.getErrorCode() != FingerManager.TEST_RESULT_OK) {
                    Log.v(FactoryTestConst.LOG_TAG, "onError: " + rsp.getErrorCode());
                    updateData(rsp.getErrorCode());
                } else {
                    Log.v(FactoryTestConst.LOG_TAG, "onTestResult: result = " + rsp.toString());
                    updateData(rsp);
                }
            }
        }
    };

    private void updateData(Object result) {
        initData();
        if (result instanceof FingerSpeedResult) {
            FingerSpeedResult object = (FingerSpeedResult) result;
            mImageQuality = object.getImageQuality();
            mEffectArea = object.getEffectiveArea();
            mCaptureTime = object.getCaptureTime();
            mReduceNoiseTime = object.getReduceBgNoiseTime();
            mAuthTime = object.getAuthTime();
            mTplUpdTime = object.getTplUpdTime();
            mAuthResult = object.getResult();
        } else {
            mErrorCode = ((Integer)result).intValue();
        }
        mFirstLoad = false;
        updateUI();
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
        wm.addView(mFingerpintImage, lp);
    }

    private void hideIcon() {
        WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
        wm.removeView(mFingerpintImage);
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
            if (mv.getAction() == MotionEvent.ACTION_DOWN) {
                if (mFingerprintImageTouchEnable) {
                    testSendFingerDownMsg();
                }
            } else if (mv.getAction() == MotionEvent.ACTION_UP) {
                if (mFingerprintImageTouchEnable) {
                    testSendFingerUpMsg();
                }
            }
            return true;
        }
    }
}
