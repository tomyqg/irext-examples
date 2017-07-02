package net.irext.ircontrol.ui.fragment;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;

import net.irext.ircontrol.IRApplication;
import net.irext.ircontrol.R;
import net.irext.ircontrol.ui.activity.CreateActivity;
import net.irext.ircontrol.ui.adapter.BrandAdapter;
import net.irext.ircontrol.ui.widget.PullToRefreshListView;
import net.irext.ircontrol.utils.MessageUtil;
import net.irext.webapi.WebAPICallbacks.ListBrandsCallback;
import net.irext.webapi.model.Brand;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * Filename:       BrandFragment.java
 * Revised:        Date: 2017-04-07
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Brand Fragment
 * <p>
 * Revision log:
 * 2017-04-07: created by strawmanbobi
 */
public class BrandFragment extends BaseCreateFragment {

    private static final String TAG = BrandFragment.class.getSimpleName();

    private static final int CMD_REFRESH_BRAND_LIST = 0;

    private PullToRefreshListView mBrandList;

    private List<Brand> mBrands;

    private BrandAdapter mBrandAdapter;

    private MsgHandler mMsgHandler;
    private IRApplication mApp;

    private ListBrandsCallback mListBrandsCallback = new ListBrandsCallback() {
        @Override
        public void onListBrandsSuccess(List<Brand> brands) {
            mBrands = brands;
            if (null == mBrands) {
                mBrands = new ArrayList<>();
            }
            MessageUtil.postMessage(mMsgHandler, CMD_REFRESH_BRAND_LIST);
        }

        @Override
        public void onListBrandsFailed() {
            Log.w(TAG, "list brands failed");
        }

        @Override
        public void onListBrandsError() {
            Log.e(TAG, "list brands error");
        }
    };

    public BrandFragment() {
    }

    private void listBrands() {
        new Thread() {
            @Override
            public void run() {
                mApp.mWeAPIs
                        .listBrands(mParent.getCurrentCategory().getId(), 0, 20,
                                mListBrandsCallback);
            }
        }.start();
    }

    private void refreshBrands() {
        mBrandAdapter = new BrandAdapter(mParent, mBrands);
        mBrandList.setAdapter(mBrandAdapter);
        mBrandList.onRefreshComplete();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        getFrom();
        View view = inflater.inflate(R.layout.fragment_brand, container, false);
        mApp = (IRApplication) getActivity().getApplication();

        mMsgHandler = new MsgHandler(this);

        mBrandList = (PullToRefreshListView) view.findViewById(R.id.lv_brand_list);

        mBrandList.setOnRefreshListener(new PullToRefreshListView.OnRefreshListener() {
            @Override
            public void onRefresh() {
                listBrands();
            }
        });

        mBrandList.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Brand brand = (Brand)mBrandAdapter.getItem(position);
                mParent.setCurrentBrand(brand);
                MessageUtil.postMessage(mParent.mMsgHandler,
                        CreateActivity.PAGE_INDEX,
                        CreateActivity.PAGE_BRAND);
            }
        });
        mBrandList.setRefreshing();

        return view;
    }

    @Override
    public boolean onBackPressed() {
        super.onBackPressed();
        return true;
    }

    private static class MsgHandler extends Handler {

        WeakReference<BrandFragment> mBrandFragment;

        MsgHandler(BrandFragment fragment) {
            mBrandFragment = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(Message msg) {
            int cmd = msg.getData().getInt(MessageUtil.KEY_CMD);
            Log.d(TAG, "handle message " + Integer.toString(cmd));

            BrandFragment brandFragment = mBrandFragment.get();
            switch (cmd) {

                case CMD_REFRESH_BRAND_LIST:
                    brandFragment.refreshBrands();
                    break;

                default:
                    break;
            }
        }
    }
}
