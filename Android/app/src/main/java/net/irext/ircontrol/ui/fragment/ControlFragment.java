package net.irext.ircontrol.ui.fragment;

import android.content.Context;
import android.hardware.ConsumerIrManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Vibrator;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.*;

import net.irext.decodesdk.bean.ACStatus;
import net.irext.decodesdk.IRDecode;
import net.irext.decodesdk.utils.Constants;
import net.irext.ircontrol.R;
import net.irext.ircontrol.bean.RemoteControl;
import net.irext.ircontrol.ui.activity.ControlActivity;
import net.irext.ircontrol.utils.FileUtils;
import net.irext.ircontrol.utils.MessageUtil;

import java.lang.ref.WeakReference;

/**
 * Filename:       ControlFragment.java
 * Revised:        Date: 2017-04-22
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Control fragment containing control panel
 * <p>
 * Revision log:
 * 2017-04-22: created by strawmanbobi
 */
public class ControlFragment extends Fragment implements View.OnClickListener {

    private static final String TAG = ControlFragment.class.getSimpleName();

    private static final int VIB_TIME = 60;

    private static final int CMD_GET_REMOTE_CONTROL = 0;

    private static final int KEY_POWER = 0;
    private static final int KEY_UP = 1;
    private static final int KEY_DOWN = 2;
    private static final int KEY_LEFT = 3;
    private static final int KEY_RIGHT = 4;
    private static final int KEY_OK = 5;
    private static final int KEY_PLUS = 6;
    private static final int KEY_MINUS = 7;
    private static final int KEY_BACK = 8;
    private static final int KEY_HOME = 9;
    private static final int KEY_MENU = 10;

    private MsgHandler mHandler;

    private ControlActivity mParent;
    private Long mRemoteID;
    private RemoteControl mCurrentRemoteControl;

    // define the single instance of IRDecode
    private IRDecode mIRDecode;

    public ControlFragment() {
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        mIRDecode = IRDecode.getInstance();
        mHandler = new MsgHandler(this);

        mParent = (ControlActivity)getActivity();
        View view = inflater.inflate(R.layout.fragment_control, container, false);

        ImageButton mBtnPower = (ImageButton) view.findViewById(R.id.iv_power);
        ImageButton mBtnBack = (ImageButton) view.findViewById(R.id.iv_back);
        ImageButton mBtnHome = (ImageButton) view.findViewById(R.id.iv_home);
        ImageButton mBtnMenu = (ImageButton) view.findViewById(R.id.iv_menu);
        ImageButton mBtnUp = (ImageButton) view.findViewById(R.id.iv_up);
        ImageButton mBtnDown = (ImageButton) view.findViewById(R.id.iv_down);
        ImageButton mBtnLeft = (ImageButton) view.findViewById(R.id.iv_left);
        ImageButton mBtnRight = (ImageButton) view.findViewById(R.id.iv_right);
        ImageButton mBtnOK = (ImageButton) view.findViewById(R.id.iv_ok);
        ImageButton mBtnPlus = (ImageButton) view.findViewById(R.id.iv_plus);
        ImageButton mBtnMinus = (ImageButton) view.findViewById(R.id.iv_minus);

        mBtnPower.setOnClickListener(this);
        mBtnBack.setOnClickListener(this);
        mBtnHome.setOnClickListener(this);
        mBtnMenu.setOnClickListener(this);
        mBtnUp.setOnClickListener(this);
        mBtnDown.setOnClickListener(this);
        mBtnLeft.setOnClickListener(this);
        mBtnRight.setOnClickListener(this);
        mBtnOK.setOnClickListener(this);
        mBtnPlus.setOnClickListener(this);
        mBtnMinus.setOnClickListener(this);

        return view;
    }

    @Override
    public void onResume() {
        super.onResume();

        mRemoteID  = getArguments().getLong(ControlActivity.KEY_REMOTE_ID, -1L);
        if (-1 == mRemoteID) {
            Log.d(TAG, "remote ID IS NULL");
        } else {
            getRemote();
        }

    }

    private void getRemote() {
        new Thread() {
            @Override
            public void run() {
                MessageUtil.postMessage(mHandler, CMD_GET_REMOTE_CONTROL);
            }
        }.start();
    }

    private void showRemote() {
        mCurrentRemoteControl = RemoteControl.getRemoteControl(mRemoteID);
        if (null != mCurrentRemoteControl) {
            int category = mCurrentRemoteControl.getCategoryId();
            String binFileName = FileUtils.BIN_PATH + FileUtils.FILE_NAME_PREFIX +
                    mCurrentRemoteControl.getRemoteMap() + FileUtils.FILE_NAME_EXT;

            /* decode SDK - load binary file */
            // int ret = mIRDecode.openBinary(category, mCurrentRemoteControl.getSubCategory(), binFileName);
            // Log.d(TAG, "open binary result = " + ret);
        }
    }

