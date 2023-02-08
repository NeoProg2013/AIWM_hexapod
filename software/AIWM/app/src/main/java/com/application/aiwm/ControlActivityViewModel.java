package com.application.aiwm;


import androidx.lifecycle.ViewModel;

public class ControlActivityViewModel extends ViewModel {
    public Swlp swlp = new Swlp();

    public ControlActivityViewModel() {
        swlp.start();
    }
    @Override protected void onCleared() {
        super.onCleared();
        swlp.stop();
    }
}
