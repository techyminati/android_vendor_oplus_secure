
package com.silead.factorytest;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

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

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.finger_about);
        if (FactoryTestConst.FP_UI_BACK_BTN_SUPPORT) {
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
        return false;
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            finish();
        }
    }
}
