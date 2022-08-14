
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class CalibrateStepResult implements Parcelable {
    private int mErrCode;
    private int mStep;
    private int[] mData;
    private int mCount;

    public CalibrateStepResult(int err, int step) {
        mErrCode = err;
        mStep = step;
        mData = new int[0];
        mCount = 0;
    }

    public void setData(byte[] data, int offset, int dataLen) {
        if (data != null && data.length >= offset + dataLen && dataLen >= 4) {
            int count = (0xFF & data[offset++]) << 24;
            count |= (0xFF & data[offset++]) << 16;
            count |= (0xFF & data[offset++]) << 8;
            count |= (0xFF & data[offset++]);
            if (count > 0 && data.length >= offset + count*4) {
                mData = new int[count];
                mCount = count;
                for (int i = 0; i < count; i++) {
                    mData[i] = (0xFF & data[offset++]) << 24;
                    mData[i] |= (0xFF & data[offset++]) << 16;
                    mData[i] |= (0xFF & data[offset++]) << 8;
                    mData[i] |= (0xFF & data[offset++]);
                }
            }
        }
    }

    private CalibrateStepResult(Parcel in) {
        mErrCode = in.readInt();
        mStep = in.readInt();
        mData = in.createIntArray();
        mCount = in.readInt();
    }

    public int getErrorCode() {
        return mErrCode;
    }

    public int getStep() {
        return mStep;
    }

    public String getDataStr() {
        StringBuilder sb = new StringBuilder();

        sb.append("[");
        for (int i = 0; i < mCount; i++) {
            if (i == 0) {
                sb.append(String.format("%d", mData[i]));
            } else {
                sb.append(String.format(",%d", mData[i]));
            }
        }
        sb.append("]");

        return sb.toString();
    }

    public boolean hasData() {
        return (mData.length > 0);
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mErrCode);
        out.writeInt(mStep);
        out.writeIntArray(mData);
    }

    public static final Parcelable.Creator<CalibrateStepResult> CREATOR = new Parcelable.Creator<CalibrateStepResult>() {
        public CalibrateStepResult createFromParcel(Parcel in) {
            return new CalibrateStepResult(in);
        }

        public CalibrateStepResult[] newArray(int size) {
            return new CalibrateStepResult[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("error:").append(mErrCode).append(" ");
        sb.append("step:").append(mStep).append(" ");
        sb.append("count:").append(mCount);
        return sb.toString();
    }

    public static CalibrateStepResult parse(byte[] result) {
        int err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;
        int step = -1;
        boolean extDataValid = false;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (result.length >= offset + 1) {
                step = (0xFF & result[offset++]);
            }
            if (result.length >= offset + 1) {
                extDataValid = true;
            }
        }

        CalibrateStepResult rst = new CalibrateStepResult(err, step);
        if (extDataValid) {
            rst.setData(result, offset, result.length - offset);
        }
        return rst;
    }
};
