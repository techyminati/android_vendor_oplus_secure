
package com.silead.frrfar;

import java.io.File;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.content.Context;
import android.graphics.BitmapFactory;

public class FingerSettingsConst {
    public static final String LOG_TAG = "frrfarTest";
    public static final boolean LOG_DBG = true;

    public static final int FP_FRR_THOUSANDTH_PERCENT_MAX = 2000; // 2%
    public static final int FP_FAR_THOUSANDTH_PERCENT_MAX = 2;    // 0.002%

    public static final boolean FP_UI_BACK_BTN_SUPPORT = true;
    private static final boolean DATA_FILE_NAME_INDICATE_FINGER = true;
    private static final int MIN_USER_NAME_LEN = 4;
    private static final int MIN_DATA_FILE_NAME_LEN = 4;

    public static final String FRR_FAR_DATA_FILE_SUFFIX = ".bmp";

    public static final String ENROLL_SETTING_CONFIG = "enroll_cfg";
    public static final String ENROLL_SETTING_COUNT_NAME = "count";
    public static final String ENROLL_SETTING_IMG_DIY_NAME = "img_diy";
    public static final String ENROLL_SETTING_IMG_DELAY_NAME = "img_delay";
    public static final String ENROLL_SETTING_ID_IMAGE_NAME_PERFIX = "id-";

    public static final int ENROLL_SETTING_COUNT_MAX = 100;
    public static final int ENROLL_SETTING_COUNT_DEFAULT = 0;
    public static final int ENROLL_SETTING_DISABLE = 0;
    public static final boolean ENROLL_SETTING_IMG_DIY_DEFAULT = false;
    public static final int ENROLL_SETTING_IMG_DELAY_DEFAULT = 30;

    public static final String ENROLL_ENV_CONFIG = "enroll_env";
    public static final String ENROLL_ENV_NEXT_HAND_NAME = "next_hand";
    public static final String ENROLL_ENV_NEXT_FINGER_NAME = "next_finger";
    public static final String ENROLL_ENV_NEXT_USERID_NAME = "next_userid";

    public static final String ENROLL_ENV_NEXT_USER_DEFAULT = "0001";
    public static final int ENROLL_ENV_NEXT_HAND_DEFAULT = 0;
    public static final int ENROLL_ENV_NEXT_FINGER_DEFAULT = 0;

    public static final String FP_SETTINGS_CONFIG = "fp_settings";
    public static final String FP_SETTINGS_SAMPLE_COUNT_NAME = "sample_count";
    public static final String FP_SETTINGS_ENROLL_FINGER_NAME = "finger_enroll";
    public static final String FP_SETTINGS_TEST_FRRFAR_IMG_RESULT_NAME = "farfar_img_result";
    public static final String FP_SETTINGS_TEST_FAR_RUN_FRR_FIRST_NAME = "far_run_frr_first";
    public static final String FP_SETTINGS_TEST_FRRFAR_IMAGE_ORIG_NAME = "farfrr_img_orig";
    public static final String FP_SETTINGS_OTHER_IS_OPTIC_NAME = "is_optic";
    public static final String FP_SETTINGS_OTHER_GATHER_FINGER_NAME = "gather_finger";
    public static final String FP_SETTINGS_OTHER_IMG_DISPLAY_NAME = "img_display";
    public static final String FP_SETTINGS_OTHER_SEPCIAL_SETTINGS_HIDEN_NAME = "spec_no";
    public static final String FP_SETTINGS_FACTORY_FINGER_DUMP_IMAGE_NAME = "dump_enable";
    public static final String FP_SETTINGS_FACTORY_CLEAR_PWD_AUTO_NAME = "auto_clear";

    public static final int OTHER_GATHER_MODE_FINAL = 0;
    public static final int OTHER_GATHER_MODE_ORIG = 1;
    public static final int OTHER_GATHER_MODE_ALL = 2;

    public static final int FP_SETTINGS_SAMPLE_COUNT_DEFAULT = 50;
    public static final int FP_SETTINGS_ENROLL_FINGER_DEFAULT = 0x07; // 0 0111
    public static final boolean FP_SETTINGS_TEST_FRRFAR_IMG_RESULT_DEFAULT = true;
    public static final boolean FP_SETTINGS_TEST_FAR_RUN_FRR_FIRST_DEFAULT = true;
    public static final boolean FP_SETTINGS_TEST_FRRFAR_IMAGE_ORIG_DEFAULT = false;
    public static final boolean FP_SETTINGS_OTHER_IS_OPTIC_DEFAULT = true;
    public static final int FP_SETTINGS_OTHER_GATHER_FINGER_DEFAULT = OTHER_GATHER_MODE_FINAL;
    public static final boolean FP_SETTINGS_OTHER_IMG_DISPLAY_DEFAULT = false;
    public static final boolean FP_SETTINGS_OTHER_SEPCIAL_SETTINGS_HIDEN_DEFAULT = true;
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

    // should the same size with enroll_env_set_hand_values in arrays.xml
    private static final String[] HAND_STR = {
            "L", "R"
    };
    // should the same size with finger_settings_enroll_finger_values in arrays.xml
    private static final String[] FINGER_STR = {
            "1", "2", "3", "4", "5"
    };

