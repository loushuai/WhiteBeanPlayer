package com.whitebean.media;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.io.IOException;
import java.lang.ref.WeakReference;

/**
 * Created by loushuai on 2016/6/26.
 */
public class MediaPlayer implements IMediaPlayer {

    private static final int MEDIA_NOP = 0; // interface test message
    private static final int MEDIA_PREPARED = 1;

    static {
        System.loadLibrary("whitebean");
        native_init();
    }

    private final static String TAG = "MediaPlayer";

    // accessed by native methods
    private int mNativeContext;
    private SurfaceHolder mSurfaceHolder;
    private EventHandler mEventHandler;

    private OnPreparedListener mOnPreparedListener;

    private int mVideoWidth;
    private int mVideoHeight;

    public MediaPlayer() {
        initPlayer();
    }

    private void initPlayer() {
        native_setup(new WeakReference<MediaPlayer>(this));

        Looper looper;
        if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }
    }

    public void setDisplay(SurfaceHolder sh) {
        mSurfaceHolder = sh;
        Surface surface;
        if (sh != null) {
            surface = sh.getSurface();
        } else {
            surface = null;
        }
        _setVideoSurface(surface);
    }

    public void setDataSource(String path)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        _setDataSource(path);
    }

    public  void start() throws IllegalStateException {
        _start();
    }

    @Override
    public void pause() throws IllegalStateException {
        _pause();
    }

    @Override
    public void release() {
        _release();
    }

    @Override
    public native boolean isPlaying();

    @Override
    public native void seekTo(long msec) throws IllegalStateException;

    @Override
    public native long getCurrentPosition();

    @Override
    public native long getDuration();

    private native void _setVideoSurface(Surface surface);
    private native void _setDataSource(String path)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException;
    public native void prepare() throws IOException, IllegalStateException;
    private native void _start() throws IllegalStateException;
    private native void _pause() throws IllegalStateException;
    private native void _release();
    private static native final void native_init();
    private native final void native_setup(Object mediaplayer_this);

    private class EventHandler extends Handler
    {
        private MediaPlayer mMediaPlayer;
        private final WeakReference<MediaPlayer> mWeakPlayer;

        public EventHandler(MediaPlayer mp, Looper looper) {
            super(looper);
            mMediaPlayer = mp;
            mWeakPlayer = new WeakReference<MediaPlayer>(mp);
        }

        @Override
        public void handleMessage(Message msg) {
            MediaPlayer player = mWeakPlayer.get();

            if (player == null || player.mNativeContext == 0) {
                Log.w(TAG, "mediaplayer went away with unhandled events");
                return;
            }

            Log.d(TAG, "handleMessage" + msg.what);
            switch(msg.what) {
                case MEDIA_PREPARED:
                    Log.d(TAG, "notifyOnPrepared");
                    player.notifyOnPrepared();
                    return;

            default:
                Log.e(TAG, "Unknown message type " + msg.what);
                return;
            }
        }
    }

    private static void postEventFromNative(Object mediaplayer_ref,
                                            int what, int arg1, int arg2, Object obj)
    {
        Log.d(TAG, "postEventFromNative");

        MediaPlayer mp = (MediaPlayer)((WeakReference)mediaplayer_ref).get();
        if (mp == null) {
            Log.d(TAG, "postEventFromNative 1");
            return;
        }

        Log.d(TAG, "postEventFromNative 2");

        if (mp.mEventHandler != null) {
            Log.d(TAG, "postEventFromNative 3");
            Message m = mp.mEventHandler.obtainMessage(what, arg1, arg2, obj);
            mp.mEventHandler.sendMessage(m);
        }

        Log.d(TAG, "postEventFromNative 4");
    }

    @Override
    public int getVideoWidth() {
        return mVideoWidth;
    }

    @Override
    public int getVideoHeight() {
        return mVideoHeight;
    }

    public void setOnPreparedListener(OnPreparedListener listener) {
        mOnPreparedListener = listener;
    }

    private void notifyOnPrepared() {
        if (mOnPreparedListener != null)
            mOnPreparedListener.onPrepared(this);
    }

}
