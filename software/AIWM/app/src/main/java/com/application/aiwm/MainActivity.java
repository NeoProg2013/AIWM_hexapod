package com.application.aiwm;

import androidx.appcompat.app.AppCompatActivity;
//import androidx.databinding.DataBindingUtil;

//import com.application.aiwm.databinding.ActivityMainBinding;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    //ActivityMainBinding binding;

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.e("MainActivity", "onCreate");
        setContentView(R.layout.activity_main);

        findViewById(R.id.settingsBtn).setOnClickListener(this);
        findViewById(R.id.connectBtn).setOnClickListener(this);
        ((TextView)findViewById(R.id.textViewVersion)).setText(BuildConfig.VERSION_NAME);
    }

    @Override protected void onDestroy() {
        super.onDestroy();
        Log.e("MainActivity", "onDestroy");
    }

    @Override public void onClick(View view) {
        Log.e("MainActivity", "call onClick " + view.getId());
        if (view.getId() == R.id.settingsBtn) {
            Intent intent = new Intent(this, SettingsActivity.class);
            startActivity(intent);
        }
        else if (view.getId() == R.id.connectBtn) {
            Intent intent = new Intent(this, ControlActivity.class);
            startActivity(intent);
        }
    }
}