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
import net.irext.ircontrol.ui.fragment.*;
import net.irext.ircontrol.utils.MessageUtil;
import net.irext.webapi.model.Brand;
import net.irext.webapi.model.Category;
import net.irext.webapi.model.City;
import net.irext.webapi.model.RemoteIndex;
import net.irext.webapi.model.StbOperator;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

public class CreateActivity extends AppCompatActivity {

    private static final String TAG = CreateActivity.class.getSimpleName();

    public static final String KEY_FROM = "FROM";

    public static final int PAGE_CATEGORY = 0;
    public static final int PAGE_BRAND = 1;
    public static final int PAGE_CITY = 2;
    public static final int PAGE_INDEX = 3;

    public static final int FROM_BRAND = 0;
    public static final int FROM_CITY = 1;

    private BaseCreateFragment mFragment;
    private List<BaseCreateFragment> mFragments;
    private FragmentManager mFragmentManager;

    public MsgHandler mMsgHandler;

    private Category mCurrentCategory;
    private Brand mCurrentBrand;
    private City mCurrentCity;
    private StbOperator mCurrentOperator;
    private RemoteIndex mCurrentRemoteIndex;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_create);

        mMsgHandler = new MsgHandler(this);

        mFragments = new ArrayList<>();
        mFragments.add(new CategoryFragment());
        mFragments.add(new BrandFragment());
        mFragments.add(new CityFragment());
        mFragments.add(new IndexFragment());

        mFragmentManager = getSupportFragmentManager();

        switchPage(PAGE_CATEGORY, -1);
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
    public void onBackPressed() {
        if (null == mFragment) {
            this.finish();
            super.onBackPressed();
            return;
        }
        if(!mFragment.onBackPressed()) {
            super.onBackPressed();
        }
    }

    private void switchPage(int next, Integer from) {
        BaseCreateFragment fragment = mFragments.get(next);
        Bundle bundle = new Bundle();
        if (null == from) {
            from = -1;
        }
        bundle.putInt(KEY_FROM, from);
        fragment.setArguments(bundle);
        FragmentTransaction transaction = mFragmentManager.beginTransaction();
        transaction.replace(R.id.rl_fragment_window, fragment);
        transaction.commit();
        mFragment = fragment;
    }

    public Category getCurrentCategory() {
        return mCurrentCategory;
    }

    public void setCurrentCategory(Category currentCategory) {
        this.mCurrentCategory = currentCategory;
    }

    public Brand getCurrentBrand() {
        return mCurrentBrand;
    }

    public void setCurrentBrand(Brand currentBrand) {
        this.mCurrentBrand = currentBrand;
    }

    public City getCurrentCity() {
        return mCurrentCity;
    }

    public void setCurrentCity(City currentCity) {
        this.mCurrentCity = currentCity;
    }

    public StbOperator getCurrentOperator() {
        return mCurrentOperator;
    }

    public void setCurrentOperator(StbOperator currentOperator) {
        this.mCurrentOperator = currentOperator;
    }

    public RemoteIndex getCurrentRemoteIndex() {
        return mCurrentRemoteIndex;
    }

    public void setCurrentRemoteIndex(RemoteIndex currentRemoteIndex) {
        this.mCurrentRemoteIndex = currentRemoteIndex;
    }

    private static class MsgHandler extends Handler {

        WeakReference<CreateActivity> mCreateActivity;

        MsgHandler(CreateActivity activity) {
            mCreateActivity = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            int cmd = msg.getData().getInt(MessageUtil.KEY_CMD);
            Log.d(TAG, "handle message " + Integer.toString(cmd));

            CreateActivity createActivity = mCreateActivity.get();
            switch (cmd) {
                case PAGE_CATEGORY:
                case PAGE_BRAND:
                case PAGE_CITY:
                case PAGE_INDEX:
                    Integer from = (Integer)msg.obj;
                    createActivity.switchPage(cmd, from);
                    break;

                default:
                    break;
            }
        }
    }
}
