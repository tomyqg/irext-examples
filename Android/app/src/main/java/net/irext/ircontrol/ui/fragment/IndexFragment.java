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

    private IndexAdapter mIndexAdapter;

    private MsgHandler mMsgHandler;
    private IRApplication mApp;

    public IndexFragment() {
    }

    private void listIndexes() {
        new Thread() {
            @Override
            public void run() {
                Brand brand = mParent.getCurrentBrand();
                City city = mParent.getCurrentCity();
                StbOperator operator = mParent.getCurrentOperator();

                int brandID = 0;
                String cityCode = null;
                String operatorID = null;

                if (null != brand) {
                    brandID = brand.getId();
                }

                if (null != city) {
                    cityCode = city.getCode();
                }

                if (null != operator) {
                    operatorID = operator.getOperator_id();
                }

                mIndexes = mApp.mWeAPIs.listRemoteIndexes(mParent.getCurrentCategory().getId(),  brandID,
                                cityCode, operatorID);
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
                String remoteMap = mCurrentIndex.getRemoteMap();
                int indexID = mCurrentIndex.getId();
                InputStream in = mApp.mWeAPIs.downloadBin(remoteMap, indexID);
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
            }
        }.start();
    }

    private boolean createDirectory() {
        File file = new File(FileUtils.BIN_PATH);
        return file.mkdirs();
    }

    private void saveRemoteControl() {
        RemoteControl remoteControl = new RemoteControl();
        remoteControl.setCategoryID(mCurrentIndex.getCategoryId());
        remoteControl.setCategoryName(mCurrentIndex.getCategoryName());
        remoteControl.setBrandID(mCurrentIndex.getBrandId());
        remoteControl.setBrandName(mCurrentIndex.getBrandName());
        remoteControl.setCityCode(mCurrentIndex.getCityCode());
        remoteControl.setCityName(mCurrentIndex.getCityName());
        remoteControl.setOperatorID(mCurrentIndex.getOperatorId());
        remoteControl.setOperatorName(mCurrentIndex.getOperatorName());
        remoteControl.setProtocol(mCurrentIndex.getProtocol());
        remoteControl.setRemote(mCurrentIndex.getRemote());
        remoteControl.setRemoteMap(mCurrentIndex.getRemoteMap());
        remoteControl.setSubCategory(mCurrentIndex.getSubCate());

        long id = RemoteControl.createRemoteControl(remoteControl);
        Log.d(TAG, "remote control has been saved: " + id);
        mParent.finish();
    }

    private void refreshIndexes() {
        mIndexAdapter = new IndexAdapter(mParent, mIndexes);
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
            Log.d(TAG, "handle message " + Integer.toString(cmd));

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
