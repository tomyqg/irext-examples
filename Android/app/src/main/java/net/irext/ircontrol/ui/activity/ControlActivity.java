package net.irext.ircontrol.ui.activity;

import android.os.Bundle;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;
import android.view.MenuItem;
import net.irext.ircontrol.R;
import net.irext.ircontrol.ui.fragment.ControlFragment;

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

    @SuppressWarnings("unused")
    private static final String TAG = ControlActivity.class.getSimpleName();

    public static final String KEY_REMOTE_ID = "KEY_REMOTE_ID";

    private ControlFragment mFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_control);

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
}
