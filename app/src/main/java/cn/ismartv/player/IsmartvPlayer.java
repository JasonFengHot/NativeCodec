package cn.ismartv.player;

import java.util.concurrent.CopyOnWriteArraySet;

/**
 * Created by huibin on 09/08/2017.
 */

public class IsmartvPlayer {
    private final CopyOnWriteArraySet listeners;

    public IsmartvPlayer() {
        listeners = new CopyOnWriteArraySet();
    }

    public native void prepare(String source);

    public native void setPlayWhenReady();

    public native void seekTo();

    public native void stop();

    public native void release();

    public void addListener(EventListener listener) {
        listeners.add(listener);
    }

    public void removeListener(EventListener listener) {
        listeners.remove(listener);
    }

    interface EventListener {

        void onPlayerError(Exception error);

    }

}
