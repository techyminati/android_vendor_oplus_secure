
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

import java.io.UnsupportedEncodingException;

/**
 * @hide
 */
public final class FingerVersion implements Parcelable {
    private CharSequence mHalVersion;
    private CharSequence mDevVersion;
    private CharSequence mAlgoVersion;
    private CharSequence mTaVersion;

    public FingerVersion(CharSequence hal, CharSequence dev, CharSequence algo, CharSequence ta) {
        mHalVersion = hal;
        mDevVersion = dev;
        mAlgoVersion = algo;
        mTaVersion = ta;
    }

    private FingerVersion(Parcel in) {
        mHalVersion = in.readString();
        mDevVersion = in.readString();
        mAlgoVersion = in.readString();
        mTaVersion = in.readString();
    }

    public CharSequence getHalVersion() {
        return mHalVersion;
    }

    public CharSequence getAlgoVersion() {
        return mAlgoVersion;
    }

    public CharSequence getDevVersion() {
        return mDevVersion;
    }

    public CharSequence getTaVersion() {
        return mTaVersion;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mHalVersion.toString());
        out.writeString(mDevVersion.toString());
        out.writeString(mAlgoVersion.toString());
        out.writeString(mTaVersion.toString());
    }

    public static final Parcelable.Creator<FingerVersion> CREATOR = new Parcelable.Creator<FingerVersion>() {
        public FingerVersion createFromParcel(Parcel in) {
            return new FingerVersion(in);
        }

        public FingerVersion[] newArray(int size) {
            return new FingerVersion[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("hal:").append(mHalVersion).append(" ");
        sb.append("dev:").append(mDevVersion).append(" ");
        sb.append("algo:").append(mAlgoVersion).append(" ");
        sb.append("ta:").append(mTaVersion);
        return sb.toString();
    }

    private static String getDefaultVersion() {
        return "unknow";
    }

    private static String getVersionValue(byte[] result, int offset) {
        if (result != null && result.length > offset && result.length >= (0xFF & result[offset]) + offset + 1) {
            try {
                return new String(result, offset + 1, (0xFF & result[offset]), "UTF-8");
            } catch (UnsupportedEncodingException e) {
            }
        }
        return null;
    }

    public static FingerVersion parse(byte[] result) {
        String[] strVersion = {null, null, null, null};
        int i = 0;
        int count = 4;
        int err = 0;
        int offset = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (FingerManager.TEST_RESULT_OK == err) { // successful
                for (i = 0; i < count; i++) {
                    strVersion[i] = getVersionValue(result, offset);
                    if (strVersion[i] == null) {
                        break;
                    }
                    offset += (0xFF & result[offset]) + 1;
                }
            }
        }

        for (i = 0; i < count; i++) {
            if (strVersion[i] == null) {
                strVersion[i] = getDefaultVersion();
            }
        }

        return new FingerVersion(strVersion[0], strVersion[1], strVersion[2], strVersion[3]);
    }
};
