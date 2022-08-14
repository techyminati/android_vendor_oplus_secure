
package com.silead.manager;

import android.os.Parcel;
import android.os.Parcelable;

import java.io.UnsupportedEncodingException;

import java.lang.Integer;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * @hide
 */
public final class FingerOTPResult implements Parcelable {
    private int mErrCode;
    private ArrayList<Integer> mOTPInfoList = new ArrayList<>();
    private CharSequence mOPTDetail;

    public FingerOTPResult(int err, ArrayList OTPInfoList, CharSequence detail) {
        mErrCode = err;
        mOTPInfoList = OTPInfoList;
        mOPTDetail = detail;
    }

    private FingerOTPResult(Parcel in) {
        mErrCode = in.readInt();
        mOTPInfoList = in.readArrayList(Integer.class.getClassLoader());
        mOPTDetail = in.readString();
    }

    public int getErrorCode() {
        return mErrCode;
    }

    public ArrayList<Integer> getOTPInfoList() {
        return mOTPInfoList;
    }

    public CharSequence getOTPDetails() {
        return mOPTDetail;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mErrCode);
        out.writeList(mOTPInfoList);
        out.writeString(mOPTDetail.toString());
    }

    public static final Parcelable.Creator<FingerOTPResult> CREATOR = new Parcelable.Creator<FingerOTPResult>() {
        public FingerOTPResult createFromParcel(Parcel in) {
            return new FingerOTPResult(in);
        }

        public FingerOTPResult[] newArray(int size) {
            return new FingerOTPResult[size];
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("[");
        Iterator<Integer> iterator = mOTPInfoList.iterator();
        if (iterator.hasNext()) {
            sb.append(String.format("0x%08x", iterator.next()).toUpperCase());
        }
        while(iterator.hasNext()) {
            sb.append(",").append(String.format("0x%08x", iterator.next()).toUpperCase());
        }
        sb.append("]");

        return sb.toString();
    }

    public static FingerOTPResult parse(byte[] result) {
        int err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
        int offset = 0;
        int count = 0;
        ArrayList<Integer> infoList = new ArrayList<>();
        CharSequence detail = "";

        if (result != null && result.length >= 4) {
            err = (0xFF & result[offset++]) << 24;
            err |= (0xFF & result[offset++]) << 16;
            err |= (0xFF & result[offset++]) << 8;
            err |= (0xFF & result[offset++]);

            if (FingerManager.TEST_RESULT_OK == err) { // successful
                if (result.length >= offset + 1) {
                    count = (0xFF & result[offset++]);
                    if (result.length >= offset + count * 4) {
                        for(int i = 0; i < count; i++) {
                            int data = 0;
                            data = (0xFF & result[offset++]) << 24;
                            data |= (0xFF & result[offset++]) << 16;
                            data |= (0xFF & result[offset++]) << 8;
                            data |= (0xFF & result[offset++]);
                            infoList.add(data);
                        }

                        if (result.length >= offset + 1) {
                            count = (0xFF & result[offset++]);
                            if (result.length >= offset + count) {
                                try {
                                    detail = new String(result, offset, count, "UTF-8");
                                } catch (UnsupportedEncodingException e) {
                                }
                            }
                        }
                    } else {
                        err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
                    }
                } else {
                    err = FingerManager.TEST_RESULT_DATA_IMCOMPLITE;
                }
            }
        }

        return new FingerOTPResult(err, infoList, detail);
    }
};
