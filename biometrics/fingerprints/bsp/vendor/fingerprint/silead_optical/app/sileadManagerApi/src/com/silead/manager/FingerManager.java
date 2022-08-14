
package com.silead.manager;

import android.content.Context;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.silead.internal.IFingerService;
import com.silead.internal.IFingerServiceReceiver;

public class FingerManager {
    private static final String TAG = "FingerManager";
    private static final boolean DBG = true;

    static public final String FINGERPRINT_TEST_SERVICE = "com.silead.fingerprintService";
    private static final boolean IS_VERIFY_DATA = true;
    private static final byte[] VERIFY_DATA = {
            0x73, 0x6c, 0x66, 0x70
    };

    public static final int TEST_CMD_TEST_BASE              = 0;
    public static final int TEST_CMD_SPI                    = TEST_CMD_TEST_BASE;
    public static final int TEST_CMD_RESET_PIN              = TEST_CMD_TEST_BASE + 1;
    public static final int TEST_CMD_DEAD_PIXEL             = TEST_CMD_TEST_BASE + 2;
    public static final int TEST_CMD_GET_VERSION            = TEST_CMD_TEST_BASE + 3;
    public static final int TEST_CMD_GET_IMAGE              = TEST_CMD_TEST_BASE + 4;
    public static final int TEST_CMD_SEND_IMAGE             = TEST_CMD_TEST_BASE + 6;
    public static final int TEST_CMD_SEND_IMAGE_NEXT_FINGER = TEST_CMD_TEST_BASE + 7;
    public static final int TEST_CMD_SELF_TEST              = TEST_CMD_TEST_BASE + 8;
    public static final int TEST_CMD_SPEED_TEST             = TEST_CMD_TEST_BASE + 9;
    public static final int TEST_CMD_TEST_FINISH            = TEST_CMD_TEST_BASE + 10;
    public static final int TEST_CMD_CALIBRATE              = TEST_CMD_TEST_BASE + 11;
    public static final int TEST_CMD_OPTIC_CALIBRATE_STEP   = TEST_CMD_TEST_BASE + 12;
    public static final int TEST_CMD_SEND_FINGER_DOWN       = TEST_CMD_TEST_BASE + 13;
    public static final int TEST_CMD_SEND_FINGER_UP         = TEST_CMD_TEST_BASE + 14;
    public static final int TEST_CMD_TEST_FLASH             = TEST_CMD_TEST_BASE + 15;
    public static final int TEST_CMD_TEST_OTP               = TEST_CMD_TEST_BASE + 16;
    public static final int TEST_CMD_ICON_READY             = TEST_CMD_TEST_BASE + 17;

    public static final int TEST_RESULT_IMAGE_ICON_CHANGE = -100;
    public static final int TEST_RESULT_IMAGE_SAVE_FAILED = -3;
    public static final int TEST_RESULT_DATA_IMCOMPLITE = -2;
    public static final int TEST_RESULT_SERVICE_FAILED = -1;
    public static final int TEST_RESULT_OK = 0;
    public static final int TEST_RESULT_BAD_PARAM = 1000;
    public static final int TEST_RESULT_NO_FINGER = 1018;
    public static final int TEST_RESULT_MOVE_TOO_FAST = 1019;
    public static final int TEST_RESULT_ENROLL_SAME_AREA = 1020;
    public static final int TEST_RESULT_ENROLL_QUALITY_FAILED = 1021;
    public static final int TEST_RESULT_ENROLL_COVERAREA_FAILED = 1022;
    public static final int TEST_RESULT_ENROLL_QUALITY_COVERAREA_FAILED = 1023;
    public static final int TEST_RESULT_ENROLL_FAKE_FINGER = 1024;
    public static final int TEST_RESULT_ENROLL_GAIN_IMPROVE_TIMEOUT = 1025;
    public static final int TEST_RESULT_CANCELED = 1030;

    private static final int MSG_SEND_RESPONSE = 101;

    private static FingerManager sInstance = null;

    private Handler mHandler;
    private IBinder mToken = new Binder();

    TestCmdCallback mTestCmdCallback;

    public static FingerManager getDefault(Context context) {
        if (sInstance == null) {
            sInstance = new FingerManager(context);
        }
        return sInstance;
    }

    public FingerManager(Context context) {
        mHandler = new MyHandler(context);
    }

    private IFingerService getIFingerService() {
        return IFingerService.Stub.asInterface(ServiceManager.getService(FINGERPRINT_TEST_SERVICE));
    }

