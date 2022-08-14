
package com.silead.frrfar;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Vibrator;
import android.util.Log;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerFrrFarEnroll;

import android.content.Context;
import android.view.Gravity;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.MotionEvent;

public class EnrollingActivity extends Activity implements View.OnClickListener {
    public static final String EXTRA_KEY_USER_TOKEN = "user_token";
    public static final String EXTRA_KEY_HAND_TOKEN = "hand_token";
    public static final String EXTRA_KEY_FINGER_TOKEN = "finger_token";

    private TextView mEnrollInfoTextView;
    private ImageView mFingerImageView;
    private ImageView mFingerOrigImageView;
    private ImageView mFingerFinalImageView;
    private TextView mImageQulityTextView;
    private TextView mEffectiveAreaTextView;
    private TextView mImageGreyTextView;
    private TextView mNumEnrollInfoTextView;
    private NumberProgressBar mSampleProgressBar;
    private Button mEnrollingFinishBtn;
    private Button mEnrollingPreImageBtn;
    private Button mEnrollingNextFingerBtn;
    private Button mEnrollingPlasticFingerBtn;

    private int mSampleCount;
    private String mUser;
    private int mHand;
    private int mFinger;

    private boolean mEnrollOneFingerInit = true;
    private int mSampleEnrolled = 0;
    private int mImageQuality = 0;
    private int mEffectiveArea = 0;
    private int mImageGreyAvg = 0;
    private int mImageGreyMax = 0;
    private Bitmap mBitmapImage = null;
    private Bitmap mOrigBitmapImage = null;
    private int mErrorCode = 0;
    private int mTplImageCount = -1;
    private String mCurrentImageFilePath = null;

    private FingerManager mFingerManager;
    private Vibrator mVibrator;
    private boolean mIsOptic;
    private int mGatherMode = 0;
    private boolean mNormalImgDisplay;

    private static final int ERR_RE_GET_ONE_IMAGE = -999;

    private WindowManager.LayoutParams lp;
    private FingerprintTestView mFingerprintImage = null;
    private int mFingerprintImageId;
    private FingerImageSize mFingerprintImageSize = new FingerImageSize();
    private int mFingerprintImageCenter;
    private String mFingerprintImageName = null;

    private int mEnrollCountPerDown;
    private boolean mEnrollImgDIY;
    private int mEnrollImgDelay;

