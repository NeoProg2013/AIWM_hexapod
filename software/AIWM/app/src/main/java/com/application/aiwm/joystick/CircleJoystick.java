package com.application.aiwm.joystick;

import android.content.Context;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.ImageView;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.lifecycle.MutableLiveData;

public class CircleJoystick extends ConstraintLayout {
    public final MutableLiveData<Float> m_x = new MutableLiveData<>();
    public final MutableLiveData<Float> m_y = new MutableLiveData<>();
    private float m_maxX = 1.0f;
    private float m_minX = -1.0f;
    private float m_maxY = 1.0f;
    private float m_minY = -1.0f;
    private boolean m_isResetAfterMove = true;

    public CircleJoystick(Context context) {
        super(context);
        init();
    }
    public CircleJoystick(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }
    public CircleJoystick(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }
    public CircleJoystick(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    public void init() {
        m_x.postValue(0.0f);
        m_y.postValue(0.0f);
    }

    public void setRange(float minX, float maxX, float minY, float maxY) {
        m_minX = minX;
        m_maxX = maxX;
        m_minY = minY;
        m_maxY = maxY;
    }
    public void setResetAfterMove(boolean v) {
        m_isResetAfterMove = v;
    }
    public void resetHandlePosition() {
        ImageView handle = (ImageView)getChildAt(0);
        handle.setX(getWidth() / 2.0f - handle.getWidth() / 2.0f);
        handle.setY(getHeight() / 2.0f - handle.getHeight() / 2.0f);

        MotionEvent touchEvent = MotionEvent.obtain(
            SystemClock.uptimeMillis(),
            SystemClock.uptimeMillis() + 100,
            MotionEvent.ACTION_UP,
            handle.getX(),
            handle.getY(),
            0
        );
        this.dispatchTouchEvent(touchEvent);
    }

    @Override public boolean onTouchEvent(MotionEvent e) {
        ImageView handle = (ImageView)getChildAt(0);

        // Layout center
        float layout_cx = getWidth() / 2.0f;
        float layout_cy = getHeight() / 2.0f;

        // Handle center
        float handle_cx = handle.getWidth() / 2.0f;
        float handle_cy = handle.getHeight() / 2.0f;

        if (e.getAction() != MotionEvent.ACTION_UP) {
            // Distance from layout center -- [0;0]
            float dx = e.getX() - layout_cx;
            float dy = e.getY() - layout_cy;

            // Vector by 45 degree
            final float v45_x = (float)(getWidth() / 2.0f * Math.cos(Math.PI / 4.0f));
            final float v45_y = (float)(getWidth() / 2.0f * Math.cos(Math.PI / 4.0f));
            final float v45_len = (float)Math.sqrt(v45_x * v45_x + v45_y + v45_y);

            // Constrain vector
            float v_len = (float)Math.sqrt(dx * dx + dy * dy);
            if (v_len > v45_len) {
                float k = v45_len / v_len;
                dx = dx * k;
                dy = dy * k;
            }

            // Handle coordinates (screen system)
            float x = layout_cx - handle_cx + dx;
            float y = layout_cy - handle_cy + dy;

            handle.setX(x);
            handle.setY(y);
        } else if (m_isResetAfterMove) {
            handle.setX(layout_cx - handle_cx);
            handle.setY(layout_cy - handle_cy);
        }

        // Scale coordinates to [-1; 1]
        float x = handle.getX() - (layout_cx - handle_cx);
        float y = (layout_cy - handle_cy) - handle.getY();
        x = x / (layout_cx - handle_cx);
        y = y / (layout_cy - handle_cy);

        // Scale coordinates from [-1; 1] to [0; 1]
        x = (x + 1.0f) / 2.0f;
        y = (y + 1.0f) / 2.0f;

        // Scale coordinates from [0; 1] to [m_min; m_max]
        x = x * (m_maxX - m_minX) + m_minX;
        y = y * (m_maxY - m_minY) + m_minY;

        m_x.postValue(x);
        m_y.postValue(y);

        Log.e("Joystick", m_x.getValue().toString() + " " + m_y.getValue().toString());

        performClick();
        return true;
    }

    @Override public boolean performClick() {
        super.performClick();
        return true;
    }
}
