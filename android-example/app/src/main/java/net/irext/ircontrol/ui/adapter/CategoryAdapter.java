package net.irext.ircontrol.ui.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;
import net.irext.ircontrol.R;
import net.irext.webapi.model.Category;

import java.util.List;


/**
 * Filename:       CategoryAdapter.java
 * Revised:        Date: 2017-04-07
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Adapter class of Category
 * <p>
 * Revision log:
 * 2017-04-07: created by strawmanbobi
 */
public class CategoryAdapter extends BaseAdapter {

    private List<Category> mCategories;
    private LayoutInflater mInflater;

    public CategoryAdapter(Context ctx, List<Category> list) {
        this.mCategories = list;
        this.mInflater = (LayoutInflater)ctx.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getCount() {
        return mCategories.size();
    }

    @Override
    public Object getItem(int position) {
        return mCategories.get(position);
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
            convertView = mInflater.inflate(R.layout.item_category, parent, false);
            holder.textView = (TextView)convertView.findViewById(R.id.tv_category_name);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder)convertView.getTag();
        }
        holder.textView.setText(mCategories.get(position).getName());
        return convertView;
    }

    private static class ViewHolder {
        TextView textView;
    }
}