    // ***************************************************************//
    public void getAllVerInfo(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "getAllVerInfo");
        }
        testCmd(TEST_CMD_GET_VERSION, data, callback);
    }

    public void testSpi(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testSpi");
        }
        testCmd(TEST_CMD_SPI, data, callback);
    }

    public void testResetPin(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testResetPin");
        }
        testCmd(TEST_CMD_RESET_PIN, data, callback);
    }

    public void testDeadPixel(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testDeadPixel");
        }
        testCmd(TEST_CMD_DEAD_PIXEL, data, callback);
    }

    public void testSpeed(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testSpeed");
        }
        testCmd(TEST_CMD_SPEED_TEST, data, callback);
    }

    public void testGetImage(int mode, int countPerDown, int delay, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + 3];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[3];
        }

        data[offset++] = (byte) (mode & 0xFF);
        data[offset++] = (byte) (countPerDown & 0xFF);
        data[offset++] = (byte) (delay & 0xFF);
        if (DBG) {
            Log.d(TAG, "testGetImage (" + mode + ":" + countPerDown + ":" + delay + ")");
        }
        testCmd(TEST_CMD_GET_IMAGE, data, callback);
    }

    public void testSendImage(int index, boolean orig, boolean frr, int imgType, byte[] buffer, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + buffer.length + 7];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[buffer.length + 7];
        }

        data[offset++] = (byte) ((index >> 24) & 0xFF);
        data[offset++] = (byte) ((index >> 16) & 0xFF);
        data[offset++] = (byte) ((index >> 8) & 0xFF);
        data[offset++] = (byte) (index & 0xFF);
        data[offset++] = (byte) (orig ? 1 : 0);
        data[offset++] = (byte) (frr ? 1 : 0);
        data[offset++] = (byte) (imgType & 0xFF);

        System.arraycopy(buffer, 0, data, offset, buffer.length);
        if (DBG) {
            Log.d(TAG, "testSendImage (" + index + ":" + orig + ":" + frr + ":" + imgType + ")");
        }
        testCmd(TEST_CMD_SEND_IMAGE, data, callback);
    }

    public void testSendImageNextFinger(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testSendImageNextFinger");
        }
        testCmd(TEST_CMD_SEND_IMAGE_NEXT_FINGER, data, callback);
    }

    public void testFinish(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testFinish");
        }
        testCmd(TEST_CMD_TEST_FINISH, data, callback);
    }

    public void selfTest(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "selfTest");
        }
        testCmd(TEST_CMD_SELF_TEST, data, callback);
    }

    public void calibrate(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "calibrate");
        }
        testCmd(TEST_CMD_CALIBRATE, data, callback);
    }

    public void calibrateStep(int step, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + 1];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[1];
        }

        data[offset++] = (byte) (step & 0xFF);
        if (DBG) {
            Log.d(TAG, "calibrateStep (" + step + ")");
        }
        testCmd(TEST_CMD_OPTIC_CALIBRATE_STEP, data, callback);
    }

    public void testSendFingerDownMsg(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testSendFingerDownMsg");
        }
        testCmd(TEST_CMD_SEND_FINGER_DOWN, data, callback);
    }

    public void testSendFingerDownMsgWithFlag(byte flag, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + 1];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[1];
        }

        data[offset++] = (byte)flag;
        if (DBG) {
            Log.d(TAG, "testSendFingerDownMsgWithFlag (" + flag + ")");
        }
        testCmd(TEST_CMD_SEND_FINGER_DOWN, data, callback);
    }

    public void testSendFingerUpMsg(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testSendFingerUpMsg");
        }
        testCmd(TEST_CMD_SEND_FINGER_UP, data, callback);
    }

    public void testSendFingerUpMsgWithFlag(byte flag, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + 1];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[1];
        }

        data[offset++] = (byte)flag;
        if (DBG) {
            Log.d(TAG, "testSendFingerUpMsgWithFlag (" + flag + ")");
        }
        testCmd(TEST_CMD_SEND_FINGER_UP, data, callback);
    }

    public void testFlash(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testFlash");
        }
        testCmd(TEST_CMD_TEST_FLASH, data, callback);
    }

    public void testOTP(TestCmdCallback callback) {
        byte[] data = null;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
        }
        if (DBG) {
            Log.d(TAG, "testOTP");
        }
        testCmd(TEST_CMD_TEST_OTP, data, callback);
    }

    public void testIconReadyFlag(byte flag, TestCmdCallback callback) {
        byte[] data = null;
        int offset = 0;
        if (IS_VERIFY_DATA) {
            data = new byte[VERIFY_DATA.length + 1];
            System.arraycopy(VERIFY_DATA, 0, data, 0, VERIFY_DATA.length);
            offset = VERIFY_DATA.length;
        } else {
            data = new byte[1];
        }

        data[offset++] = (byte) flag;
        if (DBG) {
            Log.d(TAG, "testIconReadyFlag (" + flag + ")");
        }
        testCmd(TEST_CMD_ICON_READY, data, callback);
    }

    public void testCmd(int cmdId, byte[] param, TestCmdCallback callback) {
        int ret = -1;
        mTestCmdCallback = callback;
        try {
            ret = getIFingerService().testCmd(mToken, cmdId, param, mServiceReceiver);
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
        }

        if (ret < 0) {
            int offset = 0;
            byte[] data = new byte[4];
            data[offset++] = (byte) ((ret >> 24) & 0xFF);
            data[offset++] = (byte) ((ret >> 16) & 0xFF);
            data[offset++] = (byte) ((ret >> 8) & 0xFF);
            data[offset++] = (byte) (ret & 0xFF);
            onTestCmdResult(cmdId, data);
        }
    }

    // ***************************************************************//
    public int onTestCmdResult(int cmdId, byte[] result) {
        int ret = 1;
        Object obj = null;

        if (DBG) {
            Log.d(TAG, "onTestCmdResult: cmdId=" + cmdId + ", result.length=" + (result == null ? 0 : result.length));
        }

        switch (cmdId) {
            case TEST_CMD_GET_VERSION: {
                obj = FingerVersion.parse(result);
                break;
            }
            case TEST_CMD_SPI: {
                obj = FingerSpiResult.parse(result);
                break;
            }
            case TEST_CMD_DEAD_PIXEL: {
                obj = FingerDeadPixelResult.parse(result);
                break;
            }
            case TEST_CMD_SPEED_TEST: {
                obj = FingerSpeedResult.parse(result);
                ret = 0;
                break;
            }
            case TEST_CMD_GET_IMAGE: {
                obj = FingerFrrFarEnroll.parse(result);
                ret = 0;
                break;
            }
            case TEST_CMD_SEND_IMAGE: {
                obj = FingerFrrFarImageResult.parse(result);
                break;
            }
            case TEST_CMD_OPTIC_CALIBRATE_STEP: {
                obj = CalibrateStepResult.parse(result);
                break;
            }
            case TEST_CMD_TEST_OTP: {
                obj = FingerOTPResult.parse(result);
                break;
            }
            case TEST_CMD_RESET_PIN:
            case TEST_CMD_SEND_IMAGE_NEXT_FINGER:
            case TEST_CMD_TEST_FINISH:
            case TEST_CMD_SELF_TEST:
            case TEST_CMD_CALIBRATE:
            case TEST_CMD_SEND_FINGER_DOWN: 
            case TEST_CMD_SEND_FINGER_UP: 
            case TEST_CMD_TEST_FLASH:
            case TEST_CMD_ICON_READY: {
                obj = FingerResult.parse(result);
                break;
            }
        }

        if (obj != null) {
            mHandler.obtainMessage(MSG_SEND_RESPONSE, cmdId, 0, obj).sendToTarget();
        } else {
            if (DBG) {
                Log.d(TAG, "msg:" + cmdId + " response is null");
            }
        }

        return ret;
    }

    private IFingerServiceReceiver mServiceReceiver = new IFingerServiceReceiver.Stub() {
        @Override
        public int onTestCmd(int cmdId, byte[] result) throws RemoteException {
            return onTestCmdResult(cmdId, result);
        }
    };

    public static abstract class TestCmdCallback {
        public void onTestResult(int cmdId, Object result) {
        }
    };

    private class MyHandler extends Handler {
        private MyHandler(Context context) {
            super(context.getMainLooper());
        }

        private MyHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(android.os.Message msg) {
            if (DBG) {
                Log.d(TAG, "handleMessage: what:" + msg.what + ", cmd:" + msg.arg1 + ", mTestCmdCallback:" + mTestCmdCallback);
            }

            switch (msg.what) {
                case MSG_SEND_RESPONSE: {
                    if (mTestCmdCallback != null) {
                        if (msg.obj != null) {
                            Log.d(TAG, "cmd:" + msg.arg1 + " "+ msg.obj.toString());
                        }
                        mTestCmdCallback.onTestResult(msg.arg1, msg.obj);
                    }
                    break;
                }
            }
        }
    };
}
