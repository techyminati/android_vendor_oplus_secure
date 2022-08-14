
package com.silead.frrfar;

import java.io.File;
import java.util.HashSet;
import java.util.Set;

import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.MultiSelectListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;

public class FingerTestSettingActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener, View.OnClickListener {
    private Runnable mStatusUpdateRunnable;
    private boolean mHidenSpecSettings = true;

    private EditTextPreference mEnrollSampleCountEdtPref;
    private Preference mEnrollDataPathPref;
    private MultiSelectListPreference mEnrollFingersMulSelectPref;
    private CheckBoxPreference mTestFrrFarImgResultCBoxPref;
    private CheckBoxPreference mTestFarRunFrrFirstCBoxPref;
    private CheckBoxPreference mTestFrrFarImgOrigCBoxPref;
    private CheckBoxPreference mOtherIsOpticCBoxPref;
    private Preference mOtherFingerImagePref;
    private ListPreference mOtherGatherFingerListPref;
    private CheckBoxPreference mOtherImgDisplayCBoxPref;
    private Preference mOtherEnrollSettingPref;
    private Preference mOtherSearchSensorPref;
    private CheckBoxPreference mFactoryTestImgDumpCBoxPref;
    private CheckBoxPreference mFactoryTestClearPwdAutoCBoxPref;

    private Button mBackBtn;

