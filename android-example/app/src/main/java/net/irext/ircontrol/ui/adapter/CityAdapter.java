package net.irext.ircontrol.ui.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;
import net.irext.ircontrol.R;
import net.irext.webapi.model.City;

import java.util.List;

/**
 * Filename:       CityAdapter.java
 * Revised:        Date: 2017-04-07
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Adapter class of City
 * <p>
 * Revision log:
 * 2017-04-07: created by strawmanbobi
 */
public class CityAdapter extends BaseAdapter {

    private List<City> mCities;
    private LayoutInflater mInflater;

    public CityAdapter(Context ctx, List<City> list) {
        this.mCities = list;
        this.mInflater = (LayoutInflater)ctx.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public CityAdapter(Context ctx) {
        this.mInflater = (LayoutInflater)ctx.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public void setCities(List<City> list) {
        this.mCities = list;
    }

    @Override
    public int getCount() {
        return mCities.size();
    }

    @Override
    public Object getItem(int position) {
        return mCities.get(position);
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
            convertView = mInflater.inflate(R.layout.item_city, parent, false);
            holder.textView = (TextView)convertView.findViewById(R.id.tv_city_name);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder)convertView.getTag();
        }
        holder.textView.setText(mCities.get(position).getName());
        return convertView;
    }

    private static class ViewHolder {
        TextView textView;
    }
}