    public void closeIRBinary() {
        // mIRDecode.closeBinary();
    }

    @Nullable
    private int[] irControl(int keyCode) {
        int inputKeyCode;
        ACStatus acStatus = new ACStatus();
        /* decode SDK - decode according to key code */
        if (Constants.CategoryID.AIR_CONDITIONER.getValue() == mCurrentRemoteControl.getCategoryId()) {
            acStatus.setAcPower(Constants.ACPower.POWER_OFF.getValue());
            acStatus.setAcMode(Constants.ACMode.MODE_COOL.getValue());
            acStatus.setAcTemp(Constants.ACTemperature.TEMP_24.getValue());
            acStatus.setAcWindSpeed(Constants.ACWindSpeed.SPEED_AUTO.getValue());
            acStatus.setAcWindDir(Constants.ACSwing.SWING_ON.getValue());
            acStatus.setAcDisplay(0);
            acStatus.setAcTimer(0);
            acStatus.setAcSleep(0);

            switch(keyCode) {
                case KEY_POWER:
                    // power key --> change power
                    inputKeyCode = Constants.ACFunction.FUNCTION_SWITCH_POWER.getValue();
                    break;
                case KEY_UP:
                    // up key --> change wind speed
                    inputKeyCode = Constants.ACFunction.FUNCTION_SWITCH_WIND_SPEED.getValue();
                    break;
                case KEY_DOWN:
                    // down key --> change wind dir
                    inputKeyCode = Constants.ACFunction.FUNCTION_SWITCH_WIND_DIR.getValue();
                    break;
                case KEY_RIGHT:
                    // right key --> change mode
                    inputKeyCode = Constants.ACFunction.FUNCTION_CHANGE_MODE.getValue();
                    break;
                case KEY_OK:
                    // center key --> fix wind dir
                    inputKeyCode = Constants.ACFunction.FUNCTION_SWITCH_SWING.getValue();
                    break;
                case KEY_PLUS:
                    // plus key --> temp up
                    inputKeyCode = Constants.ACFunction.FUNCTION_TEMPERATURE_UP.getValue();
                    break;
                case KEY_MINUS:
                    // minus key --> temp down
                    inputKeyCode = Constants.ACFunction.FUNCTION_TEMPERATURE_DOWN.getValue();
                    break;

                default:
                    return null;
            }
        } else {
            inputKeyCode = keyCode;
        }

        /* decode SDK - decode from binary */
        /* translate key code for AC according to the mapping above */
        /* ac status is useless for decoding devices other than AC, it's an optional parameter */
        /* change wind dir is an optional parameter, set to 0 as default */
        // return mIRDecode.decodeBinary(inputKeyCode, acStatus, 0);
        return null;
    }

    // control
    @Override
    public void onClick(View v) {
        vibrate(mParent);
        int []decoded = null;
        switch(v.getId()) {
            case R.id.iv_power:
                decoded = irControl(KEY_POWER);
                break;

            case R.id.iv_up:
                decoded = irControl(KEY_UP);
                break;

            case R.id.iv_down:
                decoded = irControl(KEY_DOWN);
                break;

            case R.id.iv_left:
                decoded = irControl(KEY_LEFT);
                break;

            case R.id.iv_right:
                decoded = irControl(KEY_RIGHT);
                break;

            case R.id.iv_ok:
                decoded = irControl(KEY_OK);
                break;

            case R.id.iv_plus:
                decoded = irControl(KEY_PLUS);
                break;

            case R.id.iv_minus:
                decoded = irControl(KEY_MINUS);
                break;

            case R.id.iv_back:
                decoded = irControl(KEY_BACK);
                break;

            case R.id.iv_home:
                decoded = irControl(KEY_HOME);
                break;

            case R.id.iv_menu:
                decoded = irControl(KEY_MENU);
                break;
        }
        // send decoded integer array to IR emitter
        ConsumerIrManager irEmitter =
                (ConsumerIrManager) mParent.getSystemService(Context.CONSUMER_IR_SERVICE);
        if (irEmitter.hasIrEmitter()) {
            irEmitter.transmit(38000, decoded);
        }
    }

    private static class MsgHandler extends Handler {

        WeakReference<ControlFragment> mMainFragment;

        MsgHandler(ControlFragment fragment) {
            mMainFragment = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(Message msg) {
            int cmd = msg.getData().getInt(MessageUtil.KEY_CMD);

            ControlFragment controlFragment = mMainFragment.get();
            switch (cmd) {

                case CMD_GET_REMOTE_CONTROL:
                    controlFragment.showRemote();
                    break;

                default:
                    break;
            }
        }
    }

    // vibrate on button click in this fragment
    private static void vibrate(Context context) {
        Vibrator vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        vibrator.vibrate(VIB_TIME);
    }
}
