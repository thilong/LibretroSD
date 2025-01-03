/*
 * Copyright 2019 The Android Open Source Project
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

package com.mobileer.oboetester;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.IOException;
import java.util.Locale;

public class ManualGlitchActivity extends GlitchActivity {

    public static final String KEY_BUFFER_BURSTS = "buffer_bursts";
    public static final int VALUE_DEFAULT_BUFFER_BURSTS = 2;

    public static final String KEY_TOLERANCE = "tolerance";
    private static final float DEFAULT_TOLERANCE = 0.10f;

    private static final long MIN_DISPLAY_PERIOD_MILLIS = 500;
    private static final int WAVEFORM_SIZE = 400;

    private TextView mTextTolerance;
    private SeekBar mFaderTolerance;
    protected ExponentialTaper mTaperTolerance;

    private CheckBox mForceGlitchesBox;
    private CheckBox mAutoScopeBox;
    private WaveformView mWaveformView;
    private float[] mWaveform = new float[WAVEFORM_SIZE];
    private long mLastDisplayTime;

    private float   mTolerance = DEFAULT_TOLERANCE;

    private SeekBar.OnSeekBarChangeListener mToleranceListener = new SeekBar.OnSeekBarChangeListener() {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            setToleranceProgress(progress);
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
        }
    };

    protected void setToleranceProgress(int progress) {
        float tolerance = (float) mTaperTolerance.linearToExponential(
                ((double)progress) / FADER_PROGRESS_MAX);
        setTolerance(tolerance);
        mTextTolerance.setText("Tolerance = " + String.format(Locale.getDefault(), "%5.3f", tolerance));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mTextTolerance = (TextView) findViewById(R.id.textTolerance);
        mFaderTolerance = (SeekBar) findViewById(R.id.faderTolerance);
        mTaperTolerance = new ExponentialTaper(0.0, 0.5, 100.0);
        mFaderTolerance.setOnSeekBarChangeListener(mToleranceListener);
        setToleranceFader(DEFAULT_TOLERANCE);

        mForceGlitchesBox = (CheckBox) findViewById(R.id.boxForceGlitch);
        mAutoScopeBox = (CheckBox) findViewById(R.id.boxAutoScope);
        mWaveformView = (WaveformView) findViewById(R.id.waveview_audio);
    }

    private void setToleranceFader(float tolerance) {
        int progress = (int) Math.round((mTaperTolerance.exponentialToLinear(
                tolerance) * FADER_PROGRESS_MAX));
        mFaderTolerance.setProgress(progress);
    }

    @Override
    protected void inflateActivity() {
        setContentView(R.layout.activity_manual_glitches);
    }

    void configureStreamsFromBundle(Bundle bundle) {
        // Configure settings
        StreamConfiguration requestedInConfig = mAudioInputTester.requestedConfiguration;
        StreamConfiguration requestedOutConfig = mAudioOutTester.requestedConfiguration;
        IntentBasedTestSupport.configureStreamsFromBundle(bundle, requestedInConfig, requestedOutConfig);

        // Extract custom parameters from the bundle.
        float tolerance = bundle.getFloat(KEY_TOLERANCE, DEFAULT_TOLERANCE);
        setToleranceFader(tolerance);
        setTolerance(tolerance);
        mTolerance = tolerance;
    }

    @Override
    public void giveAdvice(String s) {
        mWaveformView.post(() -> {
            mWaveformView.setMessage(s);
            mWaveformView.invalidate();
        });
    }

    public void startAudioTest() throws IOException {
        super.startAudioTest();
        setToleranceProgress(mFaderTolerance.getProgress());
    }

    @Override
    public void startTestUsingBundle() {
        configureStreamsFromBundle(mBundleFromIntent);

        int durationSeconds = IntentBasedTestSupport.getDurationSeconds(mBundleFromIntent);
        int numBursts = mBundleFromIntent.getInt(KEY_BUFFER_BURSTS, VALUE_DEFAULT_BUFFER_BURSTS);

        try {
            openStartAudioTestUI();
            int sizeFrames = mAudioOutTester.getCurrentAudioStream().getFramesPerBurst() * numBursts;
            mAudioOutTester.getCurrentAudioStream().setBufferSizeInFrames(sizeFrames);

            // Schedule the end of the test.
            Handler handler = new Handler(Looper.getMainLooper()); // UI thread
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    stopAutomaticTest();
                }
            }, durationSeconds * 1000);
        } catch (IOException e) {
            String report = "Open failed: " + e.getMessage();
            maybeWriteTestResult(report);
            mTestRunningByIntent = false;
        } finally {
            mBundleFromIntent = null;
        }

    }

    void stopAutomaticTest() {
        String report = getCommonTestReport()
                + String.format(Locale.getDefault(), "tolerance = %5.3f\n", mTolerance)
                + mLastGlitchReport;
        onStopAudioTest(null);
        maybeWriteTestResult(report);
        mTestRunningByIntent = false;
    }

    // Only call from UI thread.
    @Override
    public void onTestBegan() {
        mAutoScopeBox.setChecked(true);
        mWaveformView.clearSampleData();
        mWaveformView.postInvalidate();
        super.onTestBegan();
    }

    // Called on UI thread
    @Override
    protected void onGlitchDetected() {
        if (mAutoScopeBox.isChecked()) {
            mAutoScopeBox.setChecked(false); // stop auto drawing of waveform
            mLastDisplayTime = 0; // force draw first glitch
        }
        long now = System.currentTimeMillis();
        Log.i(TAG,"onGlitchDetected: glitch");
        if ((now - mLastDisplayTime) > MIN_DISPLAY_PERIOD_MILLIS) {
            mLastDisplayTime = now;
            int numSamples = getGlitch(mWaveform);
            mWaveformView.setSampleData(mWaveform, 0, numSamples);
            int glitchLength = getGlitchLength();
            int[] cursors = new int[glitchLength > 0 ? 2 : 1];
            int startOfGlitch = getSinePeriod();
            cursors[0] = startOfGlitch;
            if (glitchLength > 0) {
                cursors[1] = startOfGlitch + getGlitchLength();
            }
            mWaveformView.setCursorData(cursors);
            Log.i(TAG,"onGlitchDetected: glitch, numSamples = " + numSamples);
            mWaveformView.postInvalidate();
        }
    }
    @Override
    protected void maybeDisplayWaveform() {
        if (!mAutoScopeBox.isChecked()) return;
        long now = System.currentTimeMillis();
        if ((now - mLastDisplayTime) > MIN_DISPLAY_PERIOD_MILLIS) {
            mLastDisplayTime = now;
            int numSamples = getRecentSamples(mWaveform);
            mWaveformView.setSampleData(mWaveform, 0, numSamples);
            mWaveformView.setCursorData(null);
            mWaveformView.postInvalidate();
        }
    }

    private native int getGlitch(float[] mWaveform);
    private native int getRecentSamples(float[] mWaveform);

    public void onForceGlitchClicked(View view) {
        setForcedGlitchDuration(mForceGlitchesBox.isChecked() ? 100 : 0);
    }
}
