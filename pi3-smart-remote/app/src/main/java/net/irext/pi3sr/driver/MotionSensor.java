package net.irext.pi3sr.driver;

/**
 *
 * motion sensor interface
 *
 * created by strawmanbobi 2017-06-25
 */
public interface MotionSensor {

    void startup();

    void shutdown();

    public interface Listener {
        void onMovement();
    }

}
