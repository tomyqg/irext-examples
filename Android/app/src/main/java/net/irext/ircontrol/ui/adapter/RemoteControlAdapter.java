package net.irext.ircontrol.ui.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;
import net.irext.ircontrol.R;
import net.irext.ircontrol.bean.RemoteControl;
import net.irext.decodesdk.utils.Constants;

import java.util.List;

/**
 * Filename:       RemoteControlAdapter.java
 * Revised:        Date: 2017-04-15
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Adapter class of Remote
 * <p>
 * Revision log:
 * 2017-04-15: created by strawmanbobi
 */
public class RemoteControlAdapter extends BaseAdapter {

    private List<RemoteControl> mRemoteControls;
    private LayoutInflater mInflater;

    public RemoteControlAdapter(Context ctx, List<RemoteControl> list) {
        this.mRemoteControls = list;
        this.mInflater = (LayoutInflater)ctx.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public RemoteControlAdapter(Context ctx) {
        this.mInflater = (LayoutInflater)ctx.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public void setRemoteControls(List<RemoteControl> remoteControls) {
        this.mRemoteControls = remoteControls;
    }

    @Override
    public int getCount() {
        return mRemoteControls.size();
    }

    @Override
    public Object getItem(int position) {
        return mRemoteControls.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        ViewHolder holder;
        if (convertView == null) {
            holder = new ViewHolder();
            convertView = mInflater.inflate(R.layout.item_remote, parent, false);
            holder.textView = (TextView)convertView.findViewById(R.id.tv_remote_name);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder)convertView.getTag();
        }
        RemoteControl remoteControl = mRemoteControls.get(position);
        String remoteName;
        if (Constants.CategoryID.STB.getValue() != remoteControl.getCategoryID()) {
            remoteName = remoteControl.getCategoryName() + "-" + remoteControl.getBrandName();
        } else {
            remoteName = remoteControl.getCityName() + "-" + remoteControl.getOperatorName();
        }
        holder.textView.setText(remoteName);
        return convertView;
    }

    private static class ViewHolder {
        TextView textView;
    }
}
