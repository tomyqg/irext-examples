package net.irext.ircontrol.ui.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;
import net.irext.ircontrol.R;
import net.irext.decodesdk.utils.Constants;
import net.irext.webapi.model.RemoteIndex;

import java.util.List;

/**
 * Filename:       IndexAdapter.java
 * Revised:        Date: 2017-04-07
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Adapter class of RemoteIndex
 * <p>
 * Revision log:
 * 2017-04-07: created by strawmanbobi
 */
public class IndexAdapter extends BaseAdapter {

    private List<RemoteIndex> mIndexes;
    private LayoutInflater mInflater;
    private String mBrandName;
    private String mOperatorName;

    public IndexAdapter(Context ctx, List<RemoteIndex> list, String brandName, String operatorName) {
        this.mIndexes = list;
        this.mInflater = (LayoutInflater)ctx.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        this.mBrandName = brandName;
        this.mOperatorName = operatorName;
    }

    @Override
    public int getCount() {
        return mIndexes.size();
    }

    @Override
    public Object getItem(int position) {
        return mIndexes.get(position);
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
            convertView = mInflater.inflate(R.layout.item_index, parent, false);
            holder.textName = (TextView)convertView.findViewById(R.id.tv_index_name);
            holder.textMap = (TextView)convertView.findViewById(R.id.tv_index_map);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder)convertView.getTag();
        }
        RemoteIndex index = mIndexes.get(position);
        if (index.getCategoryId() != Constants.CategoryID.STB.getValue()) {
            holder.textName.setText(mBrandName + " " + (position + 1));
        } else {
            holder.textName.setText(mOperatorName + " " + (position + 1));
        }
        holder.textMap.setText(index.getRemoteMap());
        return convertView;
    }

    private static class ViewHolder {
        TextView textName;
        TextView textMap;
    }
}
