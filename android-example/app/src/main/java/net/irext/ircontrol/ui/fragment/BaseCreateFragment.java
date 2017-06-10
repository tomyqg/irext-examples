package net.irext.ircontrol.ui.fragment;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import net.irext.ircontrol.ui.activity.CreateActivity;
import net.irext.ircontrol.utils.MessageUtil;

/**
 * Filename:       BaseCreateFragment.java
 * Revised:        Date: 2017-04-10
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Base Fragment class for create fragments
 * <p>
 * Revision log:
 * 2017-04-10: created by strawmanbobi
 */
public abstract class BaseCreateFragment extends Fragment {

    protected static final String TAG = BaseCreateFragment.class.getSimpleName();

    int mFrom;
    CreateActivity mParent;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mParent = (CreateActivity) getActivity();
        return null;
    }

    public boolean onBackPressed() {
        if (-1 != mFrom) {
            MessageUtil.postMessage(mParent.mMsgHandler, mFrom);
        }
        return true;
    }

    void getFrom() {
        int from = getArguments().getInt(CreateActivity.KEY_FROM);
        if (-1 == from) {
            Log.d(TAG, "FROM IS NULL");
        } else {
            mFrom = from;
        }
    }

}
