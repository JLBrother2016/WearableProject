package com.gujun.wearabledevice;

import android.content.Context;
import android.util.Log;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by gyguj on 2018/1/16.
 */

public class Utils {
    private static String TAG = "WearableDevice";

    public static String getStorageDirectory(Context context) {
        return context.getExternalFilesDir(null).getAbsolutePath();
    }

    public static String getDateTimeString() {
        SimpleDateFormat date = new SimpleDateFormat("YYYY-MM-dd-HH-mm-ss");
        Date curDate = new Date(System.currentTimeMillis());
        return date.format(curDate);
    }

    public static void debug(String msg) {
        Log.d(TAG, msg);
    }
}
