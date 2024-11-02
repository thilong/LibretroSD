package com.swordfish.libretrodroid;

import android.os.Bundle;
import android.view.ViewGroup;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.aidoo.retrorunner.RetroRunnerView;

public class RetroRunnerActivity extends AppCompatActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        RetroRunnerView retroRunnerView = new RetroRunnerView(this);
        retroRunnerView.setLayoutParams(new ViewGroup.LayoutParams(-1, -1));
        setContentView(retroRunnerView);
    }
}
