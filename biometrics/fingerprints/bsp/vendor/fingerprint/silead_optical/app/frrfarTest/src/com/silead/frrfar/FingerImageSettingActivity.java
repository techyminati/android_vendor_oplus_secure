
package com.silead.frrfar;

import android.app.Activity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TextView;

import com.silead.manager.FingerManager;

import android.content.Context;
import android.view.Gravity;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.WindowManager;
import android.widget.ImageView;

public class FingerImageSettingActivity extends Activity implements View.OnClickListener {
    private Button mFingerImageBackBtn;
    private Button mFingerImageSaveBtn;
    private Button mFingerImagePreBtn;
    private Button mFingerImageNextBtn;
    private TextView mFingerImageYTextView;
    private Button mFingerImagePlusBtn;
    private Button mFingerImageSubBtn;
    private CheckBox mFingerImageTouchCBox;

    private FingerManager mFingerManager;

    private WindowManager.LayoutParams lp;
    private DisplayMetrics dm;
    private FingerprintTestView mFingerprintImage = null;
    private int mFingerprintImageId;
    private FingerImageSize mFingerprintImageSize = new FingerImageSize();
    private int mFingerprintImageCenter;
    private String mFingerprintImageName = null;
    private CharSequence[] mFingerprintImageNameArray;
    private boolean mFingerprintImageTouchEnable = true;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.finger_image_main_optic);

        dm = getResources().getDisplayMetrics();

        mFingerImageBackBtn = (Button) findViewById(R.id.finger_image_setting_back);
        mFingerImageSaveBtn = (Button) findViewById(R.id.finger_image_setting_save);
        mFingerImagePreBtn = (Button) findViewById(R.id.finger_image_setting_pre_btn);
        mFingerImageNextBtn = (Button) findViewById(R.id.finger_image_setting_next_btn);
        mFingerImageYTextView = (TextView) findViewById(R.id.finger_image_layout_y);
        mFingerImagePlusBtn = (Button) findViewById(R.id.finger_image_setting_plus_btn);
        mFingerImageSubBtn = (Button) findViewById(R.id.finger_image_setting_sub_btn);
        mFingerImageTouchCBox = (CheckBox) findViewById(R.id.finger_image_touch_enable);

        if (mFingerImageBackBtn != null) {
            mFingerImageBackBtn.setOnClickListener(this);
            if (FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
                mFingerImageBackBtn.setVisibility(View.VISIBLE);
            } else {
                mFingerImageBackBtn.setVisibility(View.GONE);
            }
        }
        if (mFingerImageSaveBtn != null) {
            mFingerImageSaveBtn.setOnClickListener(this);
        }
        if (mFingerImagePreBtn != null) {
            mFingerImagePreBtn.setOnClickListener(this);
        }
        if (mFingerImageNextBtn != null) {
            mFingerImageNextBtn.setOnClickListener(this);
        }
        if (mFingerImagePlusBtn != null) {
            mFingerImagePlusBtn.setOnClickListener(this);
        }
        if (mFingerImageSubBtn != null) {
            mFingerImageSubBtn.setOnClickListener(this);
        }

        mFingerprintImageTouchEnable = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_TOUCH_ENABLE_NAME, FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_TOUCH_ENABLE_DEFAULT);
        if (mFingerImageTouchCBox != null) {
            mFingerImageTouchCBox.setChecked(mFingerprintImageTouchEnable);
            mFingerImageTouchCBox.setOnClickListener(this);
        }

        mFingerprintImageCenter = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_Y_NAME, FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_Y_DEFAULT);
        mFingerprintImageName = SharedPreferencesData.loadDataString(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_NAME);
        if (mFingerprintImageName == null || mFingerprintImageName.isEmpty()) {
            mFingerprintImageName = FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT;
        }
        updateIconInfo();

        mFingerprintImageNameArray = getResources().getStringArray(R.array.finger_image_name_optic);

        if (mFingerImageYTextView != null) {
            mFingerImageYTextView.setText(Integer.toString(mFingerprintImageCenter));
        }

        mFingerManager = FingerManager.getDefault(this);
    }

    @Override
    public void onResume() {
        super.onResume();

        initIcon();
        showIcon();
        updateIcon(0);
    }

    @Override
    public void onPause() {
        super.onPause();

        hideIcon();

        finish();
    }

    @Override
    public void onClick(View v) {
        if (v == mFingerImageBackBtn) {
            finish();
        } else if (v == mFingerImageSaveBtn) {
            SharedPreferencesData.saveDataString(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                    FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_NAME, mFingerprintImageName);
            SharedPreferencesData.saveDataInt(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                    FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_Y_NAME, mFingerprintImageCenter);
            finish();
        } else if (v == mFingerImagePreBtn) {
            updateIcon(-1);
        } else if (v == mFingerImageNextBtn) {
            updateIcon(1);
        } else if (v == mFingerImagePlusBtn) {
            updateIconPosition(1);
        } else if (v == mFingerImageSubBtn) {
            updateIconPosition(-1);
        } else if (v == mFingerImageTouchCBox) {
            mFingerprintImageTouchEnable = mFingerImageTouchCBox.isChecked();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                    FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_TOUCH_ENABLE_NAME, mFingerprintImageTouchEnable);
        }
    }

    private void updateIconPosition(int offset) {
        if (mFingerprintImage != null) {
            int imageRadius = mFingerprintImageSize.height / 2;
            mFingerprintImageCenter += offset;
            if (mFingerprintImageCenter < imageRadius) {
                mFingerprintImageCenter = imageRadius;
            } else if (mFingerprintImageCenter > (dm.heightPixels / 2) - imageRadius) {
                mFingerprintImageCenter = (dm.heightPixels / 2) - imageRadius;
            }
            lp.y = mFingerprintImageCenter - imageRadius;
            WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
            wm.updateViewLayout(mFingerprintImage, lp);

            if (mFingerImageYTextView != null) {
                mFingerImageYTextView.setText(Integer.toString(mFingerprintImageCenter));
            }
        }
    }

    private void updateIcon(int offset) {
        int index = FingerSettingsConst.getValueIndex(mFingerprintImageName, mFingerprintImageNameArray, 0);
        if ((offset < 0) && (index > 0)) { // pre img
            index--;
        } else if ((offset > 0) && (index < mFingerprintImageNameArray.length - 1)) { // next img
            index++;
        }

        if (index == 0) {
            mFingerImagePreBtn.setEnabled(false);
        } else {
            mFingerImagePreBtn.setEnabled(true);
        }
        if (index + 1 >= mFingerprintImageNameArray.length) {
            mFingerImageNextBtn.setEnabled(false);
        } else {
            mFingerImageNextBtn.setEnabled(true);
        }

        if (mFingerprintImage != null) {
            mFingerprintImageName = mFingerprintImageNameArray[index].toString();
            updateIconInfo();

            lp.width = mFingerprintImageSize.width;
            lp.height = mFingerprintImageSize.height;
            lp.y = mFingerprintImageCenter - mFingerprintImageSize.height / 2;

            if (mFingerImageYTextView != null) {
                mFingerImageYTextView.setText(Integer.toString(mFingerprintImageCenter));
            }

            hideIcon();
            mFingerprintImage.updateIcon(mFingerprintImageId);
            showIcon();
            //WindowManager wm = (WindowManager)getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
            //wm.updateViewLayout(mFingerprintImage, lp);

            Log.v(FingerSettingsConst.LOG_TAG, "height=" + mFingerprintImageSize.height + ",width=" + mFingerprintImageSize.width);
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
        private int y_down;

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
        public boolean onTouchEvent(MotionEvent event) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN: {
                    y_down = (int) event.getRawY();
                    if (mFingerprintImageTouchEnable) {
                        testSendFingerDownMsg();
                    }
                    break;
                }
                case MotionEvent.ACTION_MOVE: {
                    int y = (int) event.getRawY();
                    updateIconPosition(y_down - y);
                    y_down = y;
                    break;
                }
                case MotionEvent.ACTION_UP: {
                    if (mFingerprintImageTouchEnable) {
                        testSendFingerUpMsg();
                    }
                    break;
                }
            }
            return true;
        }

        public void updateIcon(int drawableId) {
            setImageDrawable(mContext.getDrawable(drawableId));
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
    };
}
