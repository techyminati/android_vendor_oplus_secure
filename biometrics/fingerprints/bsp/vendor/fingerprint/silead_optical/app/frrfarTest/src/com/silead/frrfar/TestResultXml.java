
package com.silead.frrfar;

import java.io.File;
import java.io.FileOutputStream;

import libcore.io.IoUtils;

import org.xmlpull.v1.XmlSerializer;

import android.content.Context;
import android.util.AtomicFile;
import android.util.Xml;

public class TestResultXml {
    private static final String TAG_USERS = "users";
    private static final String TAG_USER = "user";
    private static final String TAG_FINGER = "finger";
    private static final String TAG_IMAGE = "img";
    private static final String TAG_RESULT = "result";

    private static final String ATTR_NAME = "name";
    private static final String ATTR_IMG_TPL = "is_tpl";
    private static final String ATTR_IMG_RESULT = "result";
    private static final String ATTR_IMG_TIME = "time";
    private static final String ATTR_IMG_TPL_UPD_TIME = "tpl_upd_time";
    private static final String ATTR_IMG_ERR = "err";
    private static final String ATTR_COUNT = "count";
    private static final String ATTR_FILE_FAIL_COUNT = "file_fail_count";
    private static final String ATTR_ENROLL_COUNT = "enroll_count";
    private static final String ATTR_AUTH_FAIL_COUNT = "autn_fail_count";
    private static final String ATTR_ENROLL_AVG_TIME = "tpl_enroll_avg_time";
    private static final String ATTR_AUTH_SUCCESS_AVG_TIME = "auth_success_avg_time";
    private static final String ATTR_AUTH_FAILED_AVG_TIME = "auth_failed_avg_time";
    private static final String ATTR_TPL_UPD_AVG_TIME = "tpl_upd_avg_time";

    private static File mFile = null;
    private static AtomicFile mDestination = null;
    private static FileOutputStream mOut = null;
    private static XmlSerializer mSerializer = null;

    private static boolean mSaveImgTestResult = true;

    private static String mPath;

    private static void writeFile(String path, String value) {
        if (path != null) {
            mPath = path;
        }
        try {
            File file = new File(mPath, "status");
            FileOutputStream fos =  new FileOutputStream(file);
            fos.write(value.getBytes());
            fos.flush();
            fos.close();
        } catch(Exception e) {
        }
    }

    public static int init(Context context, File path, String name) {
        if (mFile != null) {
            finish(false);
        }

        writeFile(path.getAbsolutePath(), "runing");

        mSaveImgTestResult = SharedPreferencesData.loadDataBoolean(context, FingerSettingsConst.FP_SETTINGS_CONFIG, FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMG_RESULT_NAME, 
                                FingerSettingsConst.FP_SETTINGS_TEST_FRRFAR_IMG_RESULT_DEFAULT);

        mFile = new File(path, name);
        mDestination = new AtomicFile(mFile);

        try {
            mOut = mDestination.startWrite();

            mSerializer = Xml.newSerializer();
            mSerializer.setOutput(mOut, "utf-8");
            mSerializer.setFeature("http://xmlpull.org/v1/doc/features.html#indent-output", true);
            mSerializer.startDocument(null, true);
            mSerializer.startTag(null, TAG_USERS);
        } catch (Throwable t) {
            if (mDestination != null) {
                mDestination.failWrite(mOut);
            }
            IoUtils.closeQuietly(mOut);
            mSerializer = null;
            mDestination = null;
            mFile = null;
        }

        return 0;
    }

    public static int writeUserTagStart(String name) {
        if (mSerializer != null) {
            try {
                mSerializer.startTag(null, TAG_USER);
                mSerializer.attribute(null, ATTR_NAME, name);
            } catch (Throwable t) {
            }
        }
        return 0;
    }

    public static int writeUserTagEnd() {
        if (mSerializer != null) {
            try {
                mSerializer.endTag(null, TAG_USER);
            } catch (Throwable t) {
            }
        }
        return 0;
    }

    public static int writeFingerTagStart(String name) {
        if (mSerializer != null) {
            try {
                mSerializer.startTag(null, TAG_FINGER);
                mSerializer.attribute(null, ATTR_NAME, name);
            } catch (Throwable t) {
            }
        }
        return 0;
    }

