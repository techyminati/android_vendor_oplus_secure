
package com.silead.internal;

import android.os.IBinder;
import com.silead.internal.IFingerServiceReceiver;

/**
 * @hide
 */
interface IFingerService {
    int testCmd(IBinder token, int cmdId, in byte[] param, IFingerServiceReceiver receiver);
}
