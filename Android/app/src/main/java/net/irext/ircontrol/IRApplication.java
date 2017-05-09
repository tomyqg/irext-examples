package net.irext.ircontrol;

import android.util.Log;

import com.activeandroid.ActiveAndroid;

import net.irext.webapi.WebAPIs;
import net.irext.webapi.model.Admin;

/**
 * Filename:       IRApplication.java
 * Revised:        Date: 2017-03-28
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Main Application class for irext decode example
 * <p>
 * Revision log:
 * 2017-03-28: created by strawmanbobi
 */
public class IRApplication extends com.activeandroid.app.Application {

    private static final String TAG = IRApplication.class.getSimpleName();

    private static final String ADDRESS = "http://192.168.1.100:8080";
    private static final String APP_NAME = "/irext";

    public WebAPIs mWeAPIs = WebAPIs.getInstance(ADDRESS, APP_NAME);

    private Admin mAdmin;

    public Admin getAdmin() {
        return mAdmin;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // initialize ActiveAndroid
        ActiveAndroid.initialize(this);

        // login with guest-admin account
        new Thread() {
            @Override
            public void run() {
                mAdmin = mWeAPIs.signIn("guest@irext.net", "irextguest");
                Log.d(TAG, "signIn response : " + mAdmin.getId() + ", " + mAdmin.getToken());
            }
        }.start();
    }
}