    private static final int MSG_UPDATE_SAMPLE_COUNT_SUMMARY = 1;
    private static final int MSG_UPDATE_SAMPLE_COUNT_VALUE = 2;
    private static final int MSG_UPDATE_ENROLL_FINGER_SUMMARY = 3;
    private static final int MSG_UPDATE_ENROLL_FINGER_VALUE = 4;
    private static final int MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE = 5;
    private static final int MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE = 6;
    private static final int MSG_UPDATE_TEST_FRRFAR_IMG_ORIG_VALUE = 7;
    private static final int MSG_UPDATE_TEST_IS_OPTIC_VALUE = 8;
    private static final int MSG_UPDATE_TEST_GATHER_FINGER_SUMMARY = 9;
    private static final int MSG_UPDATE_TEST_GATHER_FINGER_VALUE = 10;
    private static final int MSG_UPDATE_OTHER_IMG_DISPLAY_VALUE = 11;
    private static final int MSG_UPDATE_FACTORY_TEST_IMG_DUMP_VALUE = 12;
    private static final int MSG_UPDATE_FACTORY_CLEAR_PWD_AUTO_VALUE = 13;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.finger_settings);
        if (FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
            setContentView(R.layout.ui_include_preference_main);
        }

        mEnrollSampleCountEdtPref = (EditTextPreference) findPreference("finger_settings_enroll_sample_count");
        mEnrollDataPathPref = (Preference) findPreference("finger_settings_enroll_data_path");
        mEnrollFingersMulSelectPref = (MultiSelectListPreference) findPreference("finger_settings_enroll_finger");
        mTestFrrFarImgResultCBoxPref = (CheckBoxPreference) findPreference("finger_settings_test_farfrr_img_result");
        mTestFarRunFrrFirstCBoxPref = (CheckBoxPreference) findPreference("finger_settings_test_far_run_frr_first");
        mTestFrrFarImgOrigCBoxPref = (CheckBoxPreference) findPreference("finger_settings_test_farfrr_img_orig");
        mOtherIsOpticCBoxPref = (CheckBoxPreference) findPreference("finger_settings_other_is_optic");
        mOtherFingerImagePref = (PreferenceScreen) findPreference("finger_settings_other_finger_image");
        mOtherGatherFingerListPref = (ListPreference) findPreference("finger_settings_other_gather_finger");
        mOtherImgDisplayCBoxPref = (CheckBoxPreference) findPreference("finger_settings_other_img_display");
        mOtherEnrollSettingPref = (PreferenceScreen) findPreference("finger_settings_other_img_count_each_down");
        mOtherSearchSensorPref = (PreferenceScreen) findPreference("finger_settings_other_search_sensor");
        mFactoryTestImgDumpCBoxPref = (CheckBoxPreference) findPreference("finger_settings_factory_test_dump_image");
        mFactoryTestClearPwdAutoCBoxPref = (CheckBoxPreference) findPreference("finger_settings_factory_test_clear_pwd_auto");

        mBackBtn = (Button) findViewById(R.id.ui_bottom_btn);

        mHidenSpecSettings = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_OTHER_SEPCIAL_SETTINGS_HIDEN_NAME, FingerSettingsConst.FP_SETTINGS_OTHER_SEPCIAL_SETTINGS_HIDEN_DEFAULT);
        if (mHidenSpecSettings) {
            PreferenceCategory pCategory = (PreferenceCategory)findPreference("finger_settings_other_category");
            if (mOtherFingerImagePref != null) {
                pCategory.removePreference(mOtherFingerImagePref);
                mOtherFingerImagePref = null;
            }
            if (mOtherGatherFingerListPref != null) {
                pCategory.removePreference(mOtherGatherFingerListPref);
                mOtherGatherFingerListPref = null;
            }
            if (mOtherImgDisplayCBoxPref != null) {
                pCategory.removePreference(mOtherImgDisplayCBoxPref);
                mOtherImgDisplayCBoxPref = null;
            }
            if (mOtherEnrollSettingPref != null) {
                pCategory.removePreference(mOtherEnrollSettingPref);
                mOtherEnrollSettingPref = null;
            }
            if (mOtherSearchSensorPref != null) {
                pCategory.removePreference(mOtherSearchSensorPref);
                mOtherSearchSensorPref = null;
            }
            pCategory = (PreferenceCategory)findPreference("finger_settings_factory_category");
            if (mFactoryTestImgDumpCBoxPref != null) {
                pCategory.removePreference(mFactoryTestImgDumpCBoxPref);
                mFactoryTestImgDumpCBoxPref = null;
            }
            if (mFactoryTestClearPwdAutoCBoxPref != null) {
                pCategory.removePreference(mFactoryTestClearPwdAutoCBoxPref);
                mFactoryTestClearPwdAutoCBoxPref = null;
            }
        }

        mStatusUpdateRunnable = new Runnable() {
            public void run() {
                if (mEnrollSampleCountEdtPref != null) {
                    updatePreferenceStatus(mEnrollSampleCountEdtPref, MSG_UPDATE_SAMPLE_COUNT_SUMMARY);
                    updatePreferenceStatus(mEnrollSampleCountEdtPref, MSG_UPDATE_SAMPLE_COUNT_VALUE);
                }
                if (mEnrollFingersMulSelectPref != null) {
                    updatePreferenceStatus(mEnrollFingersMulSelectPref, MSG_UPDATE_ENROLL_FINGER_SUMMARY);
                    updatePreferenceStatus(mEnrollFingersMulSelectPref, MSG_UPDATE_ENROLL_FINGER_VALUE);
                }
                if (mTestFrrFarImgResultCBoxPref != null) {
                    updatePreferenceStatus(mTestFrrFarImgResultCBoxPref, MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE);
                }
                if (mTestFarRunFrrFirstCBoxPref != null) {
                    updatePreferenceStatus(mTestFarRunFrrFirstCBoxPref, MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE);
                }
                if (mTestFrrFarImgOrigCBoxPref != null) {
                    updatePreferenceStatus(mTestFrrFarImgOrigCBoxPref, MSG_UPDATE_TEST_FRRFAR_IMG_ORIG_VALUE);
                }
                if (mOtherIsOpticCBoxPref != null) {
                    updatePreferenceStatus(mOtherIsOpticCBoxPref, MSG_UPDATE_TEST_IS_OPTIC_VALUE);
                }
                if (mOtherGatherFingerListPref != null) {
                    updatePreferenceStatus(mOtherGatherFingerListPref, MSG_UPDATE_TEST_GATHER_FINGER_SUMMARY);
                    updatePreferenceStatus(mOtherGatherFingerListPref, MSG_UPDATE_TEST_GATHER_FINGER_VALUE);
                }
                if (mOtherImgDisplayCBoxPref != null) {
                    updatePreferenceStatus(mOtherImgDisplayCBoxPref, MSG_UPDATE_OTHER_IMG_DISPLAY_VALUE);
                }
                if (mFactoryTestImgDumpCBoxPref != null) {
                    updatePreferenceStatus(mFactoryTestImgDumpCBoxPref, MSG_UPDATE_FACTORY_TEST_IMG_DUMP_VALUE);
                }
                if (mFactoryTestClearPwdAutoCBoxPref != null) {
                    updatePreferenceStatus(mFactoryTestClearPwdAutoCBoxPref, MSG_UPDATE_FACTORY_CLEAR_PWD_AUTO_VALUE);
                }
            }
        };

        if (mEnrollDataPathPref != null) {
            File dataDir = FingerSettingsConst.getDataSavePath(this, false);
            mEnrollDataPathPref.setSummary(dataDir.getAbsolutePath());
        }
        if (mEnrollSampleCountEdtPref != null) {
            mEnrollSampleCountEdtPref.setOnPreferenceChangeListener(this);
        }
        if (mEnrollFingersMulSelectPref != null) {
            mEnrollFingersMulSelectPref.setOnPreferenceChangeListener(this);
        }
        if (mTestFrrFarImgResultCBoxPref != null) {
            mTestFrrFarImgResultCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mTestFarRunFrrFirstCBoxPref != null) {
            mTestFarRunFrrFirstCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mTestFrrFarImgOrigCBoxPref != null) {
            mTestFrrFarImgOrigCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mOtherIsOpticCBoxPref != null) {
            mOtherIsOpticCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mOtherGatherFingerListPref != null) {
            mOtherGatherFingerListPref.setOnPreferenceChangeListener(this);
        }
        if (mOtherImgDisplayCBoxPref != null) {
            mOtherImgDisplayCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mFactoryTestImgDumpCBoxPref != null) {
            mFactoryTestImgDumpCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mFactoryTestClearPwdAutoCBoxPref != null) {
            mFactoryTestClearPwdAutoCBoxPref.setOnPreferenceChangeListener(this);
        }
        if (mBackBtn != null) {
            mBackBtn.setOnClickListener(this);
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
                case MSG_UPDATE_SAMPLE_COUNT_SUMMARY: {
                    if (mEnrollSampleCountEdtPref != null) {
                        mEnrollSampleCountEdtPref.setSummary((String) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_SAMPLE_COUNT_VALUE: {
                    if (mEnrollSampleCountEdtPref != null) {
                        mEnrollSampleCountEdtPref.setText((String) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_ENROLL_FINGER_SUMMARY: {
                    if (mEnrollFingersMulSelectPref != null) {
                        mEnrollFingersMulSelectPref.setSummary((CharSequence) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_ENROLL_FINGER_VALUE: {
                    if (mEnrollFingersMulSelectPref != null) {
                        mEnrollFingersMulSelectPref.setValues((Set<String>) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE: {
                    mTestFrrFarImgResultCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
                    break;
                }
                case MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE: {
                    mTestFarRunFrrFirstCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
                    break;
                }
                case MSG_UPDATE_TEST_FRRFAR_IMG_ORIG_VALUE: {
                    mTestFrrFarImgOrigCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
                    break;
                }
                case MSG_UPDATE_TEST_IS_OPTIC_VALUE: {
                    mOtherIsOpticCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
                    break;
                }
                case MSG_UPDATE_TEST_GATHER_FINGER_SUMMARY: {
                    if (mOtherGatherFingerListPref != null) {
                        mOtherGatherFingerListPref.setSummary((CharSequence) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_TEST_GATHER_FINGER_VALUE: {
                    if (mOtherGatherFingerListPref != null) {
                        mOtherGatherFingerListPref.setValue((String) msg.obj);
                    }
                    break;
                }
                case MSG_UPDATE_OTHER_IMG_DISPLAY_VALUE: {
                    if(mOtherImgDisplayCBoxPref != null) {
                        mOtherImgDisplayCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
                    }
                    break;
                }
                case MSG_UPDATE_FACTORY_TEST_IMG_DUMP_VALUE: {
                    if(mFactoryTestImgDumpCBoxPref != null) {
                        mFactoryTestImgDumpCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
                    }
                    break;
                }
                case MSG_UPDATE_FACTORY_CLEAR_PWD_AUTO_VALUE: {
                    if(mFactoryTestClearPwdAutoCBoxPref != null) {
                        mFactoryTestClearPwdAutoCBoxPref.setChecked(((Boolean)msg.obj).booleanValue());
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

        if (preference instanceof EditTextPreference) {
            String name = null;
            int value = FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_DEFAULT;
            if (msg == MSG_UPDATE_SAMPLE_COUNT_SUMMARY || msg == MSG_UPDATE_SAMPLE_COUNT_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_NAME;
                value = FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_DEFAULT;
            }
            if (name != null && !name.isEmpty()) {
                value = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG, name, value);
            }
            mHandler.sendMessage(mHandler.obtainMessage(msg, String.valueOf(value)));
        } else if (preference instanceof MultiSelectListPreference) {
            String name = null;
            int value = FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT;
            if (msg == MSG_UPDATE_ENROLL_FINGER_SUMMARY || msg == MSG_UPDATE_ENROLL_FINGER_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME;
                value = FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_DEFAULT;
            }
            if (name != null && !name.isEmpty()) {
                value = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG, name, value);
            }
            if (msg == MSG_UPDATE_ENROLL_FINGER_SUMMARY) {
                CharSequence[] summarys = ((MultiSelectListPreference) preference).getEntries();
                String summary = "";
                if (summarys != null) {
                    for (int i = 0; i < summarys.length; i++) {
                        if ((value & (1 << i)) != 0) {
                            summary += summarys[i] + " ";
                        }
                    }
                }
                mHandler.sendMessage(mHandler.obtainMessage(msg, summary));
            } else {
                CharSequence[] EntryValues = ((MultiSelectListPreference) preference).getEntryValues();
                Set<String> values = new HashSet<String>();
                if (EntryValues != null) {
                    for (int i = 0; i < EntryValues.length; i++) {
                        if ((value & (1 << i)) != 0) {
                            values.add(EntryValues[i].toString());
                        }
                    }
                }
                mHandler.sendMessage(mHandler.obtainMessage(msg, values));
            }
        } else if (preference instanceof CheckBoxPreference) {
            String name = null;
            boolean value = FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMG_RESULT_DEFAULT;
            if (msg == MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMG_RESULT_NAME;
                value = FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMG_RESULT_DEFAULT;
            } else if (msg == MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_TEST_FAR_RUN_FRR_FIRST_NAME;
                value = FingerSettingsConst.FP_SETTINGS_TEST_FAR_RUN_FRR_FIRST_DEFAULT;
            } else if (msg == MSG_UPDATE_TEST_FRRFAR_IMG_ORIG_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMAGE_ORIG_NAME;
                value = FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMAGE_ORIG_DEFAULT;
            } else if (msg == MSG_UPDATE_TEST_IS_OPTIC_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_OTHER_IS_OPTIC_NAME;
                value = FingerSettingsConst.FP_SETTINGS_OTHER_IS_OPTIC_DEFAULT;
            } else if (msg == MSG_UPDATE_OTHER_IMG_DISPLAY_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_OTHER_IMG_DISPLAY_NAME;
                value = FingerSettingsConst.FP_SETTINGS_OTHER_IMG_DISPLAY_DEFAULT;
            } else if (msg == MSG_UPDATE_FACTORY_TEST_IMG_DUMP_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_FACTORY_FINGER_DUMP_IMAGE_NAME;
                value = FingerSettingsConst.FP_SETTINGS_FACTORY_FINGER_DUMP_IMAGE_DEFAULT;
            } else if (msg == MSG_UPDATE_FACTORY_CLEAR_PWD_AUTO_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_FACTORY_CLEAR_PWD_AUTO_NAME;
                value = FingerSettingsConst.FP_SETTINGS_FACTORY_CLEAR_PWD_AUTO_DEFAULT;
            }
            if (name != null && !name.isEmpty()) {
                value = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG, name, value);
            }
            if (msg == MSG_UPDATE_TEST_IS_OPTIC_VALUE) {
                if (mOtherFingerImagePref != null) {
                    mOtherFingerImagePref.setEnabled(value);
                }
                if (mOtherEnrollSettingPref != null) {
                    mOtherEnrollSettingPref.setEnabled(value);
                }
                if (mOtherSearchSensorPref != null) {
                    mOtherSearchSensorPref.setEnabled(value);
                }
            }
            mHandler.sendMessage(mHandler.obtainMessage(msg, value));
        } else if (preference instanceof ListPreference) {
            String name = null;
            int index = 0;
            if (msg == MSG_UPDATE_TEST_GATHER_FINGER_SUMMARY || msg == MSG_UPDATE_TEST_GATHER_FINGER_VALUE) {
                name = FingerSettingsConst.FP_SETTINGS_OTHER_GATHER_FINGER_NAME;
                index = FingerSettingsConst.FP_SETTINGS_OTHER_GATHER_FINGER_DEFAULT;
            }
            if (name != null && !name.isEmpty()) {
                index = SharedPreferencesData.loadDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG, name, index);
            }

            if (msg == MSG_UPDATE_TEST_GATHER_FINGER_SUMMARY) {
                CharSequence[] entries = ((ListPreference)preference).getEntries();
                if (index >= entries.length) {
                    index = 0;
                }
                mHandler.sendMessage(mHandler.obtainMessage(msg, entries[index]));
            } else if (msg == MSG_UPDATE_TEST_GATHER_FINGER_VALUE) {
                CharSequence[] values = ((ListPreference) preference).getEntryValues();
                if (index >= values.length) {
                    index = 0;
                }
                mHandler.sendMessage(mHandler.obtainMessage(msg, values[index]));
            }
        }
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference == mEnrollSampleCountEdtPref) {
            String value = (String) newValue;
            if (value != null && !value.isEmpty() && Integer.valueOf(value) > 0) {
                SharedPreferencesData.saveDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                        FingerSettingsConst.FP_SETTINGS_SAMPLE_COUNT_NAME, Integer.valueOf(value));
            }
            updatePreferenceStatus(mEnrollSampleCountEdtPref, MSG_UPDATE_SAMPLE_COUNT_SUMMARY);
            updatePreferenceStatus(mEnrollSampleCountEdtPref, MSG_UPDATE_SAMPLE_COUNT_VALUE);
            return true;
        } else if (preference == mEnrollFingersMulSelectPref) {
            int value = 0;
            CharSequence[] EntryValues = ((MultiSelectListPreference) preference).getEntryValues();
            Set<String> values = (Set<String>) newValue;
            if (values.size() > 0) {
                for (int i = 0; i < EntryValues.length; i++) {
                    if (values.contains(EntryValues[i].toString())) {
                        value |= (1 << i);
                    }
                }
            }
            if (value != 0) {
                SharedPreferencesData.saveDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                        FingerSettingsConst.FP_SETTINGS_ENROLL_FINGER_NAME, value);
                SharedPreferencesData.saveDataInt(this, FingerSettingsConst.ENROLL_ENV_CONFIG,
                        FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_NAME, FingerSettingsConst.ENROLL_ENV_NEXT_FINGER_DEFAULT);
            }

            updatePreferenceStatus(mEnrollFingersMulSelectPref, MSG_UPDATE_ENROLL_FINGER_SUMMARY);
            updatePreferenceStatus(mEnrollFingersMulSelectPref, MSG_UPDATE_ENROLL_FINGER_VALUE);

            return true;
        } else if (preference == mTestFrrFarImgResultCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMG_RESULT_NAME, value);
            updatePreferenceStatus(mTestFrrFarImgResultCBoxPref, MSG_UPDATE_TEST_FARRFAR_IMG_RESULT_VALUE);
        } else if (preference == mTestFarRunFrrFirstCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_TEST_FAR_RUN_FRR_FIRST_NAME, value);
            updatePreferenceStatus(mTestFarRunFrrFirstCBoxPref, MSG_UPDATE_TEST_FAR_RUN_FRR_FIRST_VALUE);
        } else if (preference == mTestFrrFarImgOrigCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMAGE_ORIG_NAME, value);
            updatePreferenceStatus(mTestFrrFarImgOrigCBoxPref, MSG_UPDATE_TEST_FRRFAR_IMG_ORIG_VALUE);
        } else if (preference == mOtherIsOpticCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_OTHER_IS_OPTIC_NAME, value);
            updatePreferenceStatus(mOtherIsOpticCBoxPref, MSG_UPDATE_TEST_IS_OPTIC_VALUE);
        } else if (preference == mOtherGatherFingerListPref) {
            String value = (String)newValue;
            CharSequence[] values = mOtherGatherFingerListPref.getEntryValues();
            SharedPreferencesData.saveDataInt(this, FingerSettingsConst.FP_SETTINGS_CONFIG, FingerSettingsConst.FP_SETTINGS_OTHER_GATHER_FINGER_NAME,
                    FingerSettingsConst.getValueIndex(value, values, FingerSettingsConst.FP_SETTINGS_OTHER_GATHER_FINGER_DEFAULT));
            updatePreferenceStatus(mOtherGatherFingerListPref, MSG_UPDATE_TEST_GATHER_FINGER_SUMMARY);
            updatePreferenceStatus(mOtherGatherFingerListPref, MSG_UPDATE_TEST_GATHER_FINGER_VALUE);
        } else if (preference == mOtherImgDisplayCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_OTHER_IMG_DISPLAY_NAME, value);
            updatePreferenceStatus(mOtherImgDisplayCBoxPref, MSG_UPDATE_OTHER_IMG_DISPLAY_VALUE);
        } else if (preference == mFactoryTestImgDumpCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_FACTORY_FINGER_DUMP_IMAGE_NAME, value);
            updatePreferenceStatus(mFactoryTestImgDumpCBoxPref, MSG_UPDATE_FACTORY_TEST_IMG_DUMP_VALUE);
        } else if (preference == mFactoryTestClearPwdAutoCBoxPref) {
            boolean value = ((Boolean)newValue).booleanValue();
            SharedPreferencesData.saveDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                    FingerSettingsConst.FP_SETTINGS_FACTORY_CLEAR_PWD_AUTO_NAME, value);
            updatePreferenceStatus(mFactoryTestClearPwdAutoCBoxPref, MSG_UPDATE_FACTORY_CLEAR_PWD_AUTO_VALUE);
        }
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
