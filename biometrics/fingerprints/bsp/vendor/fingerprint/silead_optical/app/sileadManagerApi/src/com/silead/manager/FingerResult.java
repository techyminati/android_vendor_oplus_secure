
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerResult implements Parcelable {
    private int mErrCode;

    public FingerResult(int err) {
        mErrCode = err;
    }

    private FingerResult(Parcel in) {
        mErrCode = in.readInt();
    }

    public int getErrorCode() {
        return mErrCode;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mErrCode);
    }

    public static final Parcelable.Creator<FingerResult> CREATOR = new Parcelable.Creator<FingerResult>() {
        public FingerResult createFromParcel(Parcel in) {
            return new FingerResult(in);
        }

        public FingerResult[] newArray(int size) {
            return new FingerResult[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("error:").append(mErrCode);
        return sb.toString();
    }

    public static FingerResult parse(byte[] result) {
        int err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);
        }

        return new FingerResult(err);
    }
};
