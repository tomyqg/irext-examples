package net.irext.ircontrol;

import android.util.Log;

import com.activeandroid.ActiveAndroid;

import net.irext.webapi.WebAPIs;
import net.irext.webapi.model.UserApp;

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

    // private static final String ADDRESS = "http://irext.net:8080";
    private static final String ADDRESS = "http://192.168.137.128:8080";
    private static final String APP_NAME = "/irext-server";

    public WebAPIs mWeAPIs = WebAPIs.getInstance(ADDRESS, APP_NAME);

    private UserApp mUserApp;

    public UserApp getAdmin() {
        return mUserApp;
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
                mUserApp = mWeAPIs.signIn(IRApplication.this);
                if (null != mUserApp) {
                    Log.d(TAG, "signIn response : " + mUserApp.getId() + ", " + mUserApp.getToken());
                }
            }
        }.start();
    }
}
