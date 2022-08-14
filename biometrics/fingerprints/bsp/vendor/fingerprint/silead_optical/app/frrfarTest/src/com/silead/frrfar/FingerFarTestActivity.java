
package com.silead.frrfar;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.silead.manager.FingerManager;
import com.silead.manager.FingerFrrFarImageResult;
import com.silead.manager.FingerResult;

public class FingerFarTestActivity extends Activity implements View.OnClickListener {
    private TextView mTestStatusTextView;

    private TextView mTestCurrentImageInfoTextView;
    private TextView mTestResultFailItemsTextView;
    private TextView mTestResultFailTextView;
    private TextView mTestResultTargetTextView;
    private Button mTestFinishBtn;
    private Button mTestStartBtn;
    private Button mTestReportBtn;
    private Button mTestAgainBtn;
    private ProgressBar mTestProgressBar;
    private TextView mTestTplEnrollTimeTextView;
    private TextView mTestAuthSuccessTimeTextView;
    private TextView mTestAuthFailedTimeTextView;
    private TextView mTestTplUpdTimeTextView;

    private static final int STATUS_IDLE = 1;
    private static final int STATUS_TESTING = 2;
    private static final int STAUTS_TEST_FINISH = 3;
    private int mTestStatus = STATUS_IDLE;

    private int mTestCount = 0;
    private int mTestAuthFailCount = 0;
    private int mTestImageCountPerFinger = 0;
    private int mTestImageEnrollCountPerFinger = 0;
    private int mTestImageFileFailPerFinger = 0;
    private int mTestImageAuthSuccessPerFinger = 0;
    private String mFingerIndicate = null;

    private long mTestTplEnrollPerFingerTime = 0;
    private long mTestTplEnrollTime = 0;
    private int mTestTplCount = 0;
    private long mTestAuthSuccessTime = 0;
    private long mTestAuthFailedTime = 0;
    private int mTestAuthSuccessCount = 0;

    boolean mFarNeedRunFrrFirst = true;

