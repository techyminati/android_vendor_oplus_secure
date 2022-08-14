
package com.silead.fingerprint;

import java.util.ArrayList;
import java.util.NoSuchElementException;

import android.content.Context;
import android.os.IBinder;
import android.os.IBinder.DeathRecipient;
import android.os.IHwBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.silead.manager.FingerManager;
import com.silead.internal.IFingerService;
import com.silead.internal.IFingerServiceReceiver;

import vendor.silead.hardware.fingerprintext.V1_0.ISileadFingerprint;
import vendor.silead.hardware.fingerprintext.V1_0.ISileadFingerprintCallback;

public class FingerService extends IFingerService.Stub implements IHwBinder.DeathRecipient {
    private static final String LOG_TAG = "fingerprintservice";
    private static final boolean DBG = true;

    private static final String USE_FINGERPRINT="android.permission.USE_FINGERPRINT";

    private Context mContext;
    private ClientMonitor mRequestClient;
    private ISileadFingerprint mDaemon;

    public FingerService(Context context) {
        mContext = context;
        publish();
        getFingerprintExtDaemon();
    }

    private void publish() {
        if (DBG) {
            Log.d(LOG_TAG, "publish: " + FingerManager.FINGERPRINT_TEST_SERVICE);
        }
        ServiceManager.addService(FingerManager.FINGERPRINT_TEST_SERVICE, this);
    }

    public synchronized ISileadFingerprint getFingerprintExtDaemon() {
        if (mDaemon == null) {
            Log.v(LOG_TAG, "mDeamon was null, reconnect to silead fingerprint");
            try {
                mDaemon = ISileadFingerprint.getService();
            } catch (java.util.NoSuchElementException e) {
                // Service doesn't exist or cannot be opened. Logged below.
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Failed to get silead fingerprint interface", e);
            }

            if (mDaemon == null) {
                Log.w(LOG_TAG, "fingerprint HIDL not available");
                return null;
            }

            mDaemon.asBinder().linkToDeath(this, 0);
            try {
                mDaemon.setNotify(mDaemonCallback);
            } catch (RemoteException e) {
                Log.w(LOG_TAG, "Failed to open fingerprint HAL", e);
                mDaemon = null; // try again later!
            }
        }
        return mDaemon;
    }

    private ISileadFingerprintCallback mDaemonCallback = new ISileadFingerprintCallback.Stub() {
        @Override
        public void onCmdResponse(final int cmdid, ArrayList<Byte> result) {
            final byte[] byteResult = new byte[result.size()];
            for (int i = 0; i < result.size(); i++) {
                byteResult[i] = result.get(i);
            }

            if (DBG) {
                Log.d(LOG_TAG, "onCmdResponse cmdid=" + cmdid + ", mRequestClient=" + mRequestClient);
            }
            if (mRequestClient != null) {
                mRequestClient.sendRequestResult(cmdid, byteResult);
            }
        }
    };

    @Override
    public void serviceDied(long cookie) {
        Log.v(LOG_TAG, "silead fingerprint demo died");
        mDaemon = null;
    }

    @Override
    public int testCmd(final IBinder token, int cmdId, byte[] param, IFingerServiceReceiver receiver) {
        checkPermission(USE_FINGERPRINT);

        ISileadFingerprint daemon = getFingerprintExtDaemon();
        if (daemon == null) {
            Log.w(LOG_TAG, "testCmd: no fingeprint test demo!");
            return -1;
        }

        removeClient(mRequestClient);
        mRequestClient = new ClientMonitor(token, receiver);

        try {
            ArrayList<Byte> paramList = new ArrayList<>();
            for (int i = 0; i < param.length; i++) {
                paramList.add(param[i]);
            }
            return daemon.requestCmd(cmdId, paramList);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, "testCmd failed", e);
        }

        return 0;
    }

    void checkPermission(String permission) {
        mContext.enforceCallingOrSelfPermission(permission,
                "Must have " + permission + " permission.");
    }

    private void removeClient(ClientMonitor client) {
        if (client == null) {
            return;
        }
        client.destroy();
        if (client == mRequestClient) {
            mRequestClient = null;
        }
    }

    private class ClientMonitor implements IBinder.DeathRecipient {
        IBinder token;
        IFingerServiceReceiver receiver;

        public ClientMonitor(IBinder token, IFingerServiceReceiver receiver) {
            this.token = token;
            this.receiver = receiver;
            try {
                token.linkToDeath(this, 0);
            } catch (RemoteException e) {
                Log.w(LOG_TAG, "caught remote exception in linkToDeath: ", e);
            }
        }

        public void destroy() {
            if (token != null) {
                try {
                    token.unlinkToDeath(this, 0);
                } catch (NoSuchElementException e) {
                    Log.e(LOG_TAG, "destroy(): " + this + ":", new Exception("here"));
                }
                token = null;
            }
            receiver = null;
        }

        @Override
        public void binderDied() {
            token = null;
            removeClient(this);
            receiver = null;
        }

        @Override
        protected void finalize() throws Throwable {
            try {
                if (token != null) {
                    if (DBG) {
                        Log.w(LOG_TAG, "removing leaked reference: " + token);
                    }
                    removeClient(this);
                }
            } finally {
                super.finalize();
            }
        }

        private boolean sendRequestResult(int cmdId, byte[] result) {
            if (receiver == null) {
                return true; // client not listening
            }

            try {
                int ret = receiver.onTestCmd(cmdId, result);
                if (DBG) {
                    Log.d(LOG_TAG, "sendRequestResult ret=" + ret);
                }
                return ret > 0 ? true : false;
            } catch (RemoteException e) {
                Log.w(LOG_TAG, "Failed to notify RequestResult:", e);
                return true;
            }
        }
    }
}
