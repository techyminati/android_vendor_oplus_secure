
package com.silead.frrfar;

import java.io.File;
import java.io.FileOutputStream;

import libcore.io.IoUtils;

import org.xmlpull.v1.XmlSerializer;

import android.content.Context;
import android.util.AtomicFile;
import android.util.Xml;

public class EnrollResultXml {
    private static final String TAG_IMAGE = "img";

    private static final String ATTR_IMG_PATH = "path";
    private static final String ATTR_IMG_QUALITY = "quality";
    private static final String ATTR_IMG_AREA = "area";
    private static final String ATTR_IMG_GREY_AVG = "grey_avg";
    private static final String ATTR_IMG_GREY_MAX = "grey_max";

    private static File mFile = null;
    private static AtomicFile mDestination = null;
    private static FileOutputStream mOut = null;
    private static XmlSerializer mSerializer = null;

    public static int init(Context context, File path, String name) {
        if (mFile != null) {
            finish(false);
        }

        mFile = new File(path, name);
        mDestination = new AtomicFile(mFile);

        try {
            mOut = mDestination.startWrite();

            mSerializer = Xml.newSerializer();
            mSerializer.setOutput(mOut, "utf-8");
            mSerializer.setFeature("http://xmlpull.org/v1/doc/features.html#indent-output", true);
            mSerializer.startDocument(null, true);
        } catch (Throwable t) {
            if (mDestination != null) {
                mDestination.failWrite(mOut);
            }
            IoUtils.closeQuietly(mOut);
            mSerializer = null;
            mDestination = null;
            mFile = null;
        }

        return 0;
    }

    public static int writeImgInfo(String path, int quality, int area, int grey_avg, int grey_max) {
        if (mSerializer != null) {
            try {
                mSerializer.startTag(null, TAG_IMAGE);
                mSerializer.attribute(null, ATTR_IMG_PATH, path);
                mSerializer.attribute(null, ATTR_IMG_QUALITY, Integer.toString(quality));
                mSerializer.attribute(null, ATTR_IMG_AREA, Integer.toString(area));
                mSerializer.attribute(null, ATTR_IMG_GREY_AVG, Integer.toString(grey_avg));
                mSerializer.attribute(null, ATTR_IMG_GREY_MAX, Integer.toString(grey_max));
                mSerializer.endTag(null, TAG_IMAGE);
            } catch (Throwable t) {
            }
        }
        return 0;
    }

    public static int finish(boolean success) {
        try {
            if (success) {
                mSerializer.endDocument();
                if (mDestination != null) {
                    mDestination.finishWrite(mOut);
                }
            } else {
                if (mDestination != null) {
                    mDestination.failWrite(mOut);
                }
            }
        } catch (Throwable t) {
            if (mDestination != null) {
                mDestination.failWrite(mOut);
            }
        } finally {
            IoUtils.closeQuietly(mOut);
            mSerializer = null;
            mDestination = null;
            mFile = null;
        }
        return 0;
    }
}
