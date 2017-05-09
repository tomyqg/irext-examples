package net.irext.ircontrol.ui.activity;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MenuItem;
import net.irext.ircontrol.R;
import net.irext.ircontrol.ui.fragment.ControlFragment;
import net.irext.ircontrol.utils.MessageUtil;

import java.lang.ref.WeakReference;

/**
 * Filename:       ControlActivity.java
 * Revised:        Date: 2017-04-22
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Control activity containing control fragment
 * <p>
 * Revision log:
 * 2017-04-22: created by strawmanbobi
 */
public class ControlActivity extends AppCompatActivity {

    private static final String TAG = ControlActivity.class.getSimpleName();

    public static final String KEY_REMOTE_ID = "KEY_REMOTE_ID";

    private MsgHandler mMsgHandler;

    private ControlFragment mFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_control);

        mMsgHandler = new MsgHandler(this);

        FragmentManager fragmentManager = getSupportFragmentManager();

        mFragment = new ControlFragment();
        Bundle bundle = getIntent().getExtras();
        mFragment.setArguments(bundle);
        FragmentTransaction transaction = fragmentManager.beginTransaction();
        transaction.replace(R.id.rl_main_layout, mFragment);
        transaction.commit();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                onBackPressed();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onStop() {
        if (null != mFragment) {
            mFragment.closeIRBinary();
        }
        super.onStop();
    }

    private static class MsgHandler extends Handler {

        WeakReference<ControlActivity> mControlActivity;

        MsgHandler(ControlActivity activity) {
            mControlActivity = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            int cmd = msg.getData().getInt(MessageUtil.KEY_CMD);
            Log.d(TAG, "handle message " + Integer.toString(cmd));

            ControlActivity controlActivity = mControlActivity.get();
            switch (cmd) {

                default:
                    break;
            }
        }
    }
}
