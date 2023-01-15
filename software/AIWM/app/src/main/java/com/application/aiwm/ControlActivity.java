package com.application.aiwm;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.ViewModelProvider;

import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.application.aiwm.joystick.CircleJoystick;
import com.application.aiwm.joystick.SquareJoystick;
import com.google.android.material.bottomnavigation.BottomNavigationView;

public class ControlActivity extends AppCompatActivity implements BottomNavigationView.OnItemSelectedListener, View.OnClickListener {

    private ControlActivityViewModel m_viewModel = null;
    final private String[] m_systemErrors = {
            "- Критическая ошибка",
            "- Внутренняя ошибка",
            "- Низкий заряд АКБ",
            "- Ошибка синхронизации",
            "- Математическая ошибка",
            "- Ошибка I2C шины",
    };
    final private String[] m_moduleErrors = {
            "- Сбой ядра передвижения",
            "- Сбой драйвера сервоприводов",
            "- Сбой подсистемы мониторинга",
            "- Сбой драйвера дисплея",
            "- Сбой подсистемы ориентации",
            "- Сбой подсистемы сенсоров",
    };

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

        //
        // MAIN
        //
        BottomNavigationView navView = findViewById(R.id.bottomNavigationView);
        navView.setOnItemSelectedListener(this);

        findViewById(R.id.motionLayout).setVisibility(View.VISIBLE);
        findViewById(R.id.offsetLayout).setVisibility(View.INVISIBLE);
        findViewById(R.id.rotateLayout).setVisibility(View.INVISIBLE);
        findViewById(R.id.diagLayout).setVisibility(View.INVISIBLE);

        //
        // MOTION
        //
        SquareJoystick joystickMotion = findViewById(R.id.joystickMotion);
        joystickMotion.setRange(-1000, 1000, -110, 110);
        joystickMotion.m_x.observe(this, v -> {
            ((TextView)findViewById(R.id.motionTextViewX)).setText(v.toString());
            m_viewModel.swlp.setCurvature(v);
        });
        joystickMotion.m_y.observe(this, v -> {
            ((TextView)findViewById(R.id.motionTextViewY)).setText(v.toString());
            m_viewModel.swlp.setDistance(v);
        });
        joystickMotion.setResetAfterMove(true);

        //
        // OFFSET
        //
        findViewById(R.id.buttonOffsetReset).setOnClickListener(this);

        SquareJoystick joystickOffset = findViewById(R.id.joystickOffset);
        joystickOffset.setRange(-150, 150, -150, 150);
        joystickOffset.m_x.observe(this, v -> {
            ((TextView)findViewById(R.id.offsetTextViewX)).setText(v.toString());
            //m_viewModel.swlp.setCurvature(v);
        });
        joystickOffset.m_y.observe(this, v -> {
            ((TextView)findViewById(R.id.offsetTextViewY)).setText(v.toString());
            //m_viewModel.swlp.setDistance(v);
        });
        joystickOffset.setResetAfterMove(false);

        //
        // ROTATE
        //
        findViewById(R.id.buttonRotateReset).setOnClickListener(this);

        CircleJoystick joystickRotate = findViewById(R.id.joystickRotate);
        joystickRotate.setRange(-15, 15, -15, 15);
        joystickRotate.m_x.observe(this, v -> {
            ((TextView)findViewById(R.id.rotateTextViewX)).setText(v.toString());
            //m_viewModel.swlp.setCurvature(v);
        });
        joystickRotate.m_y.observe(this, v -> {
            ((TextView)findViewById(R.id.rotateTextViewY)).setText(v.toString());
            //m_viewModel.swlp.setDistance(v);
        });
        joystickRotate.setResetAfterMove(false);

        //
        // DIAG
        //
        findViewById(R.id.buttonDiagUpdate).setOnClickListener(this);
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
        findViewById(R.id.motionLayout).setVisibility(item.getItemId() == R.id.menuMotion ? View.VISIBLE : View.INVISIBLE);
        findViewById(R.id.offsetLayout).setVisibility(item.getItemId() == R.id.menuOffet  ? View.VISIBLE : View.INVISIBLE);
        findViewById(R.id.rotateLayout).setVisibility(item.getItemId() == R.id.menuRotate ? View.VISIBLE : View.INVISIBLE);
        findViewById(R.id.diagLayout)  .setVisibility(item.getItemId() == R.id.menuDiag   ? View.VISIBLE : View.INVISIBLE);

            //fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_motionFragment).commit();
        /*}
        if (item.getItemId() == R.id.menuCtrl) {
            //fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_ctrlFragment).commit();
        }
        if (item.getItemId() == R.id.menuOffet) {
            findViewById(R.id.motionLayout).setVisibility(View.INVISIBLE);
            findViewById(R.id.offsetLayout).setVisibility(View.VISIBLE);
            findViewById(R.id.rotateLayout).setVisibility(View.INVISIBLE);
            //fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_offsetFragment).commit();
        }
        if (item.getItemId() == R.id.menuRotate) {
            //fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_rotateFragment).commit();
        }
        if (item.getItemId() == R.id.menuDiag) {
            //fm.beginTransaction().replace(R.id.controlFragmentContainerView, m_diagFragment).commit();
        }*/
        return true;
    }

    @Override public void onClick(View view) {
        if (view.getId() == R.id.buttonOffsetReset) {
            ((SquareJoystick)findViewById(R.id.joystickOffset)).resetHandlePosition();
        }
        else if (view.getId() == R.id.buttonRotateReset) {
            ((CircleJoystick)findViewById(R.id.joystickRotate)).resetHandlePosition();
        }
        else if (view.getId() == R.id.buttonDiagUpdate) {
            LinearLayout l = findViewById(R.id.layoutDiagSystemErrors);
            l.removeAllViews();

            int systemStatus = m_viewModel.swlp.getModuleStatus().getValue();
            for (int i = 0; i < m_systemErrors.length; ++i) {
                int mask = 0x01 << i;
                if ((systemStatus & mask) == mask) {
                    TextView t = new TextView(this);
                    t.setText(m_systemErrors[i]);
                    t.setTextColor(Color.RED);
                    l.addView(t);
                }
            }


            l = findViewById(R.id.layoutDiagModuleErrors);
            l.removeAllViews();

            int moduleStatus = m_viewModel.swlp.getModuleStatus().getValue();
            for (int i = 0; i < m_moduleErrors.length; ++i) {
                int mask = 0x01 << i;
                if ((moduleStatus & mask) == mask) {
                    TextView t = new TextView(this);
                    t.setText(m_moduleErrors[i]);
                    t.setTextColor(Color.RED);
                    l.addView(t);
                }
            }
        }
    }
}