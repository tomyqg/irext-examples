package net.irext.ircontrol.ui.fragment;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ListView;

import net.irext.ircontrol.IRApplication;
import net.irext.ircontrol.bean.RemoteControl;
import net.irext.ircontrol.ui.widget.PullToRefreshListView;
import net.irext.ircontrol.R;
import net.irext.ircontrol.ui.adapter.IndexAdapter;
import net.irext.ircontrol.utils.FileUtils;
import net.irext.ircontrol.utils.MessageUtil;
import net.irext.webapi.model.Brand;
import net.irext.webapi.model.Category;
import net.irext.webapi.model.City;
import net.irext.webapi.model.RemoteIndex;
import net.irext.webapi.model.StbOperator;

import java.io.File;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * Filename:       IndexFragment.java
 * Revised:        Date: 2017-04-12
 * Revision:       Revision: 1.0
 * <p>
 * Description:    RemoteIndex Fragment
 * <p>
 * Revision log:
 * 2017-04-12: created by strawmanbobi
 */
public class IndexFragment extends BaseCreateFragment {

    private static final String TAG = IndexFragment.class.getSimpleName();

    private static final int CMD_REFRESH_INDEX_LIST = 0;
    private static final int CMD_DOWNLOAD_BIN_FILE = 1;
    private static final int CMD_SAVE_REMOTE_CONTROL = 2;

    private PullToRefreshListView mIndexList;

    private List<RemoteIndex> mIndexes;
    private RemoteIndex mCurrentIndex;

    private int mBrandId = 0;
    private String mCityCode = "";
    private String mOperatorId = "";

    private String mCategoryName = "";
    private String mCityName = "";
    private String mBrandName = "";
    private String mOperatorName = "";

    private IndexAdapter mIndexAdapter;

    private MsgHandler mMsgHandler;
    private IRApplication mApp;

    public IndexFragment() {

    }

    private void listIndexes() {
        new Thread() {
            @Override
            public void run() {
                mIndexes = mApp.mWeAPIs.listRemoteIndexes(mParent.getCurrentCategory().getId(),
                        mBrandId, mCityCode, mOperatorId);
                if (null == mIndexes) {
                    mIndexes = new ArrayList<>();
                }
                MessageUtil.postMessage(mMsgHandler, CMD_REFRESH_INDEX_LIST);
            }
        }.start();
    }

    private void downloadBinFile() {
        new Thread() {
            @Override
            public void run() {
                try {
                    String remoteMap = mCurrentIndex.getRemoteMap();
                    int indexId = mCurrentIndex.getId();
                    InputStream in = mApp.mWeAPIs.downloadBin(remoteMap, indexId);
                    if (createDirectory()) {
                        File binFile = new File(FileUtils.BIN_PATH +
                                FileUtils.FILE_NAME_PREFIX + mCurrentIndex.getRemoteMap() +
                                FileUtils.FILE_NAME_EXT);
                        FileUtils.write(binFile, in);
                    } else {
                        Log.w(TAG, "no directory to contain bin file");
                    }

                    if (null != in) {
                        MessageUtil.postMessage(mMsgHandler, CMD_SAVE_REMOTE_CONTROL);
                    } else {
                        Log.e(TAG, "bin file download failed");
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }.start();
    }

    private boolean createDirectory() {
        File file = new File(FileUtils.BIN_PATH);
        if (file.exists()) {
            return true;
        }
        return file.mkdirs();
    }

    private void saveRemoteControl() {
        // TODO： update brand and operator name i18n
        RemoteControl remoteControl = new RemoteControl();
        remoteControl.setCategoryId(mCurrentIndex.getCategoryId());
        remoteControl.setCategoryName(mCategoryName);
        remoteControl.setBrandId(mCurrentIndex.getBrandId());
        remoteControl.setBrandName(mBrandName);
        remoteControl.setCityCode(mCurrentIndex.getCityCode());
        remoteControl.setCityName(mCityName);
        remoteControl.setOperatorId(mCurrentIndex.getOperatorId());
        remoteControl.setOperatorName(mOperatorName);
        remoteControl.setProtocol(mCurrentIndex.getProtocol());
        remoteControl.setRemote(mCurrentIndex.getRemote());
        remoteControl.setRemoteMap(mCurrentIndex.getRemoteMap());
        remoteControl.setSubCategory(mCurrentIndex.getSubCate());

        long id = RemoteControl.createRemoteControl(remoteControl);
        mParent.finish();
    }

    private void refreshIndexes() {
        mIndexAdapter = new IndexAdapter(mParent, mIndexes, mBrandName, mOperatorName);
        mIndexList.setAdapter(mIndexAdapter);
        mIndexList.onRefreshComplete();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        getFrom();
        View view = inflater.inflate(R.layout.fragment_index, container, false);

        mMsgHandler = new MsgHandler(this);
        mApp = (IRApplication) this.getActivity().getApplication();

        Category category = mParent.getCurrentCategory();
        Brand brand = mParent.getCurrentBrand();
        City city = mParent.getCurrentCity();
        StbOperator operator = mParent.getCurrentOperator();

        mCategoryName = category.getName();
        if (null != city) {
            mCityName = city.getName();
            mCityCode = city.getCode();
        }
        if (null != operator) {
            mOperatorName = operator.getOperatorName();
            mOperatorId = operator.getOperatorId();
        }
        if (null != brand) {
            mBrandName = brand.getName();
            mBrandId = brand.getId();
        }

        mIndexList = (PullToRefreshListView) view.findViewById(R.id.lv_index_list);
        mIndexList.setOnRefreshListener(new PullToRefreshListView.OnRefreshListener() {
            @Override
            public void onRefresh() {
                listIndexes();
            }
        });

        mIndexList.setOnItemClickListener(new ListView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                mCurrentIndex = (RemoteIndex) mIndexAdapter.getItem(position);
                MessageUtil.postMessage(mMsgHandler, CMD_DOWNLOAD_BIN_FILE);
            }
        });

        mIndexList.setRefreshing();

        return view;
    }

    @Override
    public boolean onBackPressed() {
        return super.onBackPressed();
    }

    private static class MsgHandler extends Handler {

        WeakReference<IndexFragment> mIndexFragment;

        MsgHandler(IndexFragment fragment) {
            mIndexFragment = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(Message msg) {
            int cmd = msg.getData().getInt(MessageUtil.KEY_CMD);

            IndexFragment indexFragment = mIndexFragment.get();
            switch (cmd) {

                case CMD_REFRESH_INDEX_LIST:
                    indexFragment.refreshIndexes();
                    break;

                case CMD_DOWNLOAD_BIN_FILE:
                    indexFragment.downloadBinFile();
                    break;

                case CMD_SAVE_REMOTE_CONTROL:
                    indexFragment.saveRemoteControl();

                default:
                    break;
            }
        }
    }
}
