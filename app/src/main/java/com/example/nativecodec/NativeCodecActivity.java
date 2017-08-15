/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.nativecodec;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import cn.ismartv.player.IsmartvPlayer;
import cn.ismartv.player.Util;

public class NativeCodecActivity extends Activity implements OnClickListener, SurfaceHolder.Callback, AdapterView.OnItemSelectedListener, CompoundButton.OnCheckedChangeListener, SeekBar.OnSeekBarChangeListener {
    static {
        System.loadLibrary("native-codec-jni");
    }

    private static final String TAG = "NativeCodecActivity";

    private String mSourceString = "/sdcard/chinesemovie_0.mp4";

    private SurfaceView mSurfaceView1;
    private SurfaceHolder mSurfaceHolder1;

    private VideoSink mSelectedVideoSink;
    private VideoSink mNativeCodecPlayerVideoSink;

    private SurfaceHolderVideoSink mSurfaceHolder1VideoSink;
    private GLViewVideoSink mGLView1VideoSink;

    private boolean mCreated = false;
    private boolean mIsPlaying = false;

    private IsmartvPlayer mIsmartvPlayer;

    private MyGLSurfaceView mGLView1;

    private RadioButton mRadio1;
    private RadioButton mRadio2;
    private Button startBtn;
    private Button rewindBtn;

    private TextView durationTextView;
    private SeekBar progressbar;

