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
import android.widget.SeekBar;
import android.widget.TextView;

import com.application.aiwm.joystick.CircleJoystick;
import com.application.aiwm.joystick.SquareJoystick;
import com.google.android.material.bottomnavigation.BottomNavigationView;

import org.w3c.dom.Text;

public class ControlActivity extends AppCompatActivity implements
        BottomNavigationView.OnItemSelectedListener,
        View.OnClickListener,
        SeekBar.OnSeekBarChangeListener {

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
        findViewById(R.id.ctrlLayout).setVisibility(View.INVISIBLE);
        findViewById(R.id.diagLayout).setVisibility(View.INVISIBLE);

        //
        // MOTION
        //
        SquareJoystick joystickMotion = findViewById(R.id.joystickMotion);
        joystickMotion.setRange(-1000, 1000, -110, 110);
        joystickMotion.m_x.observe(this, v -> m_viewModel.swlp.setCurvature(Math.round(v)));
        joystickMotion.m_y.observe(this, v -> m_viewModel.swlp.setDistance(Math.round(v)));
        joystickMotion.setResetAfterMove(true);

        //
        // OFFSET
        //
        findViewById(R.id.buttonOffsetReset).setOnClickListener(this);
        SquareJoystick joystickOffset = findViewById(R.id.joystickOffset);
        joystickOffset.setRange(-150, 150, -150, 150);
        joystickOffset.m_x.observe(this, v -> m_viewModel.swlp.setSurfacePointX(Math.round(v)));
        joystickOffset.m_y.observe(this, v -> m_viewModel.swlp.setSurfacePointZ(Math.round(v)));
        joystickOffset.setResetAfterMove(false);

        //
        // ROTATE
        //
        findViewById(R.id.buttonRotateReset).setOnClickListener(this);
        CircleJoystick joystickRotate = findViewById(R.id.joystickRotate);
        joystickRotate.setRange(-15, 15, -15, 15);
        joystickRotate.m_x.observe(this, v -> m_viewModel.swlp.setSurfaceRotateX(Math.round(v)));
        joystickRotate.m_y.observe(this, v -> m_viewModel.swlp.setSurfaceRotateZ(Math.round(v)));
        joystickRotate.setResetAfterMove(false);

        //
        // DIAG
        //
        findViewById(R.id.buttonDiagUpdate).setOnClickListener(this);

        //
        // CTRL
        //
        ((SeekBar)findViewById(R.id.seekBarCtrlBodyHeight)).setOnSeekBarChangeListener(this);
        ((SeekBar)findViewById(R.id.seekBarCtrlMotionSpeed)).setOnSeekBarChangeListener(this);
        ((SeekBar)findViewById(R.id.seekBarCtrlStepHeight)).setOnSeekBarChangeListener(this);
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
        findViewById(R.id.ctrlLayout)  .setVisibility(item.getItemId() == R.id.menuCtrl   ? View.VISIBLE : View.INVISIBLE);
        findViewById(R.id.diagLayout)  .setVisibility(item.getItemId() == R.id.menuDiag   ? View.VISIBLE : View.INVISIBLE);
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

    @Override public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
        if (seekBar.getId() == R.id.seekBarCtrlBodyHeight) {
            ((TextView)findViewById(R.id.textViewCtrlBodyHeight)).setText("Body height (" + i + ")");
            m_viewModel.swlp.setSurfacePointY(i);
        }
        else if (seekBar.getId() == R.id.seekBarCtrlMotionSpeed) {
            ((TextView)findViewById(R.id.textViewCtrlMotionSpeed)).setText("Motion speed (" + i + ")");
            m_viewModel.swlp.setMotionSpeed(i);
        }
        else if (seekBar.getId() == R.id.seekBarCtrlStepHeight) {
            ((TextView)findViewById(R.id.textViewCtrlStepHeight)).setText("Step height (" + i + ")");
            m_viewModel.swlp.setStepHeight(i);
        }
    }
    @Override public void onStartTrackingTouch(SeekBar seekBar) {}
    @Override public void onStopTrackingTouch(SeekBar seekBar) {}
}