    public static File getDataSavePath(Context context, boolean original) {
        String image_folder = "image";
        if (original) {
            image_folder = "image-ori";
        }
        File data = context.getFilesDir();
        File folder = new File(data, image_folder);
        if (!folder.exists()) {
            folder.mkdir();
        }
        return folder;
    }

    public static File getTestResultPath(Context context, boolean ffr) {
        File data = context.getFilesDir();
        File result = new File(data, "result");
        if (!result.exists()) {
            result.mkdir();
        }

        File folder;
        if (ffr) {
            folder = new File(result, "frr");
        } else {
            folder = new File(result, "far");
        }
        if (!folder.exists()) {
            folder.mkdir();
        }

        return folder;
    }

    public static String getEnrollResultFileName() {
        return "img_info.xml";
    }

    public static String getFingerPath(int hand, int fingers, int finger) {
        String path = "";
        int i;
        int count = 0;

        int index = hand;
        if (index < 0 || index >= HAND_STR.length) {
            index = 0;
        }
        path = HAND_STR[index];

        index = finger;
        if (index < 0 || index >= FINGER_STR.length) {
            index = 0;
        }
        if (DATA_FILE_NAME_INDICATE_FINGER) {
            for (i = 0; i < FINGER_STR.length; i++) {
                if ((fingers & (1 << i)) != 0) {
                    if (count == finger) {
                        break;
                    }
                    count++;
                }
            }

            if (i >= FINGER_STR.length) {
                for (i = 0; i < FINGER_STR.length; i++) {
                    if ((fingers & (1 << i)) != 0) {
                        break;
                    }
                }
            }
            path += FINGER_STR[i];
        } else {
            path += FINGER_STR[index];
        }

        return path;
    }

    public static String getDataFileName(int index, int subIndex) {
        StringBuffer name = new StringBuffer();
        String num = String.valueOf(index);
        for (int i = 0; i < MIN_DATA_FILE_NAME_LEN - num.length(); i++) {
            name.append("0");
        }
        name.append(num);
        if (subIndex > 0) {
            name.append("-");
            name.append(String.valueOf(subIndex));
        }
        name.append(FRR_FAR_DATA_FILE_SUFFIX);
        return name.toString();
    }

    public static boolean isLastFinger(int fingers, int finger) {
        if (fingers == 0) {
            fingers = FP_SETTINGS_ENROLL_FINGER_DEFAULT;
        }

        int count = numberInBinary(fingers);
        return (finger >= count - 1);
    }

    public static int getNextFinger(int finger, boolean lastfinger) {
        int next = finger + 1;
        if (lastfinger) {
            next = 0;
        }
        return next;
    }

    public static int getNextHand(int hand, boolean lastfinger) {
        int next = hand;
        if (lastfinger) {
            next += 1;
        }
        if (next >= HAND_STR.length) {
            next = 0;
        }
        return next;
    }

    public static String getNextUser(String user, int hand, boolean lastfinger) {
        String nextUser = user;
        if (hand >= HAND_STR.length - 1 && lastfinger) {
            if (isNumeric(user)) {
                int value = Integer.valueOf(user) + 1;
                String num = String.valueOf(value);
                StringBuffer sb = new StringBuffer();
                for (int i = 0; i < MIN_USER_NAME_LEN - num.length(); i++) {
                    sb.append("0");
                }
                sb.append(num);
                nextUser = sb.toString();
            }
        }
        return nextUser;
    }

    public static boolean isNumeric(String str) {
        Pattern pattern = Pattern.compile("[0-9]*");
        Matcher isNum = pattern.matcher(str);
        if (!isNum.matches()) {
            return false;
        }
        return true;
    }

    public static int numberInBinary(int n) {
        int count = 0;
        int value = n;
        while (value != 0) {
            count++;
            value = value & (value - 1);
        }
        return count;
    }

    public static void sortFiles(File[] files) {
        List<File> fileList = Arrays.asList(files);
        Collections.sort(fileList, new Comparator<File>() {
            @Override
            public int compare(File o1, File o2) {
                if (o1.isDirectory() && o2.isFile())
                    return -1;
                if (o1.isFile() && o2.isDirectory())
                    return 1;
                return o1.getName().compareTo(o2.getName());
            }
        });
    }

    public static int getValueIndex(String value, CharSequence[] values, int def) {
        int index = def;
        if (values != null && value != null && !value.isEmpty()) {
            for (int i = 0; i < values.length; i++) {
                if (value.equals(values[i])) {
                    index = i;
                    break;
                }
            }
        }
        return index;
    }

    public static int getDrawableId(Context context, String pkgName, String name, String def) {
        int drawableId = 0;

        if (name != null && !name.isEmpty()) {
            drawableId = context.getResources().getIdentifier(name, "drawable", pkgName);
        }

        if (drawableId == 0) {
            drawableId = context.getResources().getIdentifier(def, "drawable", pkgName);
        }

        return drawableId;
    }

    public static int getDrawableSize(Context context, int id, FingerImageSize size) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        BitmapFactory.decodeResource(context.getResources(), id, options);
        size.width = options.outWidth;
        size.height = options.outHeight;
        return 0;
    }
}