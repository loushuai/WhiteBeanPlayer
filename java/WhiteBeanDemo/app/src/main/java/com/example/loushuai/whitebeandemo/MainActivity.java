package com.example.loushuai.whitebeandemo;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

import java.io.IOException;

import com.whitebean.media.MediaPlayer;

public class MainActivity extends Activity {
    private SurfaceView mVideoSurface;
    private SurfaceHolder mSurfaceHolder;
    SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback()
    {
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {}

        public void surfaceCreated(SurfaceHolder holder) {
            mSurfaceHolder = holder;

            try {
                MediaPlayer mp = new MediaPlayer();
                mp.setDisplay(mSurfaceHolder);
                mp.setDataSource("/sdcard/video.mp4");
                mp.prepare();
                mp.start();
            } catch (IOException ex) {
                return;
            } catch (IllegalArgumentException ex) {
                return;
            } finally {

            }
        }

        public void surfaceDestroyed(SurfaceHolder holder) {}
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        mVideoSurface = (SurfaceView) findViewById(R.id.video_surface);
        mSurfaceHolder = mVideoSurface.getHolder();
        mSurfaceHolder.addCallback(mSHCallback);
//        mSurfaceHolder.setFixedSize(320, 220);
    }

//    @Override
//    public void onConfigurationChanged(Configuration newConfig) {
//        super.onConfigurationChanged(newConfig);
//
//    }
}
