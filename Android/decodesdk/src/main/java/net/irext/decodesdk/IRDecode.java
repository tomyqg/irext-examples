package net.irext.decodesdk;

import net.irext.decodesdk.bean.ACStatus;
import net.irext.decodesdk.bean.TemperatureRange;
import net.irext.decodesdk.utils.Constants;

/**
 * Filename:       IRDecode.java
 * Revised:        Date: 2017-04-22
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Wrapper-sdk of IR decode
 * <p>
 * Revision log:
 * 2017-04-23: created by strawmanbobi
 */
public class IRDecode {

    private static final String TAG = IRDecode.class.getSimpleName();

    static {
        System.loadLibrary("irdecode");
    }

    private native int irACLibOpen(String fileName);

    private native int[] irACControl(ACStatus acStatus, int functionCode);

    private native void irACLibClose();

    private native TemperatureRange irACGetTemperatureRange(int acMode);

    private native int irACGetSupportedMode();

    private native int irACGetSupportedWindSpeed(int acMode);

    private native int irACGetSupportedSwing(int acMode);

    private native int irTVLibOpen(String fileName, int irHexEncode);

    private native int[] irTVControl(int key_number);

    private native void irTVLibClose();

    private static IRDecode mInstance;

    public static IRDecode getInstance() {
        if (null == mInstance) {
            mInstance = new IRDecode();
        }
        return mInstance;
    }

    public int openACBinary(String fileName) {
        return irACLibOpen(fileName);
    }

    public int[] decodeACBinary(ACStatus acStatus, int functionCode) {
        return irACControl(acStatus, functionCode);
    }

    public void closeACBinary() {
        irACLibClose();
    }

    public TemperatureRange getTemperatureRange(int acMode) {
        return irACGetTemperatureRange(acMode);
    }

    public int[] getACSupportedMode() {
        // cool, heat, auto, fan, de-humidification
        int []retSupportedMode = {0, 0, 0, 0, 0};
        int supportedMode = irACGetSupportedMode();
        for (int i = Constants.ACMode.MODE_COOL.getValue(); i <= Constants.ACMode.MODE_DEHUMIDITY.getValue(); i++) {
            retSupportedMode[i] = (supportedMode >>> 1) & 1;
        }
        return retSupportedMode;
    }

    public int[] getACSupportedWindSpeed(int acMode) {
        // auto, low, medium, high
        int []retSupportedWindSpeed = {0, 0, 0, 0};
        int supportedWindSpeed = irACGetSupportedWindSpeed(acMode);
        for (int i = Constants.ACWindSpeed.SPEED_AUTO.getValue();
             i <= Constants.ACWindSpeed.SPEED_HIGH.getValue();
             i++) {
            retSupportedWindSpeed[i] = (supportedWindSpeed >>> 1) & 1;
        }
        return retSupportedWindSpeed;
    }

    public int[] getACSupportedSwing(int acMode) {
        // swing-on, swing-off
        int []retSupportedSwing= {0, 0};
        int supportedSwing = irACGetSupportedSwing(acMode);
        for (int i = Constants.ACSwing.SWING_ON.getValue();
             i <= Constants.ACSwing.SWING_OFF.getValue();
             i++) {
            retSupportedSwing[i] = (supportedSwing >>> 1) & 1;
        }
        return retSupportedSwing;
    }

    public int openTVBinary(String fileName, int subCategory) {
        int isHexType = 0;
        if (2 == subCategory) {
            isHexType = 1;
        }
        return irTVLibOpen(fileName, isHexType);
    }

    public int[] decodeTVBinary(int keyCode) {
        return irTVControl(keyCode);
    }

    public void closeTVBinary() {
        irTVLibClose();
    }
}
