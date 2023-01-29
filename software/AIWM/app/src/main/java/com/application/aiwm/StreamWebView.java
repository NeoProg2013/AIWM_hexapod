package com.application.aiwm;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.webkit.WebView;

public class StreamWebView extends WebView {

    public StreamWebView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override public void draw(Canvas canvas) {
        canvas.translate(0, getWidth());
        canvas.rotate(-90);
        canvas.clipRect(0, 0, getWidth(), getHeight(), android.graphics.Region.Op.REPLACE);
        super.draw(canvas);
    }
}