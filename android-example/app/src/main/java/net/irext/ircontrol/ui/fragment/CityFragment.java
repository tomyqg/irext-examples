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
import net.irext.ircontrol.R;
import net.irext.ircontrol.ui.activity.CreateActivity;
import net.irext.ircontrol.ui.adapter.CityAdapter;
import net.irext.ircontrol.ui.adapter.OperatorAdapter;
import net.irext.ircontrol.utils.MessageUtil;

import net.irext.webapi.WebAPICallbacks.ListProvincesCallback;
import net.irext.webapi.WebAPICallbacks.ListCitiesCallback;
import net.irext.webapi.WebAPICallbacks.ListOperatersCallback;

import net.irext.webapi.model.City;
import net.irext.webapi.model.StbOperator;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * Filename:       CityFragment.java
 * Revised:        Date: 2017-04-07
 * Revision:       Revision: 1.0
 * <p>
 * Description:    City Fragment
 * <p>
 * Revision log:
 * 2017-04-07: created by strawmanbobi
 */
public class CityFragment extends BaseCreateFragment {

    private static final String TAG = CityFragment.class.getSimpleName();

    private static final int CMD_REFRESH_PROVINCE_LIST = 0;
    private static final int CMD_REFRESH_CITY_LIST = 1;
    private static final int CMD_REFRESH_OPERATOR_LIST = 2;

    private static final int LEVEL_PROVINCE = 0;
    private static final int LEVEL_CITY = 1;
    private static final int LEVEL_OPERATOR = 2;

    private ListView mCityList;

    private List<City> mProvinces;
    private List<City> mCities;
    private List<StbOperator> mOperators;

    private CityAdapter mCityAdapter;
    private OperatorAdapter mOperatorAdapter;

    private MsgHandler mMsgHandler;
    private IRApplication mApp;

    private ListProvincesCallback mListProvincesCallback = new ListProvincesCallback() {
        @Override
        public void onListProvincesSuccess(List<City> provinces) {
            mProvinces = provinces;
            if (null == mProvinces) {
                mProvinces = new ArrayList<>();
            }
            MessageUtil.postMessage(mMsgHandler, CMD_REFRESH_PROVINCE_LIST);
        }

        @Override
        public void onListProvincesFailed() {
            Log.w(TAG, "list provinces failed");
        }

        @Override
        public void onListProvincesError() {
            Log.e(TAG, "list provinces error");
        }
    };

    private ListCitiesCallback mListCitiesCallback = new ListCitiesCallback() {
        @Override
        public void onListCitiesSuccess(List<City> cities) {
            mCities = cities;
            if (null == mCities) {
                mCities = new ArrayList<>();
            }
            MessageUtil.postMessage(mMsgHandler, CMD_REFRESH_CITY_LIST);
        }

        @Override
        public void onListCitiesFailed() {
            Log.w(TAG, "list cities failed");
        }

        @Override
        public void onListCitiesError() {
            Log.w(TAG, "list cities error");
        }
    };

    private ListOperatersCallback mListOperatorCallback = new ListOperatersCallback() {

        @Override
        public void onListOperatorsSuccess(List<StbOperator> operators) {
            mOperators = operators;
            if (null == mOperators) {
                mOperators = new ArrayList<>();
            }
            MessageUtil.postMessage(mMsgHandler, CMD_REFRESH_OPERATOR_LIST);
        }

        @Override
        public void onListOperatorsFailed() {
            Log.w(TAG, "list operators failed");
        }

        @Override
        public void onListOperatorsError() {
            Log.e(TAG, "list operators error");
        }
    };

    private City mCurrentProvince;

    private int mListLevel = LEVEL_PROVINCE;

    public CityFragment() {
    }

    private void listProvinces() {
        mListLevel = LEVEL_PROVINCE;
        if (null == mProvinces || 0 == mProvinces.size()) {
            new Thread() {
                @Override
                public void run() {
                    mApp.mWeAPIs
                            .listProvinces(mListProvincesCallback);
                }
            }.start();
        } else {
            MessageUtil.postMessage(mMsgHandler, CMD_REFRESH_PROVINCE_LIST);
        }
    }

