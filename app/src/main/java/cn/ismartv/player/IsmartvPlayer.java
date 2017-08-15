package cn.ismartv.player;

import android.view.Surface;

import java.util.concurrent.CopyOnWriteArraySet;

/**
 * Created by huibin on 09/08/2017.
 */

public class IsmartvPlayer {
    private final CopyOnWriteArraySet listeners;

    public IsmartvPlayer() {
        listeners = new CopyOnWriteArraySet();
    }

    public native boolean _prepare(String source);

    public native void _start();

    public native void _setSurface(Surface surface);

    public native void _setPlayingStreamingMediaPlayer(boolean bool);

    public native void _stop();

    public native void _rewind();



//
//    public native void seekTo();
//
//    public native void stop();
//
//    public native void release();

//    public void addListener(EventListener listener) {
//        listeners.add(listener);
//    }
//
//    public void removeListener(EventListener listener) {
//        listeners.remove(listener);
//    }
//
//    interface EventListener {
//
//        void onPlayerError(Exception error);
//
//    }

}
