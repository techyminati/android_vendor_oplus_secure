
package com.silead.factorytest;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;

public class FactoryTestConst {
    public static final String LOG_TAG = "factoryTest";
    public static final boolean LOG_DBG = true;
    public static final boolean FP_UI_BACK_BTN_SUPPORT = true;

    public static final String FP_SETTINGS_PACKAGE_NAME = "com.silead.frrfar";

    public static final String FP_SETTINGS_CONFIG = "fp_settings";
    public static final String FP_SETTINGS_IS_OPTIC_NAME = "is_optic";
    public static final String FP_SETTINGS_FACTORY_FINGER_DUMP_IMAGE_NAME = "dump_enable";
    public static final String FP_SETTINGS_FACTORY_CLEAR_PWD_AUTO_NAME = "auto_clear";
    public static final boolean FP_UI_OPTIC_MODE_DEFAULT = true;
    public static final boolean FP_SETTINGS_FACTORY_FINGER_DUMP_IMAGE_DEFAULT = false;
    public static final boolean FP_SETTINGS_FACTORY_CLEAR_PWD_AUTO_DEFAULT = false;

    public static final String OPTIC_SETTINGS_CONFIG = "optic_settings";
    public static final String OPTIC_SETTINGS_FINGER_IMAGE_WIDTH_NAME = "layout_width";
    public static final String OPTIC_SETTINGS_FINGER_IMAGE_HEIGHT_NAME = "layout_height";
    public static final String OPTIC_SETTINGS_FINGER_IMAGE_Y_NAME = "layout_y";
    public static final String OPTIC_SETTINGS_FINGER_IMAGE_NAME = "img_name";
    public static final String OPTIC_SETTINGS_FINGER_IMAGE_TOUCH_ENABLE_NAME = "touch_enable";

    public static final int OPTIC_SETTINGS_FINGER_IMAGE_WIDTH_DEFAULT = 190;
    public static final int OPTIC_SETTINGS_FINGER_IMAGE_HEIGHT_DEFAULT = 190;
    public static final int OPTIC_SETTINGS_FINGER_IMAGE_Y_DEFAULT = 278;
    public static final String OPTIC_SETTINGS_FINGER_IMAGE_ID_DEFAULT = "finger_image_green";
    public static final boolean OPTIC_SETTINGS_FINGER_IMAGE_TOUCH_ENABLE_DEFAULT = false;

    public static final int CMD_TIMEOUT_MS = 2000; // 2s

    public static final String LAUNCH_COUNT_KEY = "COUNT";
    public static final String LAUNCH_MODE_KEY = "MODE";
    public static final String LAUNCH_MODE_PCBA = "PCBA";
    public static final String LAUNCH_MODE_NORMAL = "NORMAL";

    public static boolean isPCBAMode(String mode) {
        boolean PCBAMode = false;

        if (mode != null && !mode.isEmpty()) {
            if(FactoryTestConst.LAUNCH_MODE_PCBA.equals(mode)) {
                PCBAMode = true;
            }
        }

        return PCBAMode;
    }

    public static StringBuilder readFile(String filePath, String charsetName) {
        File file = new File(filePath);
        StringBuilder fileContent = new StringBuilder("");
        if (file == null || !file.isFile()) {
            return null;
        }

        BufferedReader reader;
        try {
            InputStreamReader is = new InputStreamReader(new FileInputStream(file), charsetName);
            reader = new BufferedReader(is);
            String line;
            while ((line = reader.readLine()) != null) {
                fileContent = new StringBuilder(line);
            }
            reader.close();
            is.close();
            return fileContent;
        } catch (IOException e) {
            throw new RuntimeException("IOException occurred. ", e);
        } finally {
        }
    }

    public static boolean writeFile(String filePath, String content, boolean append) {
        if (content == null || content.isEmpty()) {
            return false;
        }

        Log.d(FactoryTestConst.LOG_TAG, "write: " + content);

        FileWriter fileWriter;
        try {
            makeDirs(filePath);
            fileWriter = new FileWriter(filePath, append);
            fileWriter.write(content + "\r\n");
            fileWriter.close();
            return true;
        } catch (IOException e) {
            throw new RuntimeException("IOException occurred. ", e);
        } finally {
        }
    }

    public static boolean makeDirs(String filePath) {
        String folderName = getFolderName(filePath);
        if (folderName == null || folderName.isEmpty()) {
            return false;
        }

        File folder = new File(folderName);
        return (folder.exists() && folder.isDirectory()) ? true : folder.mkdirs();
    }

    public static String getFolderName(String filePath) {
        if (filePath == null || filePath.isEmpty()) {
            return filePath;
        }

        int filePosi = filePath.lastIndexOf(File.separator);
        return (filePosi == -1) ? "" : filePath.substring(0, filePosi);
    }
}