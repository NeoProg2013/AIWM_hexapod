package com.application.aiwm;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProvider;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.application.aiwm.joystick.SquareJoystick;

public class ControlActivity extends AppCompatActivity {

    ControlActivityViewModel m_viewModel = null;

    @Override protected void onCreate(Bundle savedInstanceState) {
        Log.e("ControlActivity", "call onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_control);

        m_viewModel = new ViewModelProvider(this).get(ControlActivityViewModel.class);

        SquareJoystick joystick = findViewById(R.id.squareJoystick);
        joystick.setRangeX(-1000, 1000);
        joystick.setRangeY(-110, 110);
        joystick.m_x.observe(this, new Observer<Float>() {
            @Override public void onChanged(Float v) {
                ((TextView)findViewById(R.id.textViewX)).setText(v.toString());
                m_viewModel.swlp.setCurvature(v);
                //Log.e("ControlActivity", "curvature = " + v);
            }
        });
        joystick.m_y.observe(this, new Observer<Float>() {
            @Override public void onChanged(Float v) {
                ((TextView)findViewById(R.id.textViewY)).setText(v.toString());
                m_viewModel.swlp.setDistance(v);
                //Log.e("ControlActivity", "call onChanged for m_y: " + v);
            }
        });

        findViewById(R.id.button3).setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View view) {
                m_viewModel.swlp.setHeight(-85);
            }
        });
    }

    @Override protected void onStart() {
        Log.e("ControlActivity", "call onStart");
        super.onStart();
    }

    @Override protected void onStop() {
        Log.e("ControlActivity", "call onStop");
        super.onStop();
    }

    @Override protected void onDestroy() {
        Log.e("ControlActivity", "call onDestroy");
        super.onDestroy();
    }
}