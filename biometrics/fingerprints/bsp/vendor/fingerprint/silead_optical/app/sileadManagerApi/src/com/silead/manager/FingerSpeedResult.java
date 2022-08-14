
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerSpeedResult implements Parcelable {
    private int mErrCode;
    private int mResult;
    private int mImageQuality;
    private int mEffectiveArea;
    private int mCaptureTime;
    private int mReduceBgNoiseTime;
    private int mAuthTime;
    private int mTplUpdTime;

    public FingerSpeedResult(int err, int result, int quality, int area, int capturetime, int reducenoisetime, int authtime, int tplupdtime) {
        mErrCode = err;
        mResult = result;
        mImageQuality = quality;
        mEffectiveArea = area;
        mCaptureTime = capturetime;
        mReduceBgNoiseTime = reducenoisetime;
        mAuthTime = authtime;
        mTplUpdTime = tplupdtime;
    }

    private FingerSpeedResult(Parcel in) {
        mErrCode = in.readInt();
        mResult = in.readInt();
        mImageQuality = in.readInt();
        mEffectiveArea = in.readInt();
        mCaptureTime = in.readInt();
        mReduceBgNoiseTime = in.readInt();
        mAuthTime = in.readInt();
        mTplUpdTime = in.readInt();
    }

    public int getErrorCode() {
        return mErrCode;
    }

    public int getResult() {
        return mResult;
    }

    public int getImageQuality() {
        return mImageQuality;
    }

    public int getEffectiveArea() {
        return mEffectiveArea;
    }
    
    public int getCaptureTime() {
        return mCaptureTime;
    }

    public int getReduceBgNoiseTime() {
        return mReduceBgNoiseTime;
    }
    
    public int getAuthTime() {
        return mAuthTime;
    }

    public int getTplUpdTime() {
        return mTplUpdTime;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mErrCode);
        out.writeInt(mResult);
        out.writeInt(mImageQuality);
        out.writeInt(mEffectiveArea);
        out.writeInt(mCaptureTime);
        out.writeInt(mReduceBgNoiseTime);
        out.writeInt(mAuthTime);
        out.writeInt(mTplUpdTime);
    }

    public static final Parcelable.Creator<FingerSpeedResult> CREATOR = new Parcelable.Creator<FingerSpeedResult>() {
        public FingerSpeedResult createFromParcel(Parcel in) {
            return new FingerSpeedResult(in);
        }

        public FingerSpeedResult[] newArray(int size) {
            return new FingerSpeedResult[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("error:").append(mErrCode).append(" ");
        sb.append("result:").append(mResult).append(" ");
        sb.append("quality:").append(mImageQuality).append(" ");
        sb.append("area:").append(mEffectiveArea).append(" ");
        sb.append("captureTime:").append(mCaptureTime).append(" ");
        sb.append("ReduceBgNoiseTime:").append(mReduceBgNoiseTime).append(" ");
        sb.append("authTime:").append(mAuthTime).append(" ");
        sb.append("tplUpdTime:").append(mTplUpdTime);
        return sb.toString();
    }

    public static FingerSpeedResult parse(byte[] result) {
        int err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
        int rlt = 0;
        int offset = 0;
        int qualityscore = 0;
        int converarea = 0;
        int capture_time = 0;
        int reduce_noise_time = 0;
        int auth_time = 0;
        int tpl_upd_time = 0;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (err == FingerManager.TEST_RESULT_OK) {
                if (result.length >= offset + 19) {
                    rlt = (0xFF & result[offset++]);
                    qualityscore = (0xFF & result[offset++]);
                    converarea = (0xFF & result[offset++]);
                    capture_time = (0xFF & result[offset++]) << 24;
                    capture_time |= (0xFF & result[offset++]) << 16;
                    capture_time |= (0xFF & result[offset++]) << 8;
                    capture_time |= (0xFF & result[offset++]);
                    reduce_noise_time = (0xFF & result[offset++]) << 24;
                    reduce_noise_time |= (0xFF & result[offset++]) << 16;
                    reduce_noise_time |= (0xFF & result[offset++]) << 8;
                    reduce_noise_time |= (0xFF & result[offset++]);
                    auth_time = (0xFF & result[offset++]) << 24;
                    auth_time |= (0xFF & result[offset++]) << 16;
                    auth_time |= (0xFF & result[offset++]) << 8;
                    auth_time |= (0xFF & result[offset++]);
                    tpl_upd_time = (0xFF & result[offset++]) << 24;
                    tpl_upd_time |= (0xFF & result[offset++]) << 16;
                    tpl_upd_time |= (0xFF & result[offset++]) << 8;
                    tpl_upd_time |= (0xFF & result[offset++]);
                } else {
                    err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
                }
            }
        }

        return new FingerSpeedResult(err, rlt, qualityscore, converarea, capture_time, reduce_noise_time, auth_time, tpl_upd_time);
    }
};