    private TestAsyncTask mTestAsyncTask = null;
    private FingerManager mFingerManager;
    private AsyncCommand mCommandMg;
    private boolean mTestImgOrig = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.frrfar_test_main);

        mTestStatusTextView = (TextView) findViewById(R.id.frrfar_test_info);

        mTestCurrentImageInfoTextView = (TextView) findViewById(R.id.frrfar_test_current_image_info);
        mTestResultFailItemsTextView = (TextView) findViewById(R.id.frrfar_test_result_fail_times);
        mTestResultFailTextView = (TextView) findViewById(R.id.frrfar_test_result_fail);
        mTestResultTargetTextView = (TextView) findViewById(R.id.frrfar_test_result_target);
        mTestFinishBtn = (Button) findViewById(R.id.frrfar_test_finish_btn);
        mTestStartBtn = (Button) findViewById(R.id.frrfar_test_start_btn);
        mTestReportBtn = (Button) findViewById(R.id.frrfar_test_report_btn);
        mTestAgainBtn = (Button) findViewById(R.id.frrfar_test_again_btn);
        mTestProgressBar = (ProgressBar) findViewById(R.id.frrfar_test_progress_bar);

        mTestTplEnrollTimeTextView = (TextView) findViewById(R.id.frrfar_test_result_time_tpl_load);
        mTestAuthSuccessTimeTextView = (TextView) findViewById(R.id.frrfar_test_result_time_auth_success);
        mTestAuthFailedTimeTextView = (TextView) findViewById(R.id.frrfar_test_result_time_auth_failed);
        mTestTplUpdTimeTextView = (TextView) findViewById(R.id.frrfar_test_result_time_tpl_upd);
        mTestTplUpdTimeTextView.setVisibility(View.GONE);

        mFingerManager = FingerManager.getDefault(this);
        mCommandMg = new AsyncCommand(mFingerManager);
        mTestImgOrig = SharedPreferencesData.loadDataBoolean(this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMAGE_ORIG_NAME, FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMAGE_ORIG_DEFAULT);

        if (mTestFinishBtn != null) {
            mTestFinishBtn.setOnClickListener(this);
            if (!FingerSettingsConst.FP_UI_BACK_BTN_SUPPORT) {
                mTestFinishBtn.setVisibility(View.GONE);
            }
        }
        if (mTestStartBtn != null) {
            mTestStartBtn.setOnClickListener(this);
        }
        if (mTestReportBtn != null) {
            mTestReportBtn.setOnClickListener(this);
        }
        if (mTestAgainBtn != null) {
            mTestAgainBtn.setOnClickListener(this);
        }

        if (mTestResultTargetTextView != null) {
            mTestResultTargetTextView.setText(getString(R.string.frrfar_test_result_target_info, 0.001 * FingerSettingsConst.FP_FAR_THOUSANDTH_PERCENT_MAX) + "%");
        }

        updateUI();
    }

    private void updateUI() {
        if (mTestStatus == STAUTS_TEST_FINISH) {
            mTestStatusTextView.setText(R.string.frrfar_test_info_test_finish);
            mTestStartBtn.setVisibility(View.GONE);
            mTestReportBtn.setVisibility(View.VISIBLE);
            mTestAgainBtn.setVisibility(View.VISIBLE);
            mTestProgressBar.setVisibility(View.INVISIBLE);
        } else if (mTestStatus == STATUS_TESTING) {
            mTestStatusTextView.setText(R.string.frrfar_test_info_testing);
            mTestStartBtn.setVisibility(View.VISIBLE);
            mTestStartBtn.setEnabled(false);
            mTestReportBtn.setVisibility(View.GONE);
            mTestAgainBtn.setVisibility(View.GONE);
            mTestProgressBar.setVisibility(View.VISIBLE);
        } else {
            mTestStatusTextView.setText(R.string.frrfar_test_info_wait_test);
            mTestStartBtn.setVisibility(View.VISIBLE);
            mTestStartBtn.setEnabled(true);
            mTestReportBtn.setVisibility(View.GONE);
            mTestAgainBtn.setVisibility(View.GONE);
            mTestProgressBar.setVisibility(View.INVISIBLE);
        }

        if (mTestCurrentImageInfoTextView != null) {
            if (mFingerIndicate != null) {
                mTestCurrentImageInfoTextView.setText(getString(R.string.frrfar_test_current_image_info, mFingerIndicate));
                mTestCurrentImageInfoTextView.setVisibility(View.VISIBLE);
            } else {
                mTestCurrentImageInfoTextView.setText("");
                mTestCurrentImageInfoTextView.setVisibility(View.GONE);
            }
        }
        if (mTestResultFailItemsTextView != null) {
            mTestResultFailItemsTextView.setText(String.valueOf(mTestAuthSuccessCount));
        }
        if (mTestResultFailTextView != null) {
            int rate = 0;
            if(0 != mTestCount) {
                rate = mTestAuthSuccessCount * 100 * 1000 / mTestCount;
            }
            if(rate >= FingerSettingsConst.FP_FAR_THOUSANDTH_PERCENT_MAX) {
                mTestResultFailTextView.setTextColor(R.color.red);
            } else {
                mTestResultFailTextView.setTextColor(R.color.green);
            }
            mTestResultFailTextView.setText(String.valueOf(mTestAuthSuccessCount) + "/" + mTestCount + "(" + (rate / 1000)  + "." + String.format("%03d", rate % 1000) + "%)");
        }

        if (mTestTplCount == 0) {
            mTestTplEnrollTimeTextView.setText(getString(R.string.frrfar_test_load_tpl_time, 0, 0.00));
        } else {
            mTestTplEnrollTimeTextView.setText(getString(R.string.frrfar_test_load_tpl_time, mTestTplEnrollTime, (float)mTestTplEnrollTime/mTestTplCount));
        }
        if (mTestAuthFailCount == 0) {
            mTestAuthFailedTimeTextView.setText(getString(R.string.frrfar_test_auth_failed_time, 0, 0.00));
        } else {
            mTestAuthFailedTimeTextView.setText(getString(R.string.frrfar_test_auth_failed_time, mTestAuthFailedTime, (float)mTestAuthFailedTime/mTestAuthFailCount));
        }
        if (mTestAuthSuccessCount == 0) {
            mTestAuthSuccessTimeTextView.setText(getString(R.string.frrfar_test_auth_success_time, 0, 0.00));
        } else {
            mTestAuthSuccessTimeTextView.setText(getString(R.string.frrfar_test_auth_success_time, mTestAuthSuccessTime, (float)mTestAuthSuccessTime/mTestAuthSuccessCount));
        }
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onPause() {
        super.onPause();

        if (mTestAsyncTask != null && !mTestAsyncTask.isCancelled()) {
            mTestAsyncTask.cancel(true);
        }
        mTestAsyncTask = null;

        if (mCommandMg != null) {
            mCommandMg.testSendImageFinish();
        }

        finish();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onClick(View v) {
        if (v == mTestFinishBtn) {
            finish();
        } else if (v == mTestReportBtn) {
        } else if (v == mTestStartBtn || v == mTestAgainBtn) {
            runTask();
        }
    }

    private void runTask() {
        if (mTestAsyncTask != null && !mTestAsyncTask.isCancelled()) {
            mTestAsyncTask.cancel(true);
        }
        mTestAsyncTask = new TestAsyncTask();
        mTestAsyncTask.execute();
    }

    private class AsyncCommand {
        private FingerManager mFingerManager;
        private Object mResultObj;
        private Object mClearResult;
        private boolean mFinish;

        public AsyncCommand(FingerManager fm) {
            mFingerManager = fm;
            mFinish = false;
        }

        public Object testSendImage(boolean frr, int index, boolean orig, int imgType, byte[] buffer) {
            if (FingerSettingsConst.LOG_DBG) {
                Log.v(FingerSettingsConst.LOG_TAG, "testSendImage: " + index + " [" + orig + "-" + frr + "-" + imgType +"]");
            }
            mResultObj = null;
            if (mFingerManager != null) {
                mFingerManager.testSendImage(index, orig, frr, imgType, buffer, mTestCmdCallback);
                synchronized (AsyncCommand.this) {
                    while (AsyncCommand.this.mResultObj == null && !mFinish) {
                        try {
                            AsyncCommand.this.wait();
                        } catch (InterruptedException e) {
                        }
                    }
                }
            }
            return mResultObj;
        }

        public void testSendImageNextFinger() {
            mClearResult = null;
            if (mFingerManager != null) {
                mFingerManager.testSendImageNextFinger(mTestCmdCallback);
                synchronized (AsyncCommand.this) {
                    while (AsyncCommand.this.mClearResult == null && !mFinish) {
                        try {
                            AsyncCommand.this.wait();
                        } catch (InterruptedException e) {
                        }
                    }
                }
            }
        }

        public void testSendImageFinish() {
            mFinish = true;
            if (mFingerManager != null) {
                mFingerManager.testFinish(mTestCmdCallback);
            }
        }

        private FingerManager.TestCmdCallback mTestCmdCallback = new FingerManager.TestCmdCallback() {
            @Override
            public void onTestResult(int cmdId, Object result) {
                if (cmdId == FingerManager.TEST_CMD_SEND_IMAGE_NEXT_FINGER && result instanceof FingerResult) {
                    if (FingerSettingsConst.LOG_DBG) {
                        Log.v(FingerSettingsConst.LOG_TAG, "testSendImageFinish rsp");
                    }
                    mClearResult = (int) 1;
                    synchronized (AsyncCommand.this) {
                        AsyncCommand.this.notifyAll();
                    }
                } else if (cmdId == FingerManager.TEST_CMD_SEND_IMAGE && result instanceof FingerFrrFarImageResult) {
                    if (FingerSettingsConst.LOG_DBG) {
                        Log.v(FingerSettingsConst.LOG_TAG, "onSendImageSuccess: send image success");
                    }
                    mResultObj = result;
                    synchronized (AsyncCommand.this) {
                        AsyncCommand.this.notifyAll();
                    }
                } else {
                    synchronized (AsyncCommand.this) {
                        AsyncCommand.this.notifyAll();
                    }
                }
            }
        };
    };

    private String getDateFormat() {
        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss");
        return df.format(new Date());
    }

    private String getTestResultFileName() {
        String name = getDateFormat();
        name += ".xml";
        return name;
    }

    public class TestAsyncTask extends AsyncTask<Integer, Integer, Integer> {
        @Override
        protected void onPreExecute() {
            mFarNeedRunFrrFirst = SharedPreferencesData.loadDataBoolean(FingerFarTestActivity.this, FingerSettingsConst.FP_SETTINGS_CONFIG,
                        FingerSettingsConst.FP_SETTINGS_TEST_FAR_RUN_FRR_FIRST_NAME, FingerSettingsConst.FP_SETTINGS_TEST_FAR_RUN_FRR_FIRST_DEFAULT);

            mTestCount = 0;
            mTestAuthFailCount = 0;
            mTestStatus = STATUS_TESTING;

            mTestTplEnrollPerFingerTime = 0;
            mTestTplEnrollTime = 0;
            mTestTplCount = 0;
            mTestAuthSuccessTime = 0;
            mTestAuthFailedTime = 0;
            mTestAuthSuccessCount = 0;

            publishProgress(mTestCount);
        }

        private int TestImage(File image, boolean runFrr) {
            int ret = -1;
            byte[] filecontent = null;

            if (image != null && image.isFile()) {
                Long filelength = image.length();
                filecontent = new byte[filelength.intValue()];
                try {
                    FileInputStream in = new FileInputStream(image);
                    in.read(filecontent);
                    in.close();
                    ret = 0;
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            if (!runFrr) {
                if (ret >= 0 && filecontent != null && filecontent.length > 0) {
                    Object result = mCommandMg.testSendImage(false, mTestImageCountPerFinger, mTestImgOrig, 0, filecontent);
                    if (result instanceof FingerFrrFarImageResult) {
                        FingerFrrFarImageResult frr = (FingerFrrFarImageResult) result;
                        if (frr.getErrorCode() == FingerManager.TEST_RESULT_OK) {
                            if (FingerSettingsConst.LOG_DBG) {
                                Log.d(FingerSettingsConst.LOG_TAG, "frr result:" + frr.toString());
                            }
                            if (frr.isAuth()) { // auth
                                if (frr.getResult() == 0) { // auth false
                                    mTestAuthFailCount++;
                                    mTestAuthFailedTime += frr.getTime();
                                } else {
                                    mTestImageAuthSuccessPerFinger++;
                                    mTestAuthSuccessCount++;
                                    mTestAuthSuccessTime += frr.getTime();
                                }
                                mTestImageCountPerFinger++;
                                mTestCount++;
                                TestResultXml.writeImgResult(false, image.getAbsolutePath(), 0, frr.getResult(), frr.getTime(), -1, null);
                            } else if (frr.isTpl()) { // enroll
                                mTestImageEnrollCountPerFinger++;
                                mTestTplEnrollPerFingerTime += frr.getTime();
                                if (frr.getResult() >= 100) {
                                    ret = 1;
                                    mTestTplEnrollTime += mTestTplEnrollPerFingerTime;
                                    mTestTplEnrollPerFingerTime = 0;
                                    mTestTplCount++;
                                }
                                TestResultXml.writeImgResult(false, image.getAbsolutePath(), 1, 1, frr.getTime(), -1, null);
                            }
                        } else {
                            mTestImageFileFailPerFinger++;
                            TestResultXml.writeImgResult(false, image.getAbsolutePath(), -1, -1, -1, -1, "image file op failed");
                        }
                    }
                } else {
                    Log.e(FingerSettingsConst.LOG_TAG, "get image file failed");
                    mTestImageFileFailPerFinger++;
                    TestResultXml.writeImgResult(false, image.getAbsolutePath(), -1, -1, -1, -1, "get image failed");
                }
            } else { // frr
                if (ret >= 0 && filecontent != null && filecontent.length > 0) {
                    Object result = mCommandMg.testSendImage(true, mTestImageCountPerFinger, mTestImgOrig, 0, filecontent);
                    if (result instanceof FingerFrrFarImageResult) {
                        FingerFrrFarImageResult frr = (FingerFrrFarImageResult) result;
                        if (frr.getErrorCode() == FingerManager.TEST_RESULT_OK) {
                            Log.d(FingerSettingsConst.LOG_TAG, "self-learning: " + frr.toString());
                            mTestImageCountPerFinger++;
                        } else {
                            Log.e(FingerSettingsConst.LOG_TAG, "self-learning: image file op failed: " + frr.getErrorCode());
                        }
                    }
                } else {
                    Log.e(FingerSettingsConst.LOG_TAG, "self-learning: get image file failed");
                }
                ret = 1;
            }

            mFingerIndicate = image.getAbsolutePath();

            publishProgress(mTestCount);
            return ret;
        }

        private int TestAllImages(File finger, File enrollFinger) {
            int ret = 0;
            int i = 0;
            if (finger != null && finger.isDirectory()) {
                File[] images = finger.listFiles(); // get all images folder
                if (images != null && images.length > 0) {
                    FingerSettingsConst.sortFiles(images);
                    if (enrollFinger == null) {  // load tpl
                        mTestImageCountPerFinger = 0;
                        mTestImageEnrollCountPerFinger = 0;
                        mTestImageFileFailPerFinger = 0;
                        mTestImageAuthSuccessPerFinger = 0;
                        mTestTplEnrollPerFingerTime = 0;

                        boolean runFrr = false;
                        TestResultXml.writeFingerTagStart(finger.getName());
                        while (i < images.length && !isCancelled()) {
                            ret = TestImage(images[i], runFrr);
                            if (ret > 0) {
                                if (mFarNeedRunFrrFirst) {
                                    runFrr = true;
                                } else {
                                    break;
                                }
                            }
                            i++;
                        }

                        if (ret > 0) {
                            TestAllUsersWithoutEnroll(finger);
                        }

                        TestResultXml.writeFingerResult(mTestImageCountPerFinger, mTestImageFileFailPerFinger, mTestImageEnrollCountPerFinger, mTestImageAuthSuccessPerFinger);
                        TestResultXml.writeFingerTagEnd();

                        if (!isCancelled()) {
                            mCommandMg.testSendImageNextFinger();
                        }
                    } else { // auth
                        if (!finger.getAbsolutePath().equals(enrollFinger.getAbsolutePath())) {
                            while (i < images.length && !isCancelled()) {
                                ret = TestImage(images[i], false);
                                i++;
                            }
                        } else {
                            Log.e(FingerSettingsConst.LOG_TAG, "TestAllImages: same finger, except");
                        }
                    }
                }
            }
            return ret;
        }

        private int TestAllFingers(File user, File enrollFinger) {
            int ret = 0;
            int i = 0;
            if (user != null && user.isDirectory()) {
                File[] fingers = user.listFiles(); // get all fingers folder
                if (fingers != null && fingers.length > 0) {
                    FingerSettingsConst.sortFiles(fingers);

                    if (enrollFinger == null) {
                        TestResultXml.writeUserTagStart(user.getName());
                    }
                    while (i < fingers.length && !isCancelled()) {
                        ret = TestAllImages(fingers[i], enrollFinger);
                        i++;
                    }
                    if (enrollFinger == null) {
                        TestResultXml.writeUserTagEnd();
                    }
                }
            }
            return ret;
        }

        private int TestAllUsersWithoutEnroll(File enrollFinger) {
            int ret = 0;
            int i = 0;
            File dataDir = FingerSettingsConst.getDataSavePath(FingerFarTestActivity.this, mTestImgOrig);
            if (dataDir != null && dataDir.isDirectory()) {
                File[] users = dataDir.listFiles(); // get all users folder
                if (users != null) {
                    FingerSettingsConst.sortFiles(users);
                    while (i < users.length && !isCancelled()) {
                        ret = TestAllFingers(users[i], enrollFinger);
                        i++;
                    }
                }
            }
            return ret;
        }

        private int TestAllUsers() {
            int ret = 0;
            int i = 0;
            File dataDir = FingerSettingsConst.getDataSavePath(FingerFarTestActivity.this, mTestImgOrig);
            if (dataDir != null && dataDir.isDirectory()) {
                File[] users = dataDir.listFiles(); // get all users folder
                if (users != null) {
                    FingerSettingsConst.sortFiles(users);
                    File resultDir = FingerSettingsConst.getTestResultPath(FingerFarTestActivity.this, false);
                    TestResultXml.init(FingerFarTestActivity.this, resultDir, getTestResultFileName());
                    while (i < users.length && !isCancelled()) {
                        ret = TestAllFingers(users[i], null);
                        i++;
                    }

                    float tpl_enroll_avg_time = (float)0.00;
                    float auth_success_avg_time = (float)0.00;
                    float auth_failed_avg_time = (float)0.00;
                    float tpl_upd_avg_time = (float)0.00;
                    if (mTestTplCount != 0) {
                        tpl_enroll_avg_time = (float)mTestTplEnrollTime/mTestTplCount;
                    }
                    if (mTestAuthFailCount != 0) {
                        auth_failed_avg_time = (float)mTestAuthFailedTime/mTestAuthFailCount;
                    }
                    if (mTestAuthSuccessCount != 0) {
                        auth_success_avg_time = (float)mTestAuthSuccessTime/mTestAuthSuccessCount;
                    }
                    TestResultXml.writeAllFingersResult(mTestCount, mTestAuthFailCount, tpl_enroll_avg_time, auth_success_avg_time, auth_failed_avg_time, tpl_upd_avg_time);
                    TestResultXml.finish(true);
                }
            }
            return ret;
        }

        @Override
        protected Integer doInBackground(Integer... params) {
            TestAllUsers();
            if (FingerSettingsConst.LOG_DBG) {
                Log.v(FingerSettingsConst.LOG_TAG, "doInBackground finished");
            }

            mTestStatus = STAUTS_TEST_FINISH;
            mFingerIndicate = null;
            publishProgress(mTestCount);

            return 0;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            updateUI();
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (FingerSettingsConst.LOG_DBG) {
                Log.v(FingerSettingsConst.LOG_TAG, "onPostExecute");
            }

            mTestAsyncTask = null;
        }
    }
}
