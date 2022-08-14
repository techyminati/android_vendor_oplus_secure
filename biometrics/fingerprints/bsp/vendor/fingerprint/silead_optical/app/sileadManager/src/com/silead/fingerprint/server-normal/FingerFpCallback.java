
package com.silead.fingerprint;

public interface FingerFpCallback {
    public void onCmdResponse(int cmdId, byte[] result);
}
