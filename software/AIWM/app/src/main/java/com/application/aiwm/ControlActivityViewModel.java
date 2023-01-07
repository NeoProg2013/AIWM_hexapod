package com.application.aiwm;

import android.util.Log;

import androidx.lifecycle.ViewModel;

public class ControlActivityViewModel extends ViewModel {
    public Swlp swlp = null;

    public ControlActivityViewModel() {
        Log.e("CA_ViewModel", "call ctor");
        swlp = new Swlp();
        swlp.start();
    }

    public void setCurvature(float v) {
        swlp.setCurvature(v);
    }

    public void setDistance(float v) {
        swlp.setDistance(v);
    }

    @Override protected void onCleared() {
        Log.e("CA_ViewModel", "call onCleared");
        super.onCleared();
        swlp.interrupt();
    }
}
