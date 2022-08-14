
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public final class FingerFrrFarEnroll implements Parcelable {
    private int mErrCode;
    private int mImageQuality;
    private int mEffectiveArea;
    private int mOriginal;
    private int mStep;
    private int mIsTpl;
    private int mGreyAvg;
    private int mGreyMax;
    private byte[] mData;

    public static final int DATA_TYPE_OBSOLETE = 0;
    public static final int DATA_TYPE_PARAM = 1;
    public static final int DATA_TYPE_IMAGE = 2;

    public FingerFrrFarEnroll(int errcode) {
        mErrCode = errcode;
        mGreyAvg = 0;
        mGreyMax = 0;
        mData = new byte[0];
    }

    public void setParamData(byte[] data, int offset, int dataLen) {
        if (data != null && data.length >= offset + dataLen) {
            if (dataLen >= 5) {
                mImageQuality = (0xFF & data[offset++]);
                mEffectiveArea = (0xFF & data[offset++]);
                mOriginal = (0xFF & data[offset++]);
                mStep = (0xFF & data[offset++]);
                mIsTpl = (0xFF & data[offset++]);
            }
            if (dataLen >= 7) {
                mGreyAvg = (0xFF & data[offset++]);
                mGreyMax = (0xFF & data[offset++]);
            }
        }
    }

    public void setImageData(byte[] data, int offset, int dataLen) {
        if (data != null && data.length >= offset + dataLen) {
            mData = new byte[dataLen];
            System.arraycopy(data, offset, mData, 0, dataLen);
        }
    }

    public void setObsoleteData(byte[] data, int offset, int dataLen) {
        if (dataLen > 6 && data.length >= offset + dataLen) {
            mImageQuality = (0xFF & data[offset++]);
            mEffectiveArea = (0xFF & data[offset++]);
            int orig = (0xFF & data[offset++]);
            mOriginal = orig & 0x01;
            mStep = ((orig >> 1) & 0x007F);
            mIsTpl = (0xFF & data[offset++]);
            mGreyAvg = (0xFF & data[offset++]);
            mGreyMax = (0xFF & data[offset++]);
            setImageData(data, offset, dataLen - 6);
        } else {
            mErrCode = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
        }
    }

    private FingerFrrFarEnroll(Parcel in) {
        mErrCode = in.readInt();
        mImageQuality = in.readInt();
        mEffectiveArea = in.readInt();
        mOriginal = in.readInt();
        mStep = in.readInt();
        mIsTpl = in.readInt();
        mGreyAvg = in.readInt();
        mGreyMax = in.readInt();
        mData = in.createByteArray();
    }

    public int getErrCode() {
        return mErrCode;
    }

    public int getImageQuality() {
        return mImageQuality;
    }

    public int getEffectiveArea() {
        return mEffectiveArea;
    }

    public boolean isImgOrig() {
        return (mOriginal != 0);
    }

    public int getStep() {
        return mStep;
    }

    public int getGreyAvg() {
        return mGreyAvg;
    }

    public int getGreyMax() {
        return mGreyMax;
    }

    public byte[] getData() {
        return mData;
    }

    public boolean isTplImage() {
        return (mIsTpl != 0);
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mErrCode);
        out.writeInt(mImageQuality);
        out.writeInt(mEffectiveArea);
        out.writeInt(mOriginal);
        out.writeInt(mStep);
        out.writeInt(mIsTpl);
        out.writeInt(mGreyAvg);
        out.writeInt(mGreyMax);
        out.writeByteArray(mData);
    }

    public static final Parcelable.Creator<FingerFrrFarEnroll> CREATOR = new Parcelable.Creator<FingerFrrFarEnroll>() {
        public FingerFrrFarEnroll createFromParcel(Parcel in) {
            return new FingerFrrFarEnroll(in);
        }

        public FingerFrrFarEnroll[] newArray(int size) {
            return new FingerFrrFarEnroll[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("error:").append(mErrCode).append(" ");
        sb.append("quality:").append(mImageQuality).append(" ");
        sb.append("area:").append(mEffectiveArea).append(" ");
        sb.append("orig:").append(mOriginal).append(" ");
        sb.append("step:").append(mStep).append(" ");
        sb.append("istpl:").append(mIsTpl).append(" ");
        sb.append("greyavg:").append(mGreyAvg).append(" ");
        sb.append("greymax:").append(mGreyMax);
        return sb.toString();
    }

    public static FingerFrrFarEnroll parse(byte[] result) {
        int err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;
        int dataLen = 0;
        FingerFrrFarEnroll enrollResult = null;

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            enrollResult = new FingerFrrFarEnroll(err);
            if (result.length >= offset + 4) {
                int type_check = (0xFF & result[offset]);
                if (type_check == DATA_TYPE_OBSOLETE) {
                    dataLen = (0xFF & result[offset++]) << 24;
                    dataLen |= (0xFF & result[offset++]) << 16;
                    dataLen |= (0xFF & result[offset++]) << 8;
                    dataLen |= (0xFF & result[offset++]);

                    enrollResult.setObsoleteData(result, offset, dataLen);
                } else {
                    while (result.length >= offset + 5) {
                        int type = (0xFF & result[offset++]);
                        dataLen = (0xFF & result[offset++]) << 24;
                        dataLen |= (0xFF & result[offset++]) << 16;
                        dataLen |= (0xFF & result[offset++]) << 8;
                        dataLen |= (0xFF & result[offset++]);

                        if (result.length >= offset + dataLen) {
                            if (type == DATA_TYPE_PARAM) {
                                enrollResult.setParamData(result, offset, dataLen);
                            } else if (type == DATA_TYPE_IMAGE) {
                                enrollResult.setImageData(result, offset, dataLen);
                            }
                        }
                        offset += dataLen;
                    }
                }
            }
        }

        if (enrollResult == null) {
            enrollResult = new FingerFrrFarEnroll(FingerManager.TEST_RESULT_DATA_IMCOMPLITE);
        }

        return enrollResult;
    }
};
