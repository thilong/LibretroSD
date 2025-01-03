/*
 * Copyright 2017 The Android Open Source Project
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

import android.media.audiofx.Equalizer;
import android.media.audiofx.PresetReverb;
import android.util.Log;

import java.io.IOException;

abstract class TestOutputActivityBase extends TestAudioActivity {
    AudioOutputTester mAudioOutTester;

    private BufferSizeView mBufferSizeView;
    protected WorkloadView mWorkloadView;

    @Override boolean isOutput() { return true; }

    @Override
    protected void resetConfiguration() {
        super.resetConfiguration();
        mAudioOutTester.reset();
    }

    protected void findAudioCommon() {
        super.findAudioCommon();
        mBufferSizeView = (BufferSizeView) findViewById(R.id.buffer_size_view);
        mWorkloadView = (WorkloadView) findViewById(R.id.workload_view);
        if (mWorkloadView != null) {
            mWorkloadView.setWorkloadReceiver((w) -> mAudioOutTester.setWorkload(w));
        }
    }

    @Override
    public AudioOutputTester addAudioOutputTester() {
        return super.addAudioOutputTester();
    }

    @Override
    public void openAudio() throws IOException {
        super.openAudio();
        if (mBufferSizeView != null) {
            mBufferSizeView.onStreamOpened((OboeAudioStream) mAudioOutTester.getCurrentAudioStream());
        }
    }
}
