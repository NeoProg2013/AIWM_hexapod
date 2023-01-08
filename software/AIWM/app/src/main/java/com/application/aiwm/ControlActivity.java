package com.application.aiwm;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.lifecycle.ViewModelProvider;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

import com.google.android.material.bottomnavigation.BottomNavigationView;

import fragments.CtrlFragment;
import fragments.DiagFragment;
import fragments.MotionFragment;
import fragments.OffsetFragment;
import fragments.RotateFragment;

public class ControlActivity extends AppCompatActivity implements BottomNavigationView.OnItemSelectedListener {

    private ControlActivityViewModel m_viewModel = null;
    private Fragment m_motionFragment = null;
    private Fragment m_ctrlFragment = null;
    private Fragment m_offsetFragment = null;
    private Fragment m_rotateFragment = null;
    private Fragment m_diagFragment = null;

    @Override protected void onCreate(Bundle savedInstanceState) {
        Log.e("ControlActivity", "call onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_control);

        m_viewModel = new ViewModelProvider(this).get(ControlActivityViewModel.class);

        m_viewModel.swlp.getRxPacketsCounter().observe(this, (v) -> {
            ((TextView)findViewById(R.id.textViewRx)).setText("RX: " + v.toString());
        });
        m_viewModel.swlp.getTxPacketsCounter().observe(this, (v) -> {
            ((TextView)findViewById(R.id.textViewTx)).setText("TX: " + v.toString());
        });

        m_viewModel.swlp.getSystemStatus().observe(this, (v) -> {
            if (v != 0) {
                findViewById(R.id.imageViewWarning).setVisibility(View.VISIBLE);
            }
        });
        m_viewModel.swlp.getModuleStatus().observe(this, (v) -> {
            if (v != 0) {
                findViewById(R.id.imageViewWarning).setVisibility(View.VISIBLE);
            }
        });

        m_motionFragment = new MotionFragment();
        m_ctrlFragment = new CtrlFragment();
        m_offsetFragment = new OffsetFragment();
        m_rotateFragment = new RotateFragment();
        m_diagFragment = new DiagFragment();


        BottomNavigationView navView = findViewById(R.id.bottomNavigationView);
        navView.setOnItemSelectedListener(this);
    }

    @Override public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        Log.e("ControlActivity", "call onOptionsItemSelected");
        return super.onOptionsItemSelected(item);
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

    @Override public boolean onNavigationItemSelected(MenuItem item) {
        FragmentManager fm = getSupportFragmentManager();
        if (item.getItemId() == R.id.menuMotion) {
            fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_motionFragment).commit();
        }
        if (item.getItemId() == R.id.menuCtrl) {
            fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_ctrlFragment).commit();
        }
        if (item.getItemId() == R.id.menuOffet) {
            fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_offsetFragment).commit();
        }
        if (item.getItemId() == R.id.menuRotate) {
            fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_rotateFragment).commit();
        }
        if (item.getItemId() == R.id.menuDiag) {
            fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_diagFragment).commit();
        }
        return true;
    }
}