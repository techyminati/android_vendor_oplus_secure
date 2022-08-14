
package com.silead.frrfar;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.TextView;

import android.content.Context;
import android.widget.ImageView;

public class FingerEnrollSettingActivity extends Activity implements View.OnClickListener {
    private TextView mFingerEnrollCount;
    private Button mFingerEnrollCountSubBtn;
    private Button mFingerEnrollCountPlusbBtn;
    private TextView mFingerEnrollId;
    private Button mFingerEnrollIdSubBtn;
    private Button mFingerEnrollIdPlusbBtn;
    private ImageView mFingerEnrollImage;
    private Button mFingerEnrollImageSubBtn;
    private Button mFingerEnrollImagePlusbBtn;
    private TextView mFingerEnrollDelay;
    private Button mFingerEnrollDelaySubBtn;
    private Button mFingerEnrollDelayPlusbBtn;
    private CheckBox mFingerEnrollImageDIYCBox;
    private LinearLayout mFingerEnrollImageDIYLayout;
    private LinearLayout mFingerEnrollImageIdLayout;
    private LinearLayout mFingerEnrollImageIconLayout;
    private LinearLayout mFingerEnrollImageDelayLayout;

    private Button mFingerEnrollBackBtn;
    private Button mFingerEnrollRestoreBtn;

