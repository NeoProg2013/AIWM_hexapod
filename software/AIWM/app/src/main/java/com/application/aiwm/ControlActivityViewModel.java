package com.application.aiwm;

import android.util.Log;

import androidx.lifecycle.ViewModel;

public class ControlActivityViewModel extends ViewModel {
    private Swlp m_swlp = null;

    public ControlActivityViewModel() {
        Log.e("CA_ViewModel", "call ctor");
        m_swlp = new Swlp();
        m_swlp.start();
    }

    @Override protected void onCleared() {
        Log.e("CA_ViewModel", "call onCleared");
        super.onCleared();
        m_swlp.interrupt();
    }
}
