package com.application.aiwm;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

public class SettingsActivity extends AppCompatActivity implements View.OnClickListener {

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.e("SettingsActivity", "onCreate");
        setContentView(R.layout.activity_settings);

        findViewById(R.id.button2).setOnClickListener(this);
    }

    @Override protected void onDestroy() {
        super.onDestroy();
        Log.e("SettingsActivity", "onDestroy");
    }

    @Override public void onClick(View view) {
        Log.e("SettingsActivity", "call onClick " + view.getId());
        Intent intent = new Intent(this, MainActivity.class);
        startActivity(intent);
    }
}