    private boolean mImageTouchEnable = true;
    private boolean mFinish = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mIsOptic = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                FingerSettingsConst.FP_SETTINGS_OTHER_IS_OPTIC_NAME, FingerSettingsConst.FP_SETTINGS_OTHER_IS_OPTIC_DEFAULT);
        if (mIsOptic) {
            setContentView(R.layout.enrolling_main_optic);
        } else {
            setContentView(R.layout.enrolling_main);
        }

        Intent intent = getIntent();
        if (intent != null && intent.hasExtra(EXTRA_KEY_USER_TOKEN)) {
            mSampleEnrolled = 0;
            mSampleCount = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_NAME, FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_DEFAULT);
            mUser = intent.getStringExtra(EXTRA_KEY_USER_TOKEN);
            mHand = intent.getIntExtra(EXTRA_KEY_HAND_TOKEN, FingerSettingsConst.ENROLL_ENV_NEXT_HAND_DEFAULT);
            mFinger = intent.getIntExtra(EXTRA_KEY_FINGER_TOKEN, FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_DEFAULT);
        } else {
            finish();
            return;
        }

        mNormalImgDisplay = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                FingerSettingsConst.FP_SETTINGS_OTHER_IMG_DISPLAY_NAME, FingerSettingsConst.FP_SETTINGS_OTHER_IMG_DISPLAY_DEFAULT);

        mGatherMode = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                FingerSettingsConst.FP_SETTINGS_OTHER_GATHER_FINGER_NAME, FingerSettingsConst.FP_SETTINGS_OTHER_GATHER_FINGER_DEFAULT);

        mFingerprintImageCenter = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_Y_NAME, FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_Y_DEFAULT);
        mFingerprintImageName = SharedPreferencesData.loadDataString(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_NAME);
        if (mFingerprintImageName == null || mFingerprintImageName.isEmpty()) {
            mFingerprintImageName = FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT;
        }
        updateIconInfo();

        mEnrollCountPerDown = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                FingerSettingsConst.ENROLL_SETTING_COUNT_NAME, FingerSettingsConst.ENROLL_SETTING_COUNT_DEFAULT);
        mEnrollImgDIY = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                FingerSettingsConst.ENROLL_SETTING_IMG_DIY_NAME, FingerSettingsConst.ENROLL_SETTING_IMG_DIY_DEFAULT);
        mEnrollImgDelay = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                FingerSettingsConst.ENROLL_SETTING_IMG_DELAY_NAME, FingerSettingsConst.ENROLL_SETTING_IMG_DELAY_DEFAULT);
        if ((mEnrollCountPerDown < FingerSettingsConst.ENROLL_SETTING_DISABLE) || (mEnrollCountPerDown > FingerSettingsConst.ENROLL_SETTING_COUNT_MAX)) {
            mEnrollCountPerDown = FingerSettingsConst.ENROLL_SETTING_DISABLE;
        }
        if (mEnrollCountPerDown > FingerSettingsConst.ENROLL_SETTING_DISABLE) {
            mGatherMode = FingerSettingsConst.OTHER_GATHER_MODE_ORIG;
        }
        if ((mEnrollCountPerDown == FingerSettingsConst.ENROLL_SETTING_DISABLE) || (!mEnrollImgDIY)) {
            mEnrollImgDelay = 0;
        }

        mImageTouchEnable = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_TOUCH_ENABLE_NAME, FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_TOUCH_ENABLE_DEFAULT);

        mVibrator = (Vibrator)this.getSystemService(Context.VIBRATOR_SERVICE);
        mFingerManager = FingerManager.getDefault(this);

        mEnrollInfoTextView = (TextView) findViewById(R.id.enrolling_info_msg);
        mFingerImageView = (ImageView) findViewById(R.id.enrolling_finger_imageview);
        mFingerOrigImageView = (ImageView) findViewById(R.id.enrolling_finger_ori_imageview);
        mFingerFinalImageView = (ImageView) findViewById(R.id.enrolling_finger_final_imageview);
        mImageQulityTextView = (TextView) findViewById(R.id.enrolling_image_quality_textview);
        mEffectiveAreaTextView = (TextView) findViewById(R.id.enrolling_effective_area_textview);
        mImageGreyTextView = (TextView) findViewById(R.id.enrolling_image_grey_textview);
        mNumEnrollInfoTextView = (TextView) findViewById(R.id.enrolling_num_msg);
        mSampleProgressBar = (NumberProgressBar) findViewById(R.id.enrolling_sample_progressbar);
        mEnrollingFinishBtn = (Button) findViewById(R.id.enrolling_finish_btn);
        mEnrollingPreImageBtn = (Button) findViewById(R.id.enrolling_reenroll_btn);
        mEnrollingNextFingerBtn = (Button) findViewById(R.id.enrolling_next_btn);
        mEnrollingPlasticFingerBtn = (Button) findViewById(R.id.enrolling_plastic_finger_btn);

        if (mSampleProgressBar != null) {
            mSampleProgressBar.setMax(mSampleCount);
            mSampleProgressBar.setProgress(0);
        }
        if (mFingerImageView != null) {
            mFingerImageView.setScaleType(ImageView.ScaleType.FIT_CENTER);
        }
        if (mFingerOrigImageView != null) {
            mFingerOrigImageView.setScaleType(ImageView.ScaleType.CENTER);
            if (!mNormalImgDisplay || mGatherMode == FingerSettingsConst.OTHER_GATHER_MODE_FINAL) {
                mFingerOrigImageView.setVisibility(View.GONE);
            }
        }
        if (mFingerFinalImageView != null) {
            mFingerFinalImageView.setScaleType(ImageView.ScaleType.CENTER);
            if (!mNormalImgDisplay || mGatherMode == FingerSettingsConst.OTHER_GATHER_MODE_ORIG) {
                mFingerFinalImageView.setVisibility(View.GONE);
            }
        }

        if (mEnrollingFinishBtn != null) {
            mEnrollingFinishBtn.setOnClickListener(this);
            if (FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
                mEnrollingFinishBtn.setVisibility(View.VISIBLE);
            } else {
                mEnrollingFinishBtn.setVisibility(View.GONE);
            }
        }
        if (mEnrollingPreImageBtn != null) {
            mEnrollingPreImageBtn.setOnClickListener(this);
        }
        if (mEnrollingNextFingerBtn != null) {
            mEnrollingNextFingerBtn.setOnClickListener(this);
        }
        if (mEnrollingPlasticFingerBtn != null) {
            if (!mIsOptic) {
                mEnrollingPlasticFingerBtn.setVisibility(View.GONE);
            }

            mEnrollingPlasticFingerBtn.setOnTouchListener(new OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    if (v.getId() == R.id.enrolling_plastic_finger_btn) {
                        if (event.getAction() == MotionEvent.ACTION_DOWN) {
                            testSendFingerDownMsg();
                        } else if (event.getAction() == MotionEvent.ACTION_UP) {
                            testSendFingerUpMsg();
                        }
                    }
                    return true;
                }
            });
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        mFinish = false;
        initData();
        updateUI();

        if (mIsOptic) {
            initIcon();
            showIcon();
        }

        testGetImage();
    }

    @Override
    public void onPause() {
        super.onPause();

        mFinish = true;
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

        setNextEnv();
    }

    @Override
    public void onClick(View v) {
        if (v == mEnrollingFinishBtn) {
            finish();
        } else if (v == mEnrollingPreImageBtn) {
            if (mSampleEnrolled > mTplImageCount) {
                mSampleEnrolled--;
                mErrorCode = ERR_RE_GET_ONE_IMAGE;

                String fileName = FingerSettingsConst.getDataFileName(mSampleEnrolled - 1, 0);
                File folder = null;
                if (mGatherMode == FingerSettingsConst.OTHER_GATHER_MODE_ORIG) {
                    folder = getDataFile(mUser, mHand, mFinger, false, true);
                } else {
                    folder = getDataFile(mUser, mHand, mFinger, false, false);
                }
                File file = new File(folder, fileName);
                if (file.exists()) {
                    mBitmapImage = BitmapFactory.decodeFile(file.getAbsolutePath(), null);
                }

                if (mGatherMode != FingerSettingsConst.OTHER_GATHER_MODE_FINAL) {
                    File orig_folder = getDataFile(mUser, mHand, mFinger, false, true);
                    File orig_file = new File(orig_folder, fileName);
                    if (orig_file.exists()) {
                        mOrigBitmapImage = BitmapFactory.decodeFile(orig_file.getAbsolutePath(), null);
                    }
                }

                updateUI();
            }
        } else if (v == mEnrollingNextFingerBtn) {
            setNextEnv();

            initData();
            updateUI();

            testGetImage();
        }
    }

    private String getErrorMsg(int err) {
        String errString = "unknow";
        if (err == FingerManager.TEST_RESULT_OK) {
            errString = getString(R.string.enrolling_image_save_succeed, mCurrentImageFilePath);
        } else if (err == FingerManager.TEST_RESULT_IMAGE_SAVE_FAILED) {
            errString = getString(R.string.enrolling_image_save_failed, mCurrentImageFilePath);
        } else if (err == FingerManager.TEST_RESULT_DATA_IMCOMPLITE) {
            errString = getString(R.string.globle_err_data_invalide);
        } else if (err == FingerManager.TEST_RESULT_SERVICE_FAILED) {
            errString = getString(R.string.globle_err_not_found_service);
        } else if (err == FingerManager.TEST_RESULT_BAD_PARAM) {
            errString = getString(R.string.globle_err_bad_param);
        } else if (err == FingerManager.TEST_RESULT_MOVE_TOO_FAST || err == FingerManager.TEST_RESULT_NO_FINGER) {
            errString = getString(R.string.globle_err_move_to_fast);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_SAME_AREA) {
            errString = getString(R.string.globle_err_enroll_same_area);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_QUALITY_FAILED) {
            errString = getString(R.string.globle_err_enroll_quality_fail);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_COVERAREA_FAILED) {
            errString = getString(R.string.globle_err_enroll_coverarea_fail);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_QUALITY_COVERAREA_FAILED) {
            errString = getString(R.string.globle_err_enroll_quality_coverarea_fail);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_FAKE_FINGER) {
            errString = getString(R.string.globle_err_enroll_fake_finger);
        } else if (err == FingerManager.TEST_RESULT_ENROLL_GAIN_IMPROVE_TIMEOUT) {
            errString = getString(R.string.globle_err_enroll_gain_improve_timeout);
        } else if (err == FingerManager.TEST_RESULT_CANCELED) {
            errString = getString(R.string.globle_err_canceled);
        } else if (err == ERR_RE_GET_ONE_IMAGE) {
            errString = getString(R.string.enrolling_image_re_enroll_last_image, mSampleEnrolled + 1);
        } else {
            errString = getString(R.string.unknow);
        }
        return errString;
    }

    private void updateUI() {
        if (mEnrollInfoTextView != null) {
            if (mEnrollOneFingerInit) {
                String value = mUser;
                int fingers = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                        FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME, FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT);
                String fingerFolder = FingerSettingsConst.getFingerPath(mHand, fingers, mFinger);
                value += fingerFolder;

                mEnrollInfoTextView.setText(getString(R.string.enrolling_begin_enroll_info, value));
            } else {
                mEnrollInfoTextView.setText(getErrorMsg(mErrorCode));
            }
        }
        if (mFingerImageView != null) {
            if (mBitmapImage != null) {
                mFingerImageView.setImageBitmap(mBitmapImage);
            }
        }
        if (mFingerOrigImageView != null) {
            if (mOrigBitmapImage != null) {
                mFingerOrigImageView.setImageBitmap(mOrigBitmapImage);
            }
        }
        if (mFingerFinalImageView != null) {
            if (mBitmapImage != null) {
                mFingerFinalImageView.setImageBitmap(mBitmapImage);
            }
        }
        if (mImageQulityTextView != null) {
            mImageQulityTextView.setText(String.valueOf(mImageQuality));
        }
        if (mEffectiveAreaTextView != null) {
            mEffectiveAreaTextView.setText(String.valueOf(mEffectiveArea));
        }
        if (mImageGreyTextView != null) {
            mImageGreyTextView.setText(mImageGreyAvg + ":" + mImageGreyMax);
        }
        if (mNumEnrollInfoTextView != null) {
            if (mSampleEnrolled < mSampleCount) {
                mNumEnrollInfoTextView.setText(getString(R.string.enrolling_image_num_title, mSampleEnrolled + 1));
            } else {
                mNumEnrollInfoTextView.setText(getString(R.string.enrolling_image_num_finish_title));
            }
        }
        if (mSampleProgressBar != null) {
            mSampleProgressBar.setProgress(mSampleEnrolled);
        }

        if (mEnrollingPreImageBtn != null) {
            if (mTplImageCount < 0 || mSampleEnrolled <= mTplImageCount) {
                mEnrollingPreImageBtn.setEnabled(false);
            } else {
                mEnrollingPreImageBtn.setEnabled(true);
            }
        }
        if (mEnrollingNextFingerBtn != null) {
            if (mSampleEnrolled == mSampleCount) {
                mEnrollingNextFingerBtn.setEnabled(true);
            } else {
                mEnrollingNextFingerBtn.setEnabled(false);
            }
        }
    }

    private void initData() {
        getDataFile(mUser, mHand, mFinger, true, false);
        File orgFolder = getDataFile(mUser, mHand, mFinger, true, true);
        EnrollResultXml.init(this, orgFolder, FingerSettingsConst.getEnrollResultFileName());
        mSampleEnrolled = 0;
        mImageQuality = 0;
        mEffectiveArea = 0;
        mImageGreyAvg = 0;
        mImageGreyMax = 0;
        mBitmapImage = null;
        mOrigBitmapImage = null;
        mErrorCode = 0;
        mTplImageCount = -1;
        mEnrollOneFingerInit = true;
    }

    private void testGetImage() {
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testGetImage: ");
        }

        if (mFingerManager != null) {
            mFingerManager.testGetImage(mGatherMode, mEnrollCountPerDown, mEnrollImgDelay, mTestCmdCallback);
        }
    }

    public void testFinish() {
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testFinish: finish");
        }
        if (mFingerManager != null) {
            mFingerManager.testFinish(mTestCmdCallback);
        }
    }

    private void testSendFingerDownMsg() {
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testSendFingerDownMsg: ");
        }
        if (mFingerManager != null) {
            mFingerManager.testSendFingerDownMsg(mTestCmdCallback);
        }
    }

    public void testSendFingerUpMsg() {
        if (FingerSettingsConst.LOG_DBG) {
            Log.v(FingerSettingsConst.LOG_TAG, "testSendFingerUpMsg");
        }
        if (mFingerManager != null) {
            mFingerManager.testSendFingerUpMsg(mTestCmdCallback);
        }
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_GET_IMAGE && result instanceof FingerFrrFarEnroll) {
                if (!mFinish) {
                    updateData(result);
                }
            }
        }
    };

    private void updateFpData(FingerFrrFarEnroll enrollResult, boolean original, int subIndex) {
        mErrorCode = enrollResult.getErrCode();
        mImageQuality = enrollResult.getImageQuality();
        mEffectiveArea = enrollResult.getEffectiveArea();
        mImageGreyAvg = enrollResult.getGreyAvg();
        mImageGreyMax = enrollResult.getGreyMax();
        mBitmapImage = BitmapFactory.decodeByteArray(enrollResult.getData(), 0, enrollResult.getData().length);
        if (mErrorCode == FingerManager.TEST_RESULT_OK) {
            if (!enrollResult.isTplImage() && mTplImageCount < 0) {
                mTplImageCount = mSampleEnrolled;
                Log.e(FingerSettingsConst.LOG_TAG, "mTplImageCount = " + mTplImageCount);
            }

            if (!saveDataFile(mSampleEnrolled, enrollResult.getData(), original, subIndex)) {
                mErrorCode = FingerManager.TEST_RESULT_IMAGE_SAVE_FAILED;
            } else {
                if (mEnrollCountPerDown <= FingerSettingsConst.ENROLL_SETTING_DISABLE) {
                    mSampleEnrolled++;
                } else {
                    EnrollResultXml.writeImgInfo(mCurrentImageFilePath, mImageQuality, mEffectiveArea, mImageGreyAvg, mImageGreyMax);
                    if (subIndex + 1 >= mEnrollCountPerDown) {
                        mSampleEnrolled++;
                    }
                }
            }
        }
    }

    private void updateData(Object result) {
        if (mSampleEnrolled >= mSampleCount) {
            return;
        }

        mEnrollOneFingerInit = false;

        if (result instanceof FingerFrrFarEnroll) {
            FingerFrrFarEnroll enrollResult = (FingerFrrFarEnroll) result;
            if (FingerSettingsConst.LOG_DBG) {
                Log.v(FingerSettingsConst.LOG_TAG, "updateData: " + enrollResult.toString());
            }

            if (enrollResult.getErrCode() >= FingerManager.TEST_RESULT_OK) {
                int subIndex = enrollResult.getStep();
                if (enrollResult.isImgOrig()) { // original image
                    if (mGatherMode == FingerSettingsConst.OTHER_GATHER_MODE_ORIG) {
                        updateFpData(enrollResult, true, subIndex);
                    } else if (mGatherMode == FingerSettingsConst.OTHER_GATHER_MODE_ALL) {
                        if (enrollResult.getErrCode() != FingerManager.TEST_RESULT_OK) {
                            mErrorCode = enrollResult.getErrCode();
                        } else {
                            saveDataFile(mSampleEnrolled, enrollResult.getData(), true, 0);
                        }
                    }
                    if (mGatherMode != FingerSettingsConst.OTHER_GATHER_MODE_FINAL) {
                        mOrigBitmapImage = BitmapFactory.decodeByteArray(enrollResult.getData(), 0, enrollResult.getData().length);
                    }
                } else {
                    if (mGatherMode != FingerSettingsConst.OTHER_GATHER_MODE_ORIG) {
                        updateFpData(enrollResult, false, 0);
                    }
                }
            } else {
                if (enrollResult.getErrCode() == FingerManager.TEST_RESULT_IMAGE_ICON_CHANGE) {
                    updateEachIcon(enrollResult.getStep());
                } else {
                    mErrorCode = enrollResult.getErrCode();
                }
            }
        }
        updateUI();
    }

    private boolean saveDataFile(int index, byte[] data, boolean original, int subIndex) {
        boolean saved = false;
        String fileName = FingerSettingsConst.getDataFileName(index, subIndex);

        File folder = getDataFile(mUser, mHand, mFinger, false, original);
        File file = new File(folder, fileName);
        if (file.exists()) {
            file.delete();
        }
        try {
            FileOutputStream fos = new FileOutputStream(file);
            fos.write(data);
            fos.flush();
            fos.close();
            saved = true;
        } catch (FileNotFoundException e) {
        } catch (IOException e) {
        }
        mCurrentImageFilePath = file.getAbsolutePath();
        return saved;
    }

    private File getDataFile(String user, int hand, int finger, boolean init, boolean original) {
        int fingers = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME, FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT);
        String fingerFolder = FingerSettingsConst.getFingerPath(hand, fingers, finger);

        File dataDir = FingerSettingsConst.getDataSavePath(this, original);
        File userDataDir = new File(dataDir, user);
        if (!userDataDir.exists()) {
            if (!userDataDir.mkdir()) {
                Log.v(FingerSettingsConst.LOG_TAG, "Cannot make directory: " + userDataDir.getAbsolutePath());
                return null;
            }
        }

        File fingerDataDir = new File(userDataDir, fingerFolder);
        if (fingerDataDir.exists()) {
            if (init) {
                deleteFiles(fingerDataDir);
            }
        } else {
            if (!fingerDataDir.mkdir()) {
                Log.v(FingerSettingsConst.LOG_TAG, "Cannot make directory: " + fingerDataDir.getAbsolutePath());
                return null;
            }
        }

        return fingerDataDir;
    }

    private void deleteFiles(File oldPath) {
        if (oldPath.isDirectory()) {
            File[] files = oldPath.listFiles();
            if (files != null && files.length > 0) {
                for (File file : files) {
                    deleteFiles(file);
                }
            }
        } else {
            oldPath.delete();
        }
    }

    private void setNextEnv() {
        EnrollResultXml.finish(true);
        if (mSampleEnrolled >= mSampleCount) {
            int fingers = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME, FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT);

            boolean lastfinger = FingerSettingsConst.isLastFinger(fingers, mFinger);
            mUser = FingerSettingsConst.getNextUser(mUser, mHand, lastfinger);
            mHand = FingerSettingsConst.getNextHand(mHand, lastfinger);
            mFinger = FingerSettingsConst.getNextFinger(mFinger, lastfinger);

            if (FingerSettingsConst.LOG_DBG) {
                Log.v(FingerSettingsConst.LOG_TAG, "next env: mUser=" + mUser + ",mHand=" + mHand + ",mFinger=" + mFinger);
            }

            SharedPreferencesData.saveDataString(this, FingerSettingsConst.ENROLL_ENV_CONFIG,
                    FingerSettingsConst.ENROLL_ENV_NEXT_USERID_NAME, mUser);
            SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_ENV_CONFIG,
                    FingerSettingsConst.ENROLL_ENV_NEXT_HAND_NAME, mHand);
            SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_ENV_CONFIG,
                    FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_NAME, mFinger);
        }
    }

    private void updateEachIcon(int index) {
        if (mEnrollCountPerDown > FingerSettingsConst.ENROLL_SETTING_DISABLE && mEnrollImgDIY) {
            index = index%mEnrollCountPerDown;
            String enrollIdImageName = SharedPreferencesData.loadDataString(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                    FingerSettingsConst.ENROLL_SETTING_ID_IMAGE_NAME_PERFIX + index);
            if (enrollIdImageName == null || enrollIdImageName.isEmpty()) {
                enrollIdImageName = SharedPreferencesData.loadDataString(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                    FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_NAME);
            }
            if (enrollIdImageName == null || enrollIdImageName.isEmpty()) {
                enrollIdImageName = FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT;
            }
            updateIcon(enrollIdImageName);
        }
    }

    private void updateIcon(String imageName) {
        if (mFingerprintImage != null) {
            if (!mFingerprintImageName.equals(imageName)) {
                mFingerprintImageName = imageName;
                updateIconInfo();

                lp.width = mFingerprintImageSize.width;
                lp.height = mFingerprintImageSize.height;
                lp.y = mFingerprintImageCenter - mFingerprintImageSize.height / 2;

                hideIcon();
                mFingerprintImage.updateIcon(mFingerprintImageId);
                showIcon();
            }
        }
    }

    private void updateIconInfo() {
        mFingerprintImageId = FingerSettingsConst.getDrawableId(this, getPackageName(), mFingerprintImageName, FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT);
        FingerSettingsConst.getDrawableSize(this, mFingerprintImageId, mFingerprintImageSize);

        int imageRadius = mFingerprintImageSize.height / 2;

        if (mFingerprintImageCenter < imageRadius) {
            mFingerprintImageCenter = imageRadius;
        }
    }

    private void initIcon() {
        lp = new WindowManager.LayoutParams();
        lp.width = mFingerprintImageSize.width;
        lp.height = mFingerprintImageSize.height;
        lp.y = mFingerprintImageCenter - mFingerprintImageSize.height / 2;
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

        mFingerprintImage = new FingerprintTestView(getApplicationContext());
    }

    private void showIcon() {
        if (mFingerprintImage != null) {
            WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
            wm.addView(mFingerprintImage, lp);
        }
    }

    private void hideIcon() {
        if (mFingerprintImage != null) {
            WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
            wm.removeView(mFingerprintImage);
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
            setImageDrawable(context.getDrawable(mFingerprintImageId));
        }

        @Override
        public boolean onTouchEvent(MotionEvent mv) {
            if (mv.getAction() == MotionEvent.ACTION_DOWN) {
                if (mImageTouchEnable) {
                    testSendFingerDownMsg();
                }
            } else if (mv.getAction() == MotionEvent.ACTION_UP) {
                if (mImageTouchEnable) {
                    testSendFingerUpMsg();
                }
            }
            return true;
        }

        public void updateIcon(int drawableId) {
            setImageDrawable(mContext.getDrawable(drawableId));
        }
    }
}
