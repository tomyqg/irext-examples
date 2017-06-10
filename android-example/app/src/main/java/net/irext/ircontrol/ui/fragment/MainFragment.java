package net.irext.ircontrol.ui.fragment;

import android.os.Handler;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;
import net.irext.ircontrol.R;
import net.irext.ircontrol.bean.RemoteControl;
import net.irext.ircontrol.ui.activity.MainActivity;
import net.irext.ircontrol.ui.adapter.RemoteControlAdapter;
import net.irext.ircontrol.ui.widget.PullToRefreshListView;
import net.irext.ircontrol.utils.MessageUtil;

import java.lang.ref.WeakReference;
import java.util.List;

/**
 * Filename:       MainFragment.java
 * Revised:        Date: 2017-04-04
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Main Fragment class for irext decode example
 * <p>
 * Revision log:
 * 2017-04-04: created by strawmanbobi
 */
public class MainFragment extends Fragment {

    private static final String TAG = MainFragment.class.getSimpleName();

    private static final int CMD_REFRESH_REMOTE_LIST = 0;

    private MsgHandler mHandler;

    private MainActivity mParent;

    private List<RemoteControl> mRemoteControls;
    private RemoteControlAdapter mRemoteControlAdapter;

    private TextView mTVCreateNote;
    private PullToRefreshListView mRemoteControlList;

    public MainFragment() {
    }

    public MsgHandler getHandler() {
        return mHandler;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        mHandler = new MsgHandler(this);

        mParent = (MainActivity)getActivity();
        View view = inflater.inflate(R.layout.fragment_main, container, false);
        mTVCreateNote = (TextView) view.findViewById(R.id.tv_create_note);
        mRemoteControlList = (PullToRefreshListView) view.findViewById(R.id.lv_remote_list);

        mRemoteControlList.setOnRefreshListener(new PullToRefreshListView.OnRefreshListener() {
            @Override
            public void onRefresh() {
                listRemotes();
            }
        });

        mRemoteControlList.setOnItemClickListener(new ListView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                RemoteControl remoteControl = (RemoteControl)mRemoteControlAdapter.getItem(position);
                mParent.setmCurrentRemoteControl(remoteControl);
                MessageUtil.postMessage(mParent.mMsgHandler, MainActivity.CMD_GOTO_CONTROL);
            }
        });
        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        mRemoteControlList.setRefreshing();
    }

    private void listRemotes() {
        new Thread() {
            @Override
            public void run() {
                mRemoteControls = RemoteControl.listRemoteControls(0, 20);
                MessageUtil.postMessage(mHandler, CMD_REFRESH_REMOTE_LIST);
            }
        }.start();
    }

    private void refreshRemoteList() {
        if (null != mRemoteControls && mRemoteControls.size() > 0) {
            Log.d(TAG, "remote control fetched : " + mRemoteControls.size());
            mTVCreateNote.setVisibility(View.GONE);
            mRemoteControlList.setVisibility(View.VISIBLE);
            mRemoteControlAdapter = new RemoteControlAdapter(mParent, mRemoteControls);
            mRemoteControlAdapter.setRemoteControls(mRemoteControls);
            mRemoteControlList.setAdapter(mRemoteControlAdapter);
            mRemoteControlList.onRefreshComplete();
        } else {
            mTVCreateNote.setVisibility(View.VISIBLE);
            mRemoteControlList.setVisibility(View.GONE);
        }
    }

    private static class MsgHandler extends Handler {

        WeakReference<MainFragment> mMainFragment;

        MsgHandler(MainFragment fragment) {
            mMainFragment = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(Message msg) {
            int cmd = msg.getData().getInt(MessageUtil.KEY_CMD);
            Log.d(TAG, "handle message " + Integer.toString(cmd));

            MainFragment mainFragment = mMainFragment.get();
            switch (cmd) {

                case CMD_REFRESH_REMOTE_LIST:
                    mainFragment.refreshRemoteList();
                    break;

                default:
                    break;
            }
        }
    }
}
