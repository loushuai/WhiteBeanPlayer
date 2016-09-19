package com.whitebean.media;

import android.view.SurfaceHolder;

import java.io.IOException;

/**
 * Created by loushuai on 2016/8/7.
 */
public interface IMediaPlayer {
    void setDisplay(SurfaceHolder sh);
    void setDataSource(String path)  throws IOException, IllegalArgumentException, SecurityException, IllegalStateException;
    void start() throws IllegalStateException;
    void pause() throws IllegalStateException;
    void seekTo(long msec) throws IllegalStateException;
    void release();

    int getVideoWidth();
    int getVideoHeight();
    boolean isPlaying();
    long getCurrentPosition();
    long getDuration();

    void setOnPreparedListener(OnPreparedListener listener);

    public interface OnCompletionListener
    {
        void onCompletion(IMediaPlayer mp);
    }

    public interface OnBufferingUpdateListener
    {
        void onBufferingUpdate(IMediaPlayer mp, int percent);
    }

    public interface OnSeekCompleteListener
    {
        public void onSeekComplete(IMediaPlayer mp);
    }

    public interface OnPreparedListener
    {
        void onPrepared(IMediaPlayer mp);
    }

    public interface OnErrorListener
    {
        boolean onError(IMediaPlayer mp, int what, int extra);
    }

    public interface OnInfoListener
    {
        boolean onInfo(IMediaPlayer mp, int what, int extra);
    }
}
