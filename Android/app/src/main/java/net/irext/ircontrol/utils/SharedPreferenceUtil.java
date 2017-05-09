package net.irext.ircontrol.utils;


import android.content.Context;
import android.content.SharedPreferences;

import java.util.HashMap;

/**
 * Filename:       SharedPreferenceUtil.java
 * Revised:        Date: 2017-04-05
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Wrapper of shared preference
 * <p>
 * Revision log:
 * 2017-04-05: created by strawmanbobi
 */
public class SharedPreferenceUtil {

    private static final String TAG = SharedPreferenceUtil.class.getSimpleName();

    private static final String _NAME = "irext";
    private static SharedPreferences mSharedPreferences;
    private static SharedPreferences.Editor mEditor;
    private static SharedPreferenceUtil mSharedPreferenceUtils = null;
    private Context mContext;

    public SharedPreferenceUtil(Context context) {
        this.mContext = context;
    }

    public static SharedPreferenceUtil getInstance(Context context) {
        if (null == mSharedPreferenceUtils) {
            mSharedPreferenceUtils = new SharedPreferenceUtil(context);
            mSharedPreferences = context.getSharedPreferences(_NAME, Context.MODE_PRIVATE);
            mEditor = mSharedPreferences.edit();
        }
        return mSharedPreferenceUtils;
    }

    // shared preference operation
    public boolean restore(String key, Object value) {
        if (value instanceof Boolean) {
            mEditor.putBoolean(key, (Boolean) value);
        } else if (value instanceof Integer) {
            mEditor.putInt(key, (Integer) value);
        } else if (value instanceof Float) {
            mEditor.putFloat(key, (Float) value);
        } else if (value instanceof String) {
            mEditor.putString(key, (String) value);
        } else if (value instanceof Long) {
            mEditor.putLong(key, (Long) value);
        }

        return mEditor.commit();
    }

    public String getString(String key) {
        return mSharedPreferences.getString(key, null);
    }

    public String getString(String key, String defaultValue) {
        return mSharedPreferences.getString(key, defaultValue);
    }

    public boolean getBoolean(String key, boolean defaultFunc) {
        if (null != mSharedPreferences) {
            return mSharedPreferences.getBoolean(key, defaultFunc);
        } else {
            return false;
        }
    }

    public int getInt(String key, int defaultValue) {
        return mSharedPreferences.getInt(key, defaultValue);
    }

    public int getInt(String key) {
        return mSharedPreferences.getInt(key, -1);
    }

    public long getLong(String key) {
        return mSharedPreferences.getLong(key, -1);
    }

    public float getDouble(String key) {
        return mSharedPreferences.getFloat(key, -1);
    }

    public boolean saveKeyValueMap(HashMap<String, String> kvMap) {
        for (String s : kvMap.keySet()) {
            mEditor.putString(s, kvMap.get(s));
        }
        mEditor.commit();
        return true;
    }
}