    private int currentSeekPosition;

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main);

        mIsmartvPlayer = new IsmartvPlayer();

        mGLView1 = (MyGLSurfaceView) findViewById(R.id.glsurfaceview1);

        // set up the Surface 1 video sink
        mSurfaceView1 = (SurfaceView) findViewById(R.id.surfaceview1);
        mSurfaceHolder1 = mSurfaceView1.getHolder();
        mSurfaceHolder1.addCallback(this);

        // initialize content source spinner
        Spinner sourceSpinner = (Spinner) findViewById(R.id.source_spinner);
        ArrayAdapter<CharSequence> sourceAdapter = ArrayAdapter.createFromResource(
                this, R.array.source_array, android.R.layout.simple_spinner_item);
        sourceAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        sourceSpinner.setAdapter(sourceAdapter);
        sourceSpinner.setOnItemSelectedListener(this);

        mRadio1 = (RadioButton) findViewById(R.id.radio1);
        mRadio2 = (RadioButton) findViewById(R.id.radio2);

        mRadio1.setOnCheckedChangeListener(this);
        mRadio2.setOnCheckedChangeListener(this);
        mRadio2.toggle();

        // the surfaces themselves are easier targets than the radio buttons
        mSurfaceView1.setOnClickListener(this);
        mGLView1.setOnClickListener(this);

        // native MediaPlayer videoStart/pause
        startBtn = ((Button) findViewById(R.id.start_native));
        startBtn.setOnClickListener(this);

        // native MediaPlayer rewind
        rewindBtn = ((Button) findViewById(R.id.rewind_native));
        rewindBtn.setOnClickListener(this);

        durationTextView = (TextView) findViewById(R.id.duration);

        progressbar = (SeekBar) findViewById(R.id.progressbar);
        progressbar.setOnSeekBarChangeListener(this);

    }

    void switchSurface() {
        if (mCreated && mNativeCodecPlayerVideoSink != mSelectedVideoSink) {
            // shutdown and recreate on other surface
            Log.i("@@@", "shutting down player");
            mIsmartvPlayer._stop();
            mCreated = false;
            mSelectedVideoSink.useAsSinkForNative();
            mNativeCodecPlayerVideoSink = mSelectedVideoSink;
            if (mSourceString != null) {
                Log.i("@@@", "recreating player");
                mCreated = mIsmartvPlayer._prepare(mSourceString);
                Log.d(TAG, "duration: " + mIsmartvPlayer._getDuration());
                durationTextView.setText(Util.formatTime(mIsmartvPlayer._getDuration()));
                mIsPlaying = false;
            }
        }
    }

    /**
     * Called when the activity is about to be paused.
     */
    @Override
    protected void onPause() {
        mIsPlaying = false;
        mIsmartvPlayer._setPlayWhenReady(false);
        mGLView1.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mRadio2.isChecked()) {
            mGLView1.onResume();
        }
    }

    /**
     * Called when the activity is about to be destroyed.
     */
    @Override
    protected void onDestroy() {
        mIsmartvPlayer._stop();
        mCreated = false;
        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.surfaceview1:
                mRadio1.toggle();
                break;
            case R.id.glsurfaceview1:
                mRadio2.toggle();
                break;
            case R.id.start_native:
                videoStart();
                break;
            case R.id.rewind_native:
                videoRewind();
                break;
        }
    }

    private void videoStart() {
        if (!mCreated) {
            if (mNativeCodecPlayerVideoSink == null) {
                if (mSelectedVideoSink == null) {
                    return;
                }
                mSelectedVideoSink.useAsSinkForNative();
                mNativeCodecPlayerVideoSink = mSelectedVideoSink;
            }
            if (mSourceString != null) {
                mCreated = mIsmartvPlayer._prepare(mSourceString);
                durationTextView.setText(Util.formatTime(mIsmartvPlayer._getDuration()));
            }
        }
        if (mCreated) {
            mIsPlaying = !mIsPlaying;
            mIsmartvPlayer._setPlayWhenReady(mIsPlaying);
        }
    }

    private void videoRewind() {
        if (mNativeCodecPlayerVideoSink != null) {
            mIsmartvPlayer._rewind();
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.v(TAG, "surfaceChanged format=" + format + ", width=" + width + ", height=" + height);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v(TAG, "surfaceCreated");
        if (mRadio1.isChecked()) {
            mIsmartvPlayer._setSurface(holder.getSurface());
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.v(TAG, "surfaceDestroyed");
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
//        mSourceString = parent.getItemAtPosition(pos).toString();
//        Log.v(TAG, "onItemSelected " + mSourceString);
    }

    @Override
    public void onNothingSelected(AdapterView parent) {
//        Log.v(TAG, "onNothingSelected");
//        mSourceString = null;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        Log.i("@@@@", "oncheckedchanged");
        if (buttonView == mRadio1 && isChecked) {
            mRadio2.setChecked(false);
        }
        if (buttonView == mRadio2 && isChecked) {
            mRadio1.setChecked(false);
        }
        if (isChecked) {
            if (mRadio1.isChecked()) {
                if (mSurfaceHolder1VideoSink == null) {
                    mSurfaceHolder1VideoSink = new SurfaceHolderVideoSink(mSurfaceHolder1);
                }
                mSelectedVideoSink = mSurfaceHolder1VideoSink;
                mGLView1.onPause();
                Log.i("@@@@", "glview pause");
            } else {
                mGLView1.onResume();
                if (mGLView1VideoSink == null) {
                    mGLView1VideoSink = new GLViewVideoSink(mGLView1);
                }
                mSelectedVideoSink = mGLView1VideoSink;
            }
            switchSurface();
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        Log.d(TAG, "progress: " + progress);
        currentSeekPosition = progress;
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        Log.d(TAG, "onStartTrackingTouch");
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        Log.d(TAG, "onStopTrackingTouch");
        long positionMs = currentSeekPosition * mIsmartvPlayer._getDuration() / 100;
        mIsmartvPlayer._seekTo(positionMs);
    }

    // VideoSink abstracts out the difference between Surface and SurfaceTexture
    // aka SurfaceHolder and GLSurfaceView
    static abstract class VideoSink {

        abstract void setFixedSize(int width, int height);

        abstract void useAsSinkForNative();

    }

    class SurfaceHolderVideoSink extends VideoSink {

        private final SurfaceHolder mSurfaceHolder;

        SurfaceHolderVideoSink(SurfaceHolder surfaceHolder) {
            mSurfaceHolder = surfaceHolder;
        }

        @Override
        void setFixedSize(int width, int height) {
            mSurfaceHolder.setFixedSize(width, height);
        }

        @Override
        void useAsSinkForNative() {
            Surface s = mSurfaceHolder.getSurface();
            Log.i("@@@", "setting surface " + s);
            mIsmartvPlayer._setSurface(s);
        }

    }

    class GLViewVideoSink extends VideoSink {

        private final MyGLSurfaceView mMyGLSurfaceView;

        GLViewVideoSink(MyGLSurfaceView myGLSurfaceView) {
            mMyGLSurfaceView = myGLSurfaceView;
        }

        @Override
        void setFixedSize(int width, int height) {
        }

        @Override
        void useAsSinkForNative() {
            SurfaceTexture st = mMyGLSurfaceView.getSurfaceTexture();
            Surface s = new Surface(st);
            mIsmartvPlayer._setSurface(s);
            s.release();
        }

    }

}
