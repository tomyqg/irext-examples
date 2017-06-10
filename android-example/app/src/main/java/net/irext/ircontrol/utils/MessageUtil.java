package net.irext.ircontrol.utils;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

/**
 * Filename:       MainFragment.java
 * Revised:        Date: 2017-04-08
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Post message util
 * <p>
 * Revision log:
 * 2017-04-08: created by strawmanbobi
 */
public class MessageUtil {

    public static final String KEY_CMD = "CMD";

    public static void postMessage(Handler handler, int message, Object parameter) {
        Message msg = handler.obtainMessage();
        Bundle b = new Bundle();
        b.putInt(KEY_CMD, message);
        msg.setData(b);
        msg.obj = parameter;
        handler.sendMessage(msg);
    }

    public static void postMessage(Handler handler, int message) {
        Message msg = handler.obtainMessage();
        Bundle b = new Bundle();
        b.putInt(KEY_CMD, message);
        msg.setData(b);
        msg.obj = null;
        handler.sendMessage(msg);
    }
}