    private void listCities(final String prefix) {
        mListLevel = LEVEL_CITY;
        new Thread() {
            @Override
            public void run() {
                mApp.mWeAPIs
                        .listCities(prefix, mListCitiesCallback);
            }
        }.start();
    }

    private void listOperators(final String cityCode) {
        mListLevel = LEVEL_OPERATOR;
        new Thread() {
            @Override
            public void run() {
                mApp.mWeAPIs
                        .listOperators(cityCode, mListOperatorCallback);
            }
        }.start();
    }

    private void refreshCities(int level) {
        if (null == mCityAdapter) {
            mCityAdapter = new CityAdapter(mParent);
        }
        switch(level) {
            case LEVEL_PROVINCE:
                mCityAdapter.setCities(mProvinces);
                break;

            case LEVEL_CITY:
                mCityAdapter.setCities(mCities);
                break;

            default:
                Log.e(TAG, "invalid level : " + level);
                return;
        }
        mCityList.setAdapter(mCityAdapter);
        mCityAdapter.notifyDataSetChanged();
    }

    private void refreshOperators() {
        if (null == mOperatorAdapter) {
            mOperatorAdapter = new OperatorAdapter(mParent);
        }
        mOperatorAdapter.setOperators(mOperators);
        mCityList.setAdapter(mOperatorAdapter);
        mOperatorAdapter.notifyDataSetChanged();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        getFrom();
        View view = inflater.inflate(R.layout.fragment_city, container, false);
        mApp = (IRApplication) getActivity().getApplication();

        mMsgHandler = new MsgHandler(this);

        mCityList = (ListView) view.findViewById(R.id.lv_city_list);

        mCityList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                switch(mListLevel) {
                    case LEVEL_PROVINCE:
                        mCurrentProvince = mProvinces.get(position);
                        String codePrefix = mCurrentProvince.getCode().substring(0, 2);
                        listCities(codePrefix);
                        break;

                    case LEVEL_CITY:
                        City city = mCities.get(position);
                        mParent.setCurrentCity(city);
                        String cityCode = city.getCode();
                        listOperators(cityCode);
                        break;

                    case LEVEL_OPERATOR:
                        StbOperator operator = (StbOperator)mOperatorAdapter.getItem(position);
                        mParent.setCurrentOperator(operator);
                        MessageUtil.postMessage(mParent.mMsgHandler,
                                CreateActivity.PAGE_INDEX,
                                CreateActivity.PAGE_CITY);
                        break;

                    default:
                        break;
                }
            }
        });

        listProvinces();

        return view;
    }

    @Override
    public boolean onBackPressed() {
        if (LEVEL_PROVINCE == mListLevel) {
            return super.onBackPressed();
        } else if (LEVEL_CITY == mListLevel) {
            listProvinces();
        } else if (LEVEL_OPERATOR == mListLevel) {
            String prefix = mCurrentProvince.getCode().substring(0, 2);
            listCities(prefix);
        }
        return true;
    }

    private static class MsgHandler extends Handler {

        WeakReference<CityFragment> mCityFragment;

        MsgHandler(CityFragment fragment) {
            mCityFragment = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(Message msg) {
            int cmd = msg.getData().getInt(MessageUtil.KEY_CMD);
            Log.d(TAG, "handle message " + Integer.toString(cmd));

            CityFragment cityFragment = mCityFragment.get();
            switch (cmd) {
                case CMD_REFRESH_PROVINCE_LIST :
                    cityFragment.refreshCities(LEVEL_PROVINCE);
                    break;

                case CMD_REFRESH_CITY_LIST:
                    cityFragment.refreshCities(LEVEL_CITY);
                    break;

                case CMD_REFRESH_OPERATOR_LIST:
                    cityFragment.refreshOperators();
                    break;

                default:
                    break;
            }
        }
    }

}
