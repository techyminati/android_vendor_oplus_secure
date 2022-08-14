
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

import java.io.UnsupportedEncodingException;

/**
 * @hide
 */
public final class FingerSpiResult implements Parcelable {
    private CharSequence mChipId;

    public FingerSpiResult(CharSequence chipid) {
        mChipId = chipid;
    }

    private FingerSpiResult(Parcel in) {
        mChipId = in.readString();
    }

    public CharSequence getChipId() {
        return mChipId;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mChipId.toString());
    }

    public static final Parcelable.Creator<FingerSpiResult> CREATOR = new Parcelable.Creator<FingerSpiResult>() {
        public FingerSpiResult createFromParcel(Parcel in) {
            return new FingerSpiResult(in);
        }

        public FingerSpiResult[] newArray(int size) {
            return new FingerSpiResult[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("chipid:").append(mChipId);
        return sb.toString();
    }

    private static String getDefaultChipId() {
        return "unknow";
    }

    public static FingerSpiResult parse(byte[] result) {
        String strChipId = getDefaultChipId();
        int err = 0;
        int offset = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (FingerManager.TEST_RESULT_OK == err) { // successful
                if ((0xFF & result[offset]) + 2 <= result.length) {
                    try {
                        strChipId = new String(result, offset + 1, (0xFF & result[offset]), "UTF-8");
                    } catch (UnsupportedEncodingException e) {
                    }
                }
            }
        }

        return new FingerSpiResult(strChipId);
    }
};