    public static int writeFingerTagEnd() {
        if (mSerializer != null) {
            try {
                mSerializer.endTag(null, TAG_FINGER);
            } catch (Throwable t) {
            }
        }
        return 0;
    }

    public static int writeFingerResult(int count, int file_fail_count, int enroll_count, int auth_fail_count) {
        if (mSerializer != null) {
            try {
                mSerializer.startTag(null, TAG_RESULT);
                mSerializer.attribute(null, ATTR_COUNT, Integer.toString(count));
                mSerializer.attribute(null, ATTR_FILE_FAIL_COUNT, Integer.toString(file_fail_count));
                mSerializer.attribute(null, ATTR_ENROLL_COUNT, Long.toString(enroll_count));
                mSerializer.attribute(null, ATTR_AUTH_FAIL_COUNT, Long.toString(auth_fail_count));
                mSerializer.endTag(null, TAG_RESULT);
            } catch (Throwable t) {
            }
        }
        return 0;
    }

    public static int writeImgResult(boolean isfrr, String name, int istpl, int result, int time, int tpl_upd_time, String errDes) {
        if (mSaveImgTestResult) {
            if (mSerializer != null) {
                try {
                    mSerializer.startTag(null, TAG_IMAGE);
                    mSerializer.attribute(null, ATTR_NAME, name);
                    mSerializer.attribute(null, ATTR_IMG_TPL, Integer.toString(istpl));
                    if (isfrr || istpl > 0) {
                        mSerializer.attribute(null, ATTR_IMG_RESULT, (result >= 0) ? ((result > 0) ? "PASS" : "FAILED") : Integer.toString(result));
                    } else {
                        mSerializer.attribute(null, ATTR_IMG_RESULT, (result >= 0) ? ((result > 0) ? "FAILED" : "PASS") : Integer.toString(result));
                    }
                    mSerializer.attribute(null, ATTR_IMG_TIME, Long.toString(time));
                    if (isfrr && istpl == 0 && result > 0) {
                        mSerializer.attribute(null, ATTR_IMG_TPL_UPD_TIME, Long.toString(tpl_upd_time));
                    }
                    if (errDes != null) {
                        mSerializer.attribute(null, ATTR_IMG_ERR, errDes);
                    }
                    mSerializer.endTag(null, TAG_IMAGE);
                } catch (Throwable t) {
                }
            }
        }
        return 0;
    }

    public static int writeAllFingersResult(int count, int auth_fail_count, float tpl_enroll_avg_time, float auth_success_avg_time, float auth_failed_avg_time, float tpl_upd_avg_time) {
        if (mSerializer != null) {
            try {
                mSerializer.startTag(null, TAG_RESULT);
                mSerializer.attribute(null, ATTR_COUNT, Integer.toString(count));
                mSerializer.attribute(null, ATTR_AUTH_FAIL_COUNT, Long.toString(auth_fail_count));
                mSerializer.attribute(null, ATTR_ENROLL_AVG_TIME, Float.toString(tpl_enroll_avg_time));
                if (auth_success_avg_time > 0) {
                    mSerializer.attribute(null, ATTR_AUTH_SUCCESS_AVG_TIME, Float.toString(auth_success_avg_time));
                }
                mSerializer.attribute(null, ATTR_AUTH_FAILED_AVG_TIME, Float.toString(auth_failed_avg_time));
                mSerializer.attribute(null, ATTR_TPL_UPD_AVG_TIME, Float.toString(tpl_upd_avg_time));

                mSerializer.endTag(null, TAG_RESULT);
            } catch (Throwable t) {
            }
        }
        return 0;
    }

    public static int finish(boolean success) {
        try {
            if (success) {
                mSerializer.endTag(null, TAG_USERS);
                mSerializer.endDocument();
                if (mDestination != null) {
                    mDestination.finishWrite(mOut);
                }
            } else {
                if (mDestination != null) {
                    mDestination.failWrite(mOut);
                }
            }
        } catch (Throwable t) {
            if (mDestination != null) {
                mDestination.failWrite(mOut);
            }
        } finally {
            IoUtils.closeQuietly(mOut);
            mSerializer = null;
            mDestination = null;
            mFile = null;

            writeFile(null, "finish");
        }
        return 0;
    }
}
