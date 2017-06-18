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

    private native int irOpen(int category, int subCate, String fileName);

    private native int irOpenBinary(int category, int subCate, byte[] binaries, int binLength);

    private native int[] irDecode(int keyCode, ACStatus acStatus, int changeWindDirection);

    private native void irClose();

    private native TemperatureRange irACGetTemperatureRange(int acMode);

    private native int irACGetSupportedMode();

    private native int irACGetSupportedWindSpeed(int acMode);

    private native int irACGetSupportedSwing(int acMode);

    private static IRDecode mInstance;

    public static IRDecode getInstance() {
        if (null == mInstance) {
            mInstance = new IRDecode();
        }
        return mInstance;
    }

    private IRDecode() {

    }

    public int openFile(int category, int subCate, String fileName) {
        return irOpen(category, subCate, fileName);
    }

    public int openBinary(int category, int subCate, byte[] binaries, int binLength) {
        return irOpenBinary(category, subCate, binaries, binLength);
    }

    public int[] decodeBinary(int keyCode, ACStatus acStatus, int changeWindDir) {
        if (null == acStatus) {
            acStatus = new ACStatus();
        }
        return irDecode(keyCode, acStatus, changeWindDir);
    }

    public void closeBinary() {
        irClose();
    }

    public TemperatureRange getTemperatureRange(int acMode) {
        return irACGetTemperatureRange(acMode);
    }

    public int[] getACSupportedMode() {
        // cool, heat, auto, fan, de-humidification
        int []retSupportedMode = {0, 0, 0, 0, 0};
        int supportedMode = irACGetSupportedMode();
        for (int i = Constants.ACMode.MODE_COOL.getValue(); i <=
                Constants.ACMode.MODE_DEHUMIDITY.getValue(); i++) {
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
}
