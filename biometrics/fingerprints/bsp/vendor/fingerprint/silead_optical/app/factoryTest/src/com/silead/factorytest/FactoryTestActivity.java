
package com.silead.factorytest;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;

public class FactoryTestActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener, View.OnClickListener {
    private Button mBackBtn;

    private String mLaunchMode = "";
    private boolean mIsOptic = true;

    private final int REQUEST_CODE_BASE = 0;
    private final int REQUEST_CODE_SPI_TEST     = REQUEST_CODE_BASE + 0;
    private final int REQUEST_CODE_RESET_PIN    = REQUEST_CODE_BASE + 1;
    private final int REQUEST_CODE_DEADPIXEL    = REQUEST_CODE_BASE + 2;
    private final int REQUEST_CODE_FLASH_TEST   = REQUEST_CODE_BASE + 3;
    private final int REQUEST_CODE_OTP_TEST     = REQUEST_CODE_BASE + 4;
    private final int REQUEST_CODE_SELF_TEST    = REQUEST_CODE_BASE + 5;

    private int mTestCasesFailed = 0;

    // adb shell am start -n com.silead.factorytest/.FactoryTestActivity --es MODE "PCBA"
    private ArrayList<?> mAutoTestList = new ArrayList<>(Arrays.asList(
        SpiTestActivity.class,
        FlashTestActivity.class,
        OTPTestActivity.class,
        SelfTestActivity.class,
        ResetPinTestActivity.class,
        DeadPixelTestActivity.class
    ));

    private ArrayList<Integer> mAutoRequestCodeList = new ArrayList<>(Arrays.asList(
        REQUEST_CODE_SPI_TEST,
        REQUEST_CODE_FLASH_TEST,
        REQUEST_CODE_OTP_TEST,
        REQUEST_CODE_SELF_TEST,
        REQUEST_CODE_RESET_PIN,
        REQUEST_CODE_DEADPIXEL
    ));

    private Iterator<?> mTestIterator;
    private Iterator<Integer> mRequestCodeIterator;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.factory_test_main);

        Bundle bundle = this.getIntent().getExtras();
        if(null != bundle) {
            mLaunchMode = bundle.getString(FactoryTestConst.LAUNCH_MODE_KEY);
            Log.d(FactoryTestConst.LOG_TAG, "bundle: MODE " + mLaunchMode);
        } else {
            Log.d(FactoryTestConst.LOG_TAG, "bundle is null");
        }

        if (FactoryTestConst.FP_UI_BACK_BTN_SUPPORT) {
            setContentView(R.layout.ui_include_preference_main);
        }

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
        }

        Context sharecontext = this;
        try {
            sharecontext = createPackageContext(FactoryTestConst.FP_SETTINGS_PACKAGE_NAME, Context.CONTEXT_IGNORE_SECURITY);
        } catch (PackageManager.NameNotFoundException e) {
        }
        mIsOptic = SharedPreferencesData.loadDataBoolean(sharecontext, FactoryTestConst.FP_SETTINGS_CONFIG,
                FactoryTestConst.FP_SETTINGS_IS_OPTIC_NAME, FactoryTestConst.FP_UI_OPTIC_MODE_DEFAULT);

        PreferenceScreen preferenceScreen = getPreferenceScreen();
        if (!mIsOptic) {
            preferenceScreen.removePreference(findPreference("main_test_item_flash"));
            preferenceScreen.removePreference(findPreference("main_test_item_otp"));
            preferenceScreen.removePreference(findPreference("main_test_item_cal"));

            mAutoTestList.remove(FlashTestActivity.class);
            mAutoTestList.remove(OTPTestActivity.class);
            if (mAutoRequestCodeList.contains(REQUEST_CODE_FLASH_TEST)) {
                mAutoRequestCodeList.remove((Integer) REQUEST_CODE_FLASH_TEST);
            }
            if (mAutoRequestCodeList.contains(REQUEST_CODE_OTP_TEST)) {
                mAutoRequestCodeList.remove((Integer) REQUEST_CODE_OTP_TEST);
            }
        }

        Preference preference = findPreference("main_test_item_finger");
        if (preference != null) {
            Intent preferenceIntent = preference.getIntent();
            if (!checkIntent(preferenceIntent)) {
                preferenceScreen.removePreference(preference);
            }
        }

        mTestIterator = mAutoTestList.iterator();
        mRequestCodeIterator = mAutoRequestCodeList.iterator();
        if(FactoryTestConst.isPCBAMode(mLaunchMode)) {
            TestModePCBA();
        }
    }

    public boolean onPreferenceChange(Preference preference, Object objValue) {
        return true;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        return false;
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.d(FactoryTestConst.LOG_TAG, "requestCode:"+ requestCode + ", resultCode:" + resultCode);
        if (0 != resultCode){
            mTestCasesFailed |= (1 << requestCode);
        }

        TestModePCBA();
    }

    private void TestModePCBA() {
        if (mTestIterator.hasNext() && mRequestCodeIterator.hasNext()) {
            Log.d(FactoryTestConst.LOG_TAG, "-----------------");
            Intent intent = new Intent(this, (Class<?>) mTestIterator.next());
            intent.putExtra(FactoryTestConst.LAUNCH_MODE_KEY, FactoryTestConst.LAUNCH_MODE_PCBA);
            startActivityForResult(intent, mRequestCodeIterator.next());
        } else {
            Log.d(FactoryTestConst.LOG_TAG, "-----------------");
            Log.d(FactoryTestConst.LOG_TAG, "mTestCasesFailed:" + mTestCasesFailed);
            Intent intent = new Intent();
            intent.putExtra("ErrorCode", mTestCasesFailed);
            setResult(mTestCasesFailed, intent);
            finish();
        }
    }

    public boolean checkIntent(Intent intent) {
        if (intent == null) {
            return false;
        }

        if (getPackageManager().resolveActivity(intent, 0) == null) {
            return false;
        }
        return true;
    }
}
