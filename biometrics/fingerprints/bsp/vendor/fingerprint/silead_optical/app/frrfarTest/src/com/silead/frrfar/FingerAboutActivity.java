
package com.silead.frrfar;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.os.Bundle;
import android.os.SystemClock;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerVersion;

public class FingerAboutActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener, View.OnClickListener {
    private FingerManager mFingerManager;

    private Preference mAboutAppVerPref;
    private Preference mAboutHalVerPref;
    private Preference mAboutDevVerPref;
    private Preference mAboutAlgoVerPref;
    private Preference mAboutTaVerPref;

    private Button mBackBtn;

    private long[] mHits = new long[5];
    private Toast mDevHitToast;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.finger_about);
        if (FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
            setContentView(R.layout.ui_include_preference_main);
        }

        mAboutAppVerPref = (Preference) findPreference("about_info_test");
        mAboutHalVerPref = (Preference) findPreference("about_info_hal");
        mAboutDevVerPref = (Preference) findPreference("about_info_dev");
        mAboutAlgoVerPref = (Preference) findPreference("about_info_algo");
        mAboutTaVerPref = (Preference) findPreference("about_info_ta");

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);

        if (mAboutAppVerPref != null) {
            String strAPPVer = getAPPVersion(this);
            if (strAPPVer != null) {
                mAboutAppVerPref.setSummary(strAPPVer);
            }
        }

        mFingerManager = FingerManager.getDefault(this);
        mFingerManager.getAllVerInfo(mTestCmdCallback);

        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
        }
    }

    private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
        @Override
        public void onTestResult(int cmdId, Object result) {
            if (cmdId == FingerManager.TEST_CMD_GET_VERSION && result instanceof FingerVersion) {
                FingerVersion version = (FingerVersion)result;
                if (mAboutHalVerPref != null) {
                    mAboutHalVerPref.setSummary(version.getHalVersion());
                }
                if (mAboutDevVerPref != null) {
                    mAboutDevVerPref.setSummary(version.getDevVersion());
                }
                if (mAboutAlgoVerPref != null) {
                    mAboutAlgoVerPref.setSummary(version.getAlgoVersion());
                }
                if (mAboutTaVerPref != null) {
                    mAboutTaVerPref.setSummary(version.getTaVersion());
                }
            }
        }
    };

    private String getAPPVersion(Context ctx) {
        String versionName = null;
        PackageManager manager = ctx.getPackageManager();
        try {
            PackageInfo info = manager.getPackageInfo(ctx.getPackageName(), 0);
            versionName = info.versionName;
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }
        return versionName;
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        return true;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference == mAboutTaVerPref) {
            System.arraycopy(mHits, 1, mHits, 0, mHits.length - 1);
            mHits[mHits.length-1] = SystemClock.uptimeMillis();
            if (mHits[0] >= (SystemClock.uptimeMillis() - 1000)) {
                for (int j = 0; j < mHits.length-1; j++) {
                    mHits[j]=0;
                }
                boolean specialHidden = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_OTHER_SEPCIAL_SETTINGS_HIDEN_NAME, FingerSettingsConst.FP_SETTINGS_OTHER_SEPCIAL_SETTINGS_HIDEN_DEFAULT);
                specialHidden = !specialHidden;

                SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                            FingerSettingsConst.FP_SETTINGS_OTHER_SEPCIAL_SETTINGS_HIDEN_NAME, specialHidden);

                specialHidden = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_OTHER_SEPCIAL_SETTINGS_HIDEN_NAME, FingerSettingsConst.FP_SETTINGS_OTHER_SEPCIAL_SETTINGS_HIDEN_DEFAULT);
                if (mDevHitToast != null) {
                    mDevHitToast.cancel();
                }
                if (specialHidden) {
                    mDevHitToast = Toast.makeText(this, R.string.finger_settings_other_special_hidden_enabled, Toast.LENGTH_SHORT);
                } else {
                    mDevHitToast = Toast.makeText(this, R.string.finger_settings_other_special_hidden_disabled, Toast.LENGTH_SHORT);
                }
                mDevHitToast.show();
            }
        }
        return false;
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        }
    }
}
