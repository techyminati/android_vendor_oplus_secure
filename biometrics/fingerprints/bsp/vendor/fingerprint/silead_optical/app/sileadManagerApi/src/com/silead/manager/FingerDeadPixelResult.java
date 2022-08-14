
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerDeadPixelResult implements Parcelable {
    private int mErrCode;
    private int mResult;
    private int mDeadPixelNum;
    private int mBadlineNum;

    public FingerDeadPixelResult(int err, int result, int deadPixelnum, int badlinenum) {
        mErrCode = err;
        mResult = result;
        mDeadPixelNum = deadPixelnum;
        mBadlineNum = badlinenum;
    }

    private FingerDeadPixelResult(Parcel in) {
        mErrCode = in.readInt();
        mResult = in.readInt();
        mDeadPixelNum = in.readInt();
        mBadlineNum = in.readInt();
    }

    public int getErrorCode() {
        return mErrCode;
    }

    public int getResult() {
        return mResult;
    }

    public int getDeadPixelNum() {
        return mDeadPixelNum;
    }

    public int getBadlineNum() {
        return mBadlineNum;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mErrCode);
        out.writeInt(mResult);
        out.writeInt(mDeadPixelNum);
        out.writeInt(mBadlineNum);
    }

    public static final Parcelable.Creator<FingerDeadPixelResult> CREATOR = new Parcelable.Creator<FingerDeadPixelResult>() {
        public FingerDeadPixelResult createFromParcel(Parcel in) {
            return new FingerDeadPixelResult(in);
        }

        public FingerDeadPixelResult[] newArray(int size) {
            return new FingerDeadPixelResult[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("error:").append(mErrCode).append(" ");
        sb.append("result:").append(mResult).append(" ");
        sb.append("deadpix:").append(mDeadPixelNum).append(" ");
        sb.append("badline:").append(mBadlineNum);
        return sb.toString();
    }

    public static FingerDeadPixelResult parse(byte[] result) {
        int err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
        int rlt = 0;
        int offset = 0;
        int deadpixelnum = 0;
        int badlinenum = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (err == FingerManager.TEST_RESULT_OK) {
                if (result.length >= offset + 9) {
                    rlt = (0xFF & result[offset++]);
                    deadpixelnum = (0xFF & result[offset++]) << 24;
                    deadpixelnum |= (0xFF & result[offset++]) << 16;
                    deadpixelnum |= (0xFF & result[offset++]) << 8;
                    deadpixelnum |= (0xFF & result[offset++]);
                    badlinenum = (0xFF & result[offset++]) << 24;
                    badlinenum |= (0xFF & result[offset++]) << 16;
                    badlinenum |= (0xFF & result[offset++]) << 8;
                    badlinenum |= (0xFF & result[offset++]);
                } else {
                    err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
                }
            }
        }

        return new FingerDeadPixelResult(err, rlt, deadpixelnum, badlinenum);
    }
};
