
package com.silead.frrfar;

import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.view.View;
import android.widget.Button;
import android.os.Bundle;
import android.util.Log;

public class FingerTestActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener, View.OnClickListener {
    private Button mBackBtn;
    private Preference mEnrollingPref;

    private String CALIBRATE_PACKAGE_NAME = "com.silead.factorytest";
    private String CALIBRATE_ACTIVITY_NAME = "com.silead.factorytest.CalibrateActivity";

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.frrfar_test_main);
        if (FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
            setContentView(R.layout.ui_include_preference_main);
        }

        mEnrollingPref = (Preference) findPreference("main_finger_item_new");

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
        }
    }

    public boolean onPreferenceChange(Preference preference, Object objValue) {
        return true;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        /*if (mEnrollingPref == preference) {
            if (checkActivityExist(CALIBRATE_PACKAGE_NAME, CALIBRATE_ACTIVITY_NAME)) {
                showNormalDialog();
                return true;
            }
        }*/
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

    private boolean checkActivityExist(String packageName, String name) {
        boolean exist = false;

        Intent intent = new Intent();
        intent.setClassName(packageName, name);
        ResolveInfo resolveInfo = getPackageManager().resolveActivity(intent, PackageManager.MATCH_DEFAULT_ONLY);
        if(resolveInfo != null) {
            exist = true;
        }
        return exist;
    }

    private void showNormalDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(FingerTestActivity.this);
        builder.setTitle(getText(R.string.enrolling_calibrate_title));
        builder.setMessage(getString(R.string.enrolling_calibrate_desc));

        DialogInterface.OnClickListener onClickListener = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case DialogInterface.BUTTON_NEGATIVE: {
                        Intent intent = new Intent(FingerTestActivity.this, EnrollEnvSettingActivity.class);
                        startActivity(intent);
                        break;
                    }
                    case DialogInterface.BUTTON_POSITIVE: {
                        Intent intent = new Intent();
                        ComponentName cn = new ComponentName(CALIBRATE_PACKAGE_NAME, CALIBRATE_ACTIVITY_NAME);
                        intent.setComponent(cn);
                        startActivity(intent);
                        break;
                    }
                }
                dialog.dismiss();
            }
        };

        builder.setPositiveButton(R.string.enrolling_calibrate_yes, onClickListener);
        builder.setNegativeButton(R.string.enrolling_calibrate_no, onClickListener);
        builder.show();
    }
}
