package net.irext.ircontrol.ui.activity;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.app.FragmentManager;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import net.irext.ircontrol.R;
import net.irext.ircontrol.bean.RemoteControl;
import net.irext.ircontrol.ui.fragment.MainFragment;
import net.irext.ircontrol.utils.MessageUtil;

import java.lang.ref.WeakReference;

/**
 * Filename:       MainActivity.java
 * Revised:        Date: 2017-04-04
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Main Activity class for irext decode example
 * <p>
 * Revision log:
 * 2017-04-04: created by strawmanbobi
 */
public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();

    public static final int CMD_GOTO_CONTROL = 0;

    private FragmentManager mFragmentManager;
    private MainFragment mRemoteListFragment;

    private RemoteControl mCurrentRemoteControl;

    public MsgHandler mMsgHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initView();
    }

    @Override
    protected void onResume() {
        super.onResume();

        mFragmentManager = this.getSupportFragmentManager();
        mRemoteListFragment = (MainFragment) mFragmentManager.findFragmentById(R.id.fragment_remote);

        if (null == mRemoteListFragment) {
            Log.e(TAG, "MainFragment is null");
        }
        mRemoteListFragment.onResume();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                Log.d(TAG, "BUTTON PRESSED");
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public RemoteControl getmCurrentRemoteControl() {
        return mCurrentRemoteControl;
    }

    public void setmCurrentRemoteControl(RemoteControl mCurrentRemoteControl) {
        this.mCurrentRemoteControl = mCurrentRemoteControl;
    }

    private void initView() {
        setContentView(R.layout.activity_main);

        mMsgHandler = new MsgHandler(this);
        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                gotoCreateNew();
            }
        });
    }

    private void gotoCreateNew() {
        Intent intent = new Intent(this, CreateActivity.class);
        startActivity(intent);
    }

    private void gotoControl() {
        Intent intent = new Intent(this, ControlActivity.class);
        Bundle bundle = new Bundle();
        bundle.putLong(ControlActivity.KEY_REMOTE_ID, mCurrentRemoteControl.getID());
        intent.putExtras(bundle);
        startActivity(intent);
    }

    private  static class MsgHandler extends Handler {

        WeakReference<MainActivity> mMainActivity;

        MsgHandler(MainActivity activity) {
            mMainActivity = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            int cmd = msg.getData().getInt(MessageUtil.KEY_CMD);
            Log.d(TAG, "handle message " + Integer.toString(cmd));

            MainActivity mainActivity = mMainActivity.get();
            switch (cmd) {
                case CMD_GOTO_CONTROL:
                    mainActivity.gotoControl();
                    break;

                default:
                    break;
            }
        }
    }
}
