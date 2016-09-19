package com.whitebean.activities;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.WindowManager;

import com.example.loushuai.whitebeandemo.R;
import com.whitebean.widget.MediaController;
import com.whitebean.widget.VideoView;

/**
 * Created by loushuai on 2016/8/6.
 */
public class VideoActivity extends AppCompatActivity {
    private static final String TAG = "VideoActivity";

    private MediaController mMediaController;
    private String mVideoPath;
    private VideoView mVideoView;

    public static Intent newIntent(Context context, String videoPath, String videoTitle) {
        Intent intent = new Intent(context, VideoActivity.class);
        intent.putExtra("videoPath", videoPath);
        intent.putExtra("videoTitle", videoTitle);
        return intent;
    }

    public static void intentTo(Context context, String videoPath, String videoTitle) {
        context.startActivity(newIntent(context, videoPath, videoTitle));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        mVideoPath = getIntent().getStringExtra("videoPath");

        mMediaController = new MediaController(this, false);

        mVideoView = (VideoView) findViewById(R.id.video_view);
        mVideoView.setMediaController(mMediaController);
        if (mVideoPath != null)
            mVideoView.setVideoPath(mVideoPath);
        mVideoView.start();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }
}
