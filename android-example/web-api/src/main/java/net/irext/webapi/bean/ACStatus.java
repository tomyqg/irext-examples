package net.irext.webapi.bean;

/**
 * Filename:       ACStatus.java
 * Revised:        Date: 2017-03-28
 * Revision:       Revision: 1.0
 * <p>
 * Description:    Status descriptor for air-conditioner
 * <p>
 * Revision log:
 * 2017-03-28: created by strawmanbobi
 */
public class ACStatus {

    private static final String TAG = ACStatus.class.getSimpleName();

    private int acPower;
    private int acTemp;
    private int acMode;
    private int acWindDir;
    private int acWindSpeed;
    private int acDisplay;
    private int acSleep;
    private int acTimer;

    public ACStatus() {
    }

    public ACStatus(int acPower, int acMode, int acTemp, int acWindSpeed, int acWindDir,
                    int acDisplay, int acSleep, int acTimer) {
        this.acPower = acPower;
        this.acTemp = acTemp;
        this.acMode = acMode;
        this.acWindDir = acWindDir;
        this.acWindSpeed = acWindSpeed;
        this.acDisplay = acDisplay;
        this.acSleep = acSleep;
        this.acTimer = acTimer;
    }

    public int getAcPower() {
        return acPower;
    }

    public void setAcPower(int acPower) {
        this.acPower = acPower;
    }

    public int getAcTemp() {
        return acTemp;
    }

    public void setAcTemp(int acTemp) {
        this.acTemp = acTemp;
    }

    public int getAcMode() {
        return acMode;
    }

    public void setAcMode(int acMode) {
        this.acMode = acMode;
    }

    public int getAcWindDir() {
        return acWindDir;
    }

    public void setAcWindDir(int acWindDir) {
        this.acWindDir = acWindDir;
    }

    public int getAcWindSpeed() {
        return acWindSpeed;
    }

    public void setAcWindSpeed(int acWindSpeed) {
        this.acWindSpeed = acWindSpeed;
    }

    public int getAcDisplay() {
        return acDisplay;
    }

    public void setAcDisplay(int acDisplay) {
        this.acDisplay = acDisplay;
    }

    public int getAcSleep() {
        return acSleep;
    }

    public void setAcSleep(int acSleep) {
        this.acSleep = acSleep;
    }

    public int getAcTimer() {
        return acTimer;
    }

    public void setAcTimer(int acTimer) {
        this.acTimer = acTimer;
    }
}