    private int mEnrollCountPerDown;
    private boolean mEnrollImageDIY;
    private int mEnrollImageDelay;
    private int mEnrollId;
    private String mEnrollIdImageName;
    private CharSequence[] mFingerprintImageNameArray;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.finger_enroll_main_optic);

        mFingerEnrollCount = (TextView) findViewById(R.id.finger_enroll_setting_count);
        mFingerEnrollCountSubBtn = (Button) findViewById(R.id.finger_enroll_setting_count_sub_btn);
        mFingerEnrollCountPlusbBtn = (Button) findViewById(R.id.finger_enroll_setting_count_plus_btn);
        mFingerEnrollId = (TextView) findViewById(R.id.finger_enroll_setting_id);
        mFingerEnrollIdSubBtn = (Button) findViewById(R.id.finger_enroll_setting_id_sub_btn);
        mFingerEnrollIdPlusbBtn = (Button) findViewById(R.id.finger_enroll_setting_id_plus_btn);
        mFingerEnrollImage = (ImageView) findViewById(R.id.finger_enroll_setting_image);
        mFingerEnrollImageSubBtn = (Button) findViewById(R.id.finger_enroll_setting_image_sub_btn);
        mFingerEnrollImagePlusbBtn = (Button) findViewById(R.id.finger_enroll_setting_image_plus_btn);
        mFingerEnrollImageDIYCBox = (CheckBox) findViewById(R.id.finger_enroll_img_diy);
        mFingerEnrollDelay = (TextView) findViewById(R.id.finger_enroll_setting_delay);
        mFingerEnrollDelaySubBtn = (Button) findViewById(R.id.finger_enroll_setting_delay_sub_btn);
        mFingerEnrollDelayPlusbBtn = (Button) findViewById(R.id.finger_enroll_setting_delay_plus_btn);

        mFingerEnrollImageDIYLayout = (LinearLayout) findViewById(R.id.finger_enroll_setting_diy_layout);
        mFingerEnrollImageIdLayout = (LinearLayout) findViewById(R.id.finger_enroll_setting_id_layout);
        mFingerEnrollImageIconLayout = (LinearLayout) findViewById(R.id.finger_enroll_setting_image_layout);
        mFingerEnrollImageDelayLayout = (LinearLayout) findViewById(R.id.finger_enroll_setting_delay_layout);

        mFingerEnrollBackBtn = (Button) findViewById(R.id.finger_image_setting_back);
        mFingerEnrollRestoreBtn = (Button) findViewById(R.id.finger_image_setting_restore);

        if (mFingerEnrollCountSubBtn != null) {
            mFingerEnrollCountSubBtn.setOnClickListener(this);
        }

        if (mFingerEnrollCountPlusbBtn != null) {
            mFingerEnrollCountPlusbBtn.setOnClickListener(this);
        }

        if (mFingerEnrollIdSubBtn != null) {
            mFingerEnrollIdSubBtn.setOnClickListener(this);
        }

        if (mFingerEnrollIdPlusbBtn != null) {
            mFingerEnrollIdPlusbBtn.setOnClickListener(this);
        }

        if (mFingerEnrollImageSubBtn != null) {
            mFingerEnrollImageSubBtn.setOnClickListener(this);
        }

        if (mFingerEnrollImagePlusbBtn != null) {
            mFingerEnrollImagePlusbBtn.setOnClickListener(this);
        }

        if (mFingerEnrollImageDIYCBox != null) {
            mFingerEnrollImageDIYCBox.setOnClickListener(this);
        }

        if (mFingerEnrollDelaySubBtn != null) {
            mFingerEnrollDelaySubBtn.setOnClickListener(this);
        }

        if (mFingerEnrollDelayPlusbBtn != null) {
            mFingerEnrollDelayPlusbBtn.setOnClickListener(this);
        }

        if (mFingerEnrollBackBtn != null) {
            mFingerEnrollBackBtn.setOnClickListener(this);
            if (FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
                mFingerEnrollBackBtn.setVisibility(View.VISIBLE);
            } else {
                mFingerEnrollBackBtn.setVisibility(View.GONE);
            }
        }

        if (mFingerEnrollRestoreBtn != null) {
            mFingerEnrollRestoreBtn.setOnClickListener(this);
        }

        mFingerprintImageNameArray = getResources().getStringArray(R.array.finger_image_name_optic);
        initUI();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onClick(View v) {
        if (v == mFingerEnrollBackBtn) {
            finish();
        } else if (v == mFingerEnrollRestoreBtn) {
            clearData();
        } else if (v == mFingerEnrollCountSubBtn) {
            updateCount(-1);
        } else if (v == mFingerEnrollCountPlusbBtn) {
            updateCount(1);
        } else if (v == mFingerEnrollIdSubBtn) {
            updateId(-1);
        } else if (v == mFingerEnrollIdPlusbBtn) {
            updateId(1);
        } else if (v == mFingerEnrollImageSubBtn) {
            updateImage(-1);
        } else if (v == mFingerEnrollImagePlusbBtn) {
            updateImage(1);
        } else if (v == mFingerEnrollDelaySubBtn) {
            updateDelay(-1);
        } else if (v == mFingerEnrollDelayPlusbBtn) {
            updateDelay(1);
        } else if (v == mFingerEnrollImageDIYCBox) {
            updateDIY(mFingerEnrollImageDIYCBox.isChecked());
        }
    }

    private void updateCount(int offset) {
        mEnrollCountPerDown += offset;
        if (mEnrollCountPerDown < 0) {
            mEnrollCountPerDown = 0;
        } else if (mEnrollCountPerDown > FingerSettingsConst.ENROLL_SETTING_COUNT_MAX) {
            mEnrollCountPerDown = FingerSettingsConst.ENROLL_SETTING_COUNT_MAX;
        }

        SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                    FingerSettingsConst.ENROLL_SETTING_COUNT_NAME, mEnrollCountPerDown);

        if (mEnrollId > mEnrollCountPerDown) {
            mEnrollId = 0;
        }

        updateUI();
    }

    private void updateId(int offset) {
        mEnrollId += offset;
        if (mEnrollId < 0 || mEnrollCountPerDown == 0) {
            mEnrollId = 0;
        } else if (mEnrollId + 1 > mEnrollCountPerDown) {
            mEnrollId = mEnrollCountPerDown - 1;
        }

        updateUI();
    }

    private void updateImage(int offset) {
        int index = FingerSettingsConst.getValueIndex(mEnrollIdImageName, mFingerprintImageNameArray, 0);
        if ((offset < 0) && (index > 0)) { // pre img
            index--;
        } else if ((offset > 0) && (index < mFingerprintImageNameArray.length - 1)) { // next img
            index++;
        }

        mEnrollIdImageName = mFingerprintImageNameArray[index].toString();
        SharedPreferencesData.saveDataString(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                    FingerSettingsConst.ENROLL_SETTING_ID_IMAGE_NAME_PERFIX + mEnrollId, mEnrollIdImageName);

        updateUI();
    }

    private void updateDIY(boolean enabled) {
        mEnrollImageDIY = enabled;
        SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                    FingerSettingsConst.ENROLL_SETTING_IMG_DIY_NAME, mEnrollImageDIY);
        updateUI();
    }

    private void updateDelay(int offset) {
        mEnrollImageDelay += offset;
        if (mEnrollImageDelay < 0) {
            mEnrollImageDelay = 0;
        }

        SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                    FingerSettingsConst.ENROLL_SETTING_IMG_DELAY_NAME, mEnrollImageDelay);
        updateUI();
    }

    private void clearData() {
        SharedPreferencesData.clearData(this, FingerSettingsConst.ENROLL_SETTING_CONFIG);
        initUI();
    }

    private void initUI() {
        mEnrollId = 0;
        updateUI();
    }

    private void updateUI() {
        mEnrollCountPerDown = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                FingerSettingsConst.ENROLL_SETTING_COUNT_NAME, FingerSettingsConst.ENROLL_SETTING_COUNT_DEFAULT);
        mEnrollImageDIY = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                FingerSettingsConst.ENROLL_SETTING_IMG_DIY_NAME, FingerSettingsConst.ENROLL_SETTING_IMG_DIY_DEFAULT);
        mEnrollImageDelay = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                FingerSettingsConst.ENROLL_SETTING_IMG_DELAY_NAME, FingerSettingsConst.ENROLL_SETTING_IMG_DELAY_DEFAULT);
        mEnrollIdImageName = SharedPreferencesData.loadDataString(this, FingerSettingsConst.ENROLL_SETTING_CONFIG,
                FingerSettingsConst.ENROLL_SETTING_ID_IMAGE_NAME_PERFIX + mEnrollId);
        if (mEnrollIdImageName == null || mEnrollIdImageName.isEmpty()) {
            mEnrollIdImageName = SharedPreferencesData.loadDataString(this, FingerSettingsConst.OPTIC_SETTINGS_CONFIG,
                FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_NAME);
        }
        if (mEnrollIdImageName == null || mEnrollIdImageName.isEmpty()) {            
            mEnrollIdImageName = FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT;
        }

        mFingerEnrollCount.setText(Integer.toString(mEnrollCountPerDown));
        mFingerEnrollId.setText(Integer.toString(mEnrollId + 1));
        mFingerEnrollImageDIYCBox.setChecked(mEnrollImageDIY);
        mFingerEnrollDelay.setText(Integer.toString(mEnrollImageDelay * 10));

        int drawableId = FingerSettingsConst.getDrawableId(this, getPackageName(), mEnrollIdImageName, FingerSettingsConst.OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT);
        FingerImageSize imageSize = new FingerImageSize();
        FingerSettingsConst.getDrawableSize(this, drawableId, imageSize);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(imageSize.width, imageSize.height);
        mFingerEnrollImage.setLayoutParams(params);
        mFingerEnrollImage.setImageDrawable(this.getDrawable(drawableId));

        if ((mEnrollCountPerDown == FingerSettingsConst.ENROLL_SETTING_DISABLE) || (!mEnrollImageDIY)) {
            if (mEnrollCountPerDown != FingerSettingsConst.ENROLL_SETTING_DISABLE) {
                mFingerEnrollImageDIYLayout.setVisibility(View.VISIBLE);
            } else {
                mFingerEnrollImageDIYLayout.setVisibility(View.GONE);
            }
            mFingerEnrollImageIdLayout.setVisibility(View.GONE);
            mFingerEnrollImageIconLayout.setVisibility(View.GONE);
            mFingerEnrollImageDelayLayout.setVisibility(View.GONE);
        } else {
            mFingerEnrollImageDIYLayout.setVisibility(View.VISIBLE);
            mFingerEnrollImageIdLayout.setVisibility(View.VISIBLE);
            mFingerEnrollImageIconLayout.setVisibility(View.VISIBLE);
            mFingerEnrollImageDelayLayout.setVisibility(View.VISIBLE);
        }
    }
}
