package com.application.aiwm;

import android.util.Log;

public class Swlp extends Thread {
    private static Swlp m_instance = null;
    public static Swlp getInstance() {
        if (m_instance == null) {
            Log.e("SWLP", "create instance");
            m_instance = new Swlp();
        }
        return m_instance;
    }

    @Override public void run() {
        try {
            Log.d("SWLP", "----->");
            while (true) {
                Log.d("SWLP", "LOOP");
                Thread.sleep(1000);
                if (this.isInterrupted()) {
                    break;
                }
            }
            Log.d("SWLP", "<-----");
        } catch (InterruptedException e) {
            Log.d("SWLP", "SocketException! " + e.toString());
        }
    }
}
