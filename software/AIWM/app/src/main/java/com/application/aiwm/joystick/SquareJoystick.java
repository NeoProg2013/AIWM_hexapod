package com.application.aiwm.joystick;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.ImageView;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.lifecycle.MutableLiveData;


public class SquareJoystick extends ConstraintLayout {
    public MutableLiveData<Float> m_x = new MutableLiveData<>();
    public MutableLiveData<Float> m_y = new MutableLiveData<>();
    private float m_maxX = 1.0f;
    private float m_minX = -1.0f;
    private float m_maxY = 1.0f;
    private float m_minY = -1.0f;

    public SquareJoystick(Context context) {
        super(context);
        init();
    }
    public SquareJoystick(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }
    public SquareJoystick(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }
    public SquareJoystick(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    public void init() {
        m_x.postValue(0.0f);
        m_y.postValue(0.0f);
    }

    public void setRangeX(float min, float max) {
        m_minX = min;
        m_maxX = max;
    }
    public void setRangeY(float min, float max) {
        m_minY = min;
        m_maxY = max;
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

            if (dx < -(layout_cx - handle_cx)) dx = -(layout_cx - handle_cx);
            if (dx > (layout_cx - handle_cx))  dx = (layout_cx - handle_cx);
            if (dy < -(layout_cy - handle_cy)) dy = -(layout_cy - handle_cy);
            if (dy > (layout_cy - handle_cy))  dy = (layout_cy - handle_cy);

            // Handle coordinates (screen system)
            float x = layout_cx - handle_cx + dx;
            float y = layout_cy - handle_cy + dy;

            handle.setX(x);
            handle.setY(y);
        } else {
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

        performClick();
        return true;
    }

    @Override public boolean performClick() {
        super.performClick();
        return true;
    }
}
