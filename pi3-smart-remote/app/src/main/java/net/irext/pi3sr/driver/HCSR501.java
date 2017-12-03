package net.irext.pi3sr.driver;

import android.util.Log;

import com.google.android.things.pio.Gpio;
import com.google.android.things.pio.GpioCallback;

import java.io.IOException;

/**
 *
 * HC-SR501 driver
 *
 * created by strawmanbobi 2017-06-25
 */
public class HCSR501 implements MotionSensor {

    private static final String TAG = HCSR501.class.getSimpleName();

    private final Gpio bus;

    private final MotionSensor.Listener listener;

    public HCSR501(Gpio bus, Listener listener) {
        this.bus = bus;
        this.listener = listener;
    }

    @Override
    public void startup() {
        try {
            bus.setDirection(Gpio.DIRECTION_IN);
            bus.setActiveType(Gpio.ACTIVE_HIGH);
            bus.setEdgeTriggerType(Gpio.EDGE_RISING);
        } catch (IOException e) {
            throw new IllegalStateException("Sensor can't start - App is foobar'd", e);
        }
        try {
            bus.registerGpioCallback(callback);
        } catch (IOException e) {
            throw new IllegalStateException("Sensor can't register callback", e);
        }
    }

    private final GpioCallback callback = new GpioCallback() {
        @Override
        public boolean onGpioEdge(Gpio gpio) {
            listener.onMovement();
            return true;
        }
    };

    @Override
    public void shutdown() {
        bus.unregisterGpioCallback(callback);
        try {
            bus.close();
        } catch (IOException e) {
            Log.e(TAG, "Failed to shut down. You might get errors next time you try to start.", e);
        }
    }

}
