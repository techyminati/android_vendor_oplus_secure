
package com.silead.frrfar;

import android.content.Intent;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;

public class EnrollEnvSettingActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener, View.OnClickListener {
    private Runnable mStatusUpdateRunnable;

    private Button mEnrollFinishBtn;
    private Button mEnrollStartBtn;
    private EditTextPreference mEnrollEnvSetUserIdEdtPref;
    private ListPreference mEnrollEnvSetHandListPref;
    private ListPreference mEnrollEnvSetFingerListPref;

    private static final int REQUEST_START_ENROLL = 100;
    public static final int REPONSE_START_ENROLL_SUCCESS = 1000;

    private static final int MSG_UPDATE_USERID_SUMMARY = 1;
    private static final int MSG_UPDATE_USERID_VALUE = 2;
    private static final int MSG_UPDATE_HAND_SUMMARY = 3;
    private static final int MSG_UPDATE_HAND_VALUE = 4;
    private static final int MSG_UPDATE_FINGER_SUMMARY = 5;
    private static final int MSG_UPDATE_FINGER_VALUE = 6;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.enroll_env_settings);
        setContentView(R.layout.ui_include_preference_main);

        mEnrollFinishBtn = (Button) findViewById(R.id.ui_bottom_btn);
        mEnrollStartBtn = (Button) findViewById(R.id.ui_bottom_btn_2);
        mEnrollEnvSetUserIdEdtPref = (EditTextPreference) findPreference("enroll_env_set_userid");
        mEnrollEnvSetHandListPref = (ListPreference) findPreference("enroll_env_set_hand");
        mEnrollEnvSetFingerListPref = (ListPreference) findPreference("enroll_env_set_finger");

        mStatusUpdateRunnable = new Runnable() {
            public void run() {
                if (mEnrollEnvSetHandListPref != null) {
                    updatePreferenceStatus(mEnrollEnvSetHandListPref, MSG_UPDATE_HAND_SUMMARY);
                    updatePreferenceStatus(mEnrollEnvSetHandListPref, MSG_UPDATE_HAND_VALUE);
                }
                if (mEnrollEnvSetFingerListPref != null) {
                    updatePreferenceStatus(mEnrollEnvSetFingerListPref, MSG_UPDATE_FINGER_SUMMARY);
                    updatePreferenceStatus(mEnrollEnvSetFingerListPref, MSG_UPDATE_FINGER_VALUE);
                }
                if (mEnrollEnvSetUserIdEdtPref != null) {
                    updatePreferenceStatus(mEnrollEnvSetUserIdEdtPref, MSG_UPDATE_USERID_SUMMARY);
                    updatePreferenceStatus(mEnrollEnvSetUserIdEdtPref, MSG_UPDATE_USERID_VALUE);
                }
            }
        };

        if (mEnrollFinishBtn != null) {
            if (!FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
                mEnrollFinishBtn.setVisibility(View.GONE);
            }
            mEnrollFinishBtn.setOnClickListener(this);
        }
        if (mEnrollStartBtn != null) {
            mEnrollStartBtn.setOnClickListener(this);
            mEnrollStartBtn.setText(R.string.enroll_env_set_start_enroll);
            mEnrollStartBtn.setVisibility(View.VISIBLE);
        }
        if (mEnrollEnvSetHandListPref != null) {
            mEnrollEnvSetHandListPref.setOnPreferenceChangeListener(this);
        }
        if (mEnrollEnvSetFingerListPref != null) {
            int value = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME, FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT);
            String[] finger_name = this.getResources().getStringArray(R.array.finger_settings_enroll_finger_string);
            String[] finger_value = this.getResources().getStringArray(R.array.finger_settings_enroll_finger_values);
            if (value == 0) {
                value = FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT;
            }

            int count = FingerSettingsConst.numberInBinary(value);
            CharSequence[] entries = new CharSequence[count];
            CharSequence[] entryValues = new CharSequence[count];
            int index = 0;
            for (int i = 0; i < finger_name.length & index < count; i++) {
                if ((value & (1 << i)) != 0) {
                    entries[index] = finger_name[i];
                    entryValues[index++] = finger_value[i];
                }
            }
            mEnrollEnvSetFingerListPref.setEntries(entries);
            mEnrollEnvSetFingerListPref.setEntryValues(entryValues);

            mEnrollEnvSetFingerListPref.setOnPreferenceChangeListener(this);
        }
        if (mEnrollEnvSetUserIdEdtPref != null) {
            mEnrollEnvSetUserIdEdtPref.setOnPreferenceChangeListener(this);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updateStatus();
    }

    private void updateStatus() {
        new Thread(mStatusUpdateRunnable).start();
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_USERID_SUMMARY: {
                    if (mEnrollEnvSetUserIdEdtPref != null) {
                        mEnrollEnvSetUserIdEdtPref.setSummary((String) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_USERID_VALUE: {
                    if (mEnrollEnvSetUserIdEdtPref != null) {
                        mEnrollEnvSetUserIdEdtPref.setText((String) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_HAND_SUMMARY: {
                    if (mEnrollEnvSetHandListPref != null) {
                        mEnrollEnvSetHandListPref.setTitle((CharSequence) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_HAND_VALUE: {
                    if (mEnrollEnvSetHandListPref != null) {
                        mEnrollEnvSetHandListPref.setValueIndex((Integer) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_FINGER_SUMMARY: {
                    if (mEnrollEnvSetFingerListPref != null) {
                        mEnrollEnvSetFingerListPref.setTitle((CharSequence) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_FINGER_VALUE: {
                    if (mEnrollEnvSetFingerListPref != null) {
                        mEnrollEnvSetFingerListPref.setValueIndex((Integer) msg.obj);
                    }
                    break;
                }
            }
        }
    };

    private void updatePreferenceStatus(Preference preference, int msg) {
        if (preference == null) {
            return;
        }

        if (preference instanceof ListPreference) {
            int index = 0;
            String name = null;

            if (msg == MSG_UPDATE_HAND_SUMMARY || msg == MSG_UPDATE_HAND_VALUE) {
                name = FingerSettingsConst.ENROLL_ENV_NEXT_HAND_NAME;
                index = FingerSettingsConst.ENROLL_ENV_NEXT_HAND_DEFAULT;
            } else if (msg == MSG_UPDATE_FINGER_SUMMARY || msg == MSG_UPDATE_FINGER_VALUE) {
                name = FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_NAME;
                index = FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_DEFAULT;
            }
            if (name != null && !name.isEmpty()) {
                index = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.ENROLL_ENV_CONFIG, name, index);
            }

            if (msg == MSG_UPDATE_HAND_SUMMARY || msg == MSG_UPDATE_FINGER_SUMMARY) {
                CharSequence[] summarys = ((ListPreference) preference).getEntries();
                if (index >= summarys.length) {
                    index = 0;
                }
                mHandler.sendMessage(mHandler.obtainMessage(msg, summarys[index]));
            } else {
                CharSequence[] values = ((ListPreference) preference).getEntryValues();
                if (index >= values.length) {
                    index = 0;
                }
                mHandler.sendMessage(mHandler.obtainMessage(msg, index));
            }
        } else if (preference instanceof EditTextPreference) {
            String name = null;
            String value = FingerSettingsConst.ENROLL_ENV_NEXT_USER_DEFAULT;
            if (msg == MSG_UPDATE_USERID_SUMMARY || msg == MSG_UPDATE_USERID_VALUE) {
                name = FingerSettingsConst.ENROLL_ENV_NEXT_USERID_NAME;
            }
            if (name != null && !name.isEmpty()) {
                String svalue = SharedPreferencesData.loadDataString(this, FingerSettingsConst.ENROLL_ENV_CONFIG, name);
                if (svalue != null && !svalue.isEmpty()) {
                    value = svalue;
                }
            }
            mHandler.sendMessage(mHandler.obtainMessage(msg, value));
        }
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference == mEnrollEnvSetHandListPref) {
            String value = (String) newValue;
            CharSequence[] values = mEnrollEnvSetHandListPref.getEntryValues();
            SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_ENV_CONFIG, FingerSettingsConst.ENROLL_ENV_NEXT_HAND_NAME,
                    FingerSettingsConst.getValueIndex(value, values, FingerSettingsConst.ENROLL_ENV_NEXT_HAND_DEFAULT));
            updatePreferenceStatus(mEnrollEnvSetHandListPref, MSG_UPDATE_HAND_SUMMARY);
            updatePreferenceStatus(mEnrollEnvSetHandListPref, MSG_UPDATE_HAND_VALUE);
            return true;
        } else if (preference == mEnrollEnvSetFingerListPref) {
            String value = (String) newValue;
            CharSequence[] values = mEnrollEnvSetFingerListPref.getEntryValues();
            SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_ENV_CONFIG, FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_NAME,
                    FingerSettingsConst.getValueIndex(value, values, FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_DEFAULT));
            updatePreferenceStatus(mEnrollEnvSetFingerListPref, MSG_UPDATE_FINGER_SUMMARY);
            updatePreferenceStatus(mEnrollEnvSetFingerListPref, MSG_UPDATE_FINGER_VALUE);
            return true;
        } else if (preference == mEnrollEnvSetUserIdEdtPref) {
            String value = (String) newValue;
            if (value != null && !value.isEmpty()) {
                SharedPreferencesData.saveDataString(this, FingerSettingsConst.ENROLL_ENV_CONFIG,
                        FingerSettingsConst.ENROLL_ENV_NEXT_USERID_NAME, value);
            }
            updatePreferenceStatus(mEnrollEnvSetUserIdEdtPref, MSG_UPDATE_USERID_SUMMARY);
            updatePreferenceStatus(mEnrollEnvSetUserIdEdtPref, MSG_UPDATE_USERID_VALUE);
        }
        return true;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        return false;
    }

    @Override
    public void onClick(View v) {
        if (v == mEnrollStartBtn) {
            String value = null;
            String user = FingerSettingsConst.ENROLL_ENV_NEXT_USER_DEFAULT;
            int hand = FingerSettingsConst.ENROLL_ENV_NEXT_HAND_DEFAULT;
            int finger = FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_DEFAULT;
            if (mEnrollEnvSetUserIdEdtPref != null) {
                user = mEnrollEnvSetUserIdEdtPref.getText();
                if (user == null || user.isEmpty()) {
                    user = FingerSettingsConst.ENROLL_ENV_NEXT_USER_DEFAULT;
                }
            }
            if (mEnrollEnvSetHandListPref != null) {
                value = mEnrollEnvSetHandListPref.getValue();
                CharSequence[] values = mEnrollEnvSetHandListPref.getEntryValues();
                hand = FingerSettingsConst.getValueIndex(value, values, hand);
            }
            if (mEnrollEnvSetFingerListPref != null) {
                value = mEnrollEnvSetFingerListPref.getValue();
                CharSequence[] values = mEnrollEnvSetFingerListPref.getEntryValues();
                finger = FingerSettingsConst.getValueIndex(value, values, finger);
            }

            if (FingerSettingsConst.LOG_DBG) {
                Log.d(FingerSettingsConst.LOG_TAG, "enroll: user=" + user + ", hand=" + hand + ", finger=" + finger);
            }

            Intent intent = new Intent();
            intent.setClass(this, EnrollingActivity.class);
            Bundle bundle = new Bundle();
            bundle.putString(EnrollingActivity.EXTRA_KEY_USER_TOKEN, user);
            bundle.putInt(EnrollingActivity.EXTRA_KEY_HAND_TOKEN, hand);
            bundle.putInt(EnrollingActivity.EXTRA_KEY_FINGER_TOKEN, finger);
            intent.putExtras(bundle);
            startActivityForResult(intent, REQUEST_START_ENROLL);
        } else if (v == mEnrollFinishBtn) {
            finish();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case REQUEST_START_ENROLL: {
                this.finish();
                break;
            }
        }
    }
}
