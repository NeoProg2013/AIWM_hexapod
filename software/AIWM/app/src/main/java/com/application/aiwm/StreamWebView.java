package com.application.aiwm;

import android.content.Context;
import android.util.AttributeSet;
import android.webkit.WebView;

public class StreamWebView extends WebView {
    public StreamWebView(Context context) {
        super(context);
    }
    public StreamWebView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    public StreamWebView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

   @Override protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int desiredWidth = MeasureSpec.getSize(heightMeasureSpec);
        int desiredHeight = MeasureSpec.getSize(widthMeasureSpec);
        setTranslationY(desiredWidth);
        setMeasuredDimension(desiredWidth, desiredHeight);
    }
}