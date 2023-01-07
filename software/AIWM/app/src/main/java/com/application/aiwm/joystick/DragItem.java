package com.application.aiwm.joystick;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;

import androidx.appcompat.widget.AppCompatImageView;

public class DragItem extends AppCompatImageView {
    public DragItem(Context c) {
        super(c);
        Log.e("DragItem", "DragItem()");
    }
    public DragItem(Context c, AttributeSet a) {
        super(c, a);
        Log.e("DragItem", "DragItem(x,x) " + Float.toString(getX()));
    }
    public DragItem(Context c, AttributeSet a, int defStyleAttr) {
        super(c, a, defStyleAttr);
        Log.e("DragItem", "DragItem(x,x,x) " + a.toString());
    }

    public void init() {
        Log.d("", "init() " + Float.toString(getX()));
    }

    /*@Override public boolean onTouchEvent(MotionEvent e) {
        super.onTouchEvent(e);

        if (!m_isInitStartPos) {
            m_startX = getX();
            m_startY = getY();
            m_isInitStartPos = true;
        }


        //Log.e("DragItem", "onTouchEvent " + Float.toString(getX()));

        float dx = m_startX - e.getX();
        float dy = m_startY - e.getY();

        float l = (float)Math.sqrt(dx * dx + dy * dy);

        Log.e("DragItem", "onTouchEvent: " + e.getX() + " " + e.getY() + " " + dx + " " + dy + " " + l);

        if (e.getAction() != MotionEvent.ACTION_UP) {
            if (l < 150) {
                setX(getX() + e.getX() - getWidth() / 2.0f);
                setY(getY() + e.getY() - getHeight() / 2.0f);
            }
            return true;
        } else {
            setX(m_startX);
            setY(m_startY);
            //performClick();
        }
        return false;
    }

    @Override public boolean performClick() {
        super.performClick();
        return true;
    }*/

    private float m_startX = 0.0f;
    private float m_startY = 0.0f;
    private boolean m_isInitStartPos = false;
}
