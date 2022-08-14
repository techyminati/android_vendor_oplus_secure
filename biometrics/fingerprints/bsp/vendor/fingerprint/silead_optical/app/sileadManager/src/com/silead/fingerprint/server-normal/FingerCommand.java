
package com.silead.fingerprint;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;

import com.silead.manager.FingerManager;

class FPRequest {
    int mRequest;
    byte[] data;

    static FPRequest obtain(int request, byte[] param) {
        FPRequest rr = new FPRequest();
        rr.mRequest = request;
        rr.data = new byte[param.length + 4];
        rr.data[0] = (byte)((request >> 24) & 0xff);
        rr.data[1] = (byte)((request >> 16) & 0xff);
        rr.data[2] = (byte)((request >> 8) & 0xff);
        rr.data[3] = (byte)((request) & 0xff);
        System.arraycopy(param, 0, rr.data, 4, param.length);

        return rr;
    }

    private FPRequest() {
    }
}

public class FingerCommand {
    private static final String LOG_TAG = "FingerCommand";
    private static final boolean DBG = true;

    static final String FP_SOCKET_NAME = "com_silead_fpext";
    static final int MAX_COMMAND_BYTES = (150 * 1024);
    static final int SOCKET_OPEN_RETRY_MILLIS = 4 * 1000;

    LocalSocket mSocket;

    Thread mReceiverThread;
    ResultReceiver mReceiver;

    FingerFpCallback mFingerprintCallback = null;

    public FingerCommand() {
        mReceiver = new ResultReceiver();
        mReceiverThread = new Thread(mReceiver, "FPReceiver");
        mReceiverThread.start();
    }

    public void setFpCallback(FingerFpCallback callback) {
        mFingerprintCallback = callback;
    }

    private static int readFpMessage(InputStream is, byte[] buffer) throws IOException {
        int countRead;
        int offset;
        int remaining;
        int messageLength;

        // First, read in the length of the message
        offset = 0;
        remaining = 4;
        do {
            countRead = is.read(buffer, offset, remaining);

            if (countRead < 0 ) {
                Log.e(LOG_TAG, "Hit EOS reading message length");
                return -1;
            }

            offset += countRead;
            remaining -= countRead;
        } while (remaining > 0);

        messageLength = ((buffer[0] & 0xff) << 24) | ((buffer[1] & 0xff) << 16) | ((buffer[2] & 0xff) << 8) | (buffer[3] & 0xff);

        // Then, re-use the buffer and read in the message itself
        offset = 0;
        remaining = messageLength;
        do {
            countRead = is.read(buffer, offset, remaining);

            if (countRead < 0 ) {
                Log.e(LOG_TAG, "Hit EOS reading message.  messageLength=" + messageLength + " remaining=" + remaining);
                return -1;
            }

            offset += countRead;
            remaining -= countRead;
        } while (remaining > 0);

        return messageLength;
    }

    class ResultReceiver implements Runnable {
        byte[] buffer;

        ResultReceiver() {
            buffer = new byte[MAX_COMMAND_BYTES];
        }

        @Override
        public void run() {
            int retryCount = 0;
            String fpSocketName = FP_SOCKET_NAME;

            try {
                for (;;) {
                    LocalSocket s = null;
                    LocalSocketAddress l;

                    try {
                        s = new LocalSocket();
                        l = new LocalSocketAddress(fpSocketName, LocalSocketAddress.Namespace.ABSTRACT);
                        s.connect(l);
                    } catch (IOException ex){
                        try {
                            if (s != null) {
                                s.close();
                            }
                        } catch (IOException ex2) {
                            //ignore failure to close after failure to connect
                        }

                        if (retryCount == 8) {
                            Log.e(LOG_TAG, "Couldn't find '" + fpSocketName + "' socket after " + retryCount + " times, continuing to retry silently");
                        } else if (retryCount >= 0 && retryCount < 8) {
                            Log.i(LOG_TAG, "Couldn't find '" + fpSocketName + "' socket; retrying after timeout");
                        }

                        try {
                            Thread.sleep(SOCKET_OPEN_RETRY_MILLIS);
                        } catch (InterruptedException er) {
                        }

                        retryCount++;
                        continue;
                    }

                    retryCount = 0;

                    mSocket = s;
                    Log.i(LOG_TAG, "Connected to '" + fpSocketName + "' socket");

                    int length = 0;
                    try {
                        InputStream is = mSocket.getInputStream();

                        for (;;) {
                            length = readFpMessage(is, buffer);
                            if (length < 0) { // End-of-stream reached
                                break;
                            }

                            processResponse(buffer, length);
                        }
                    } catch (java.io.IOException ex) {
                        Log.i(LOG_TAG, "'" + fpSocketName + "' socket closed", ex);
                    } catch (Throwable tr) {
                        Log.e(LOG_TAG, "Uncaught exception read length=" + length + "Exception:" + tr.toString());
                    }

                    Log.i(LOG_TAG, "Disconnected from '" + fpSocketName + "' socket");

                    try {
                        mSocket.close();
                    } catch (IOException ex) {
                    }

                    mSocket = null;
                }
            } catch (Throwable tr) {
                Log.e(LOG_TAG,"Uncaught exception", tr);
            }
        }
    }

    private void processResponse(byte[] buffer, int len) {
        int cmd;
        byte[] result = null;

        if (len < 4) {
            return;
        }

        cmd = ((buffer[0] & 0xff) << 24) | ((buffer[1] & 0xff) << 16) | ((buffer[2] & 0xff) << 8) | (buffer[3] & 0xff);
        if (len > 4) {
            result = new byte[len - 4];
            System.arraycopy(buffer, 4, result, 0, len - 4);
        }

        if (mFingerprintCallback != null) {
            mFingerprintCallback.onCmdResponse(cmd, result);
        }
    }

    public int requestCmd(int cmdId, byte[] param) {
        try {
            LocalSocket s = mSocket;
            if (s == null) {
                return FingerManager.TEST_RESULT_SERVICE_FAILED;
            }

            if (param.length + 4 > MAX_COMMAND_BYTES) {
                Log.e(LOG_TAG, "Parcel larger than max bytes allowed! " + param.length);
                return FingerManager.TEST_RESULT_SERVICE_FAILED;
            }

            FPRequest rr = FPRequest.obtain(cmdId, param);
            byte[] dataLength = new byte[4];
            dataLength[0] = dataLength[1] = 0;
            dataLength[2] = (byte)((rr.data.length >> 8) & 0xff);
            dataLength[3] = (byte)((rr.data.length) & 0xff);

            s.getOutputStream().write(dataLength);
            s.getOutputStream().write(rr.data);
        } catch (IOException ex) {
            Log.e(LOG_TAG, "IOException", ex);
            return FingerManager.TEST_RESULT_SERVICE_FAILED;
        } catch (RuntimeException exc) {
            Log.e(LOG_TAG, "Uncaught exception ", exc);
            return FingerManager.TEST_RESULT_SERVICE_FAILED;
        }
        return 0;
    }
}
