package com.application.aiwm;

import android.util.Log;

import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import com.application.aiwm.swlp.Swlp;

public class ControlActivityViewModel extends ViewModel {
    public Swlp swlp = new Swlp();


    public ControlActivityViewModel() {
        Log.e("CA_ViewModel", "call ctor");
        swlp.start();
    }
    @Override protected void onCleared() {
        Log.e("CA_ViewModel", "call onCleared");
        super.onCleared();

        swlp.stop();
    }
}
