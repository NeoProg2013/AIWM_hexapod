package com.application.aiwm.swlp;

import android.util.Log;

import androidx.lifecycle.MutableLiveData;

import java.net.*;
import java.util.Random;

public class Swlp {
    private final static String SERVER_IP_ADDRESS = "111.111.111.111";
    private final static int SERVER_PORT = 3333;

    private DatagramSocket m_socket = null;
    private Thread m_recvThread = null;
    private Thread m_sendThread = null;
    private MutableLiveData<Integer> m_rxCountLive = new MutableLiveData<>(0);
    private MutableLiveData<Integer> m_txCountLive = new MutableLiveData<>(0);

    private MutableLiveData<Integer> m_systemStatus = new MutableLiveData<>(0);
    private MutableLiveData<Integer> m_moduleStatus = new MutableLiveData<>(0);

    private int m_rxCount = 0;
    private int m_txCount = 0;

    // Request
    volatile private int m_curvature = 0;
    volatile private int m_distance = 0;
    volatile private int m_stepHeight = 0;
    volatile private int m_motionCtrl = 0;
    volatile private int m_surfacePointX = 0;
    volatile private int m_surfacePointY = 0;
    volatile private int m_surfacePointZ = 0;
    volatile private int m_surfaceRotateX = 0;
    volatile private int m_surfaceRotateY = 0;
    volatile private int m_surfaceRotateZ = 0;


    public void setCurvature(float v) {
        m_curvature = Math.round(v);
    }
    public void setDistance(float v) {
        m_distance = Math.round(v);
    }
    public void setStepHeight(float v) {
        m_stepHeight = Math.round(v);
    }
    public void setSurfacePointY(float v) {
        m_surfacePointY = Math.round(v);
    }

    public MutableLiveData<Integer> getRxPacketsCounter() { return m_rxCountLive; }
    public MutableLiveData<Integer> getTxPacketsCounter() { return m_txCountLive; }
    public MutableLiveData<Integer> getSystemStatus() { return m_systemStatus; }
    public MutableLiveData<Integer> getModuleStatus() { return m_moduleStatus; }

    public void start() {
        Log.e("SWLP", "call start()");
        try {
            m_socket = new DatagramSocket(SERVER_PORT);
        } catch (Exception e) {
            Log.e("SWLP", "Exception! " + e);
        }

        m_recvThread = new Thread(() -> {
            recvResponseLoop();
            Log.e("SWLP", "recv thread stopped");
        });
        m_sendThread = new Thread(() -> {
            sendRequestLoop();
            Log.e("SWLP", "send thread stopped");
        });

        m_recvThread.start();
        m_sendThread.start();
    }

    public void stop() {
        Log.e("SWLP", "call stop()");
        m_socket.close();
        m_recvThread.interrupt();
        m_sendThread.interrupt();

        try {
            m_recvThread.join(1000);
            m_sendThread.join(1000);
        }
        catch (Exception e) {
            Log.e("SWLP", "exception: " + e);
        }
        m_recvThread = null;
        m_sendThread = null;
    }

    private void sendRequestLoop() {
        Random rand = new Random();
        try {
            while (true) {
                Log.e("SWLP", String.format("send: %d %d %d", m_curvature, m_distance, m_surfacePointY));

                byte[] frame = new byte[32];
                // Start mark (0xAABBCCDD)
                frame[0] = (byte)0xDD;
                frame[1] = (byte)0xCC;
                frame[2] = (byte)0xBB;
                frame[3] = (byte)0xAA;
                // Version (0x04)
                frame[4] = (byte)0x04;
                // swlp_request_t
                frame[5] = 0;
                frame[6] = (byte)((m_curvature >> 0) & 0xFF);
                frame[7] = (byte)((m_curvature >> 8) & 0xFF);
                frame[8] = (byte)(m_distance & 0xFF);
                frame[9] = (byte)(m_stepHeight & 0xFF);
                frame[10] = 0; // motion_ctrl
                frame[11] = 0;
                frame[12] = 0; // surface_point_x
                frame[13] = 0;
                frame[14] = (byte)((m_surfacePointY >> 0) & 0xFF); // surface_point_y
                frame[15] = (byte)((m_surfacePointY >> 8) & 0xFF);

                int crc = calculateChecksum(frame);
                frame[30] = (byte)((crc >> 0) & 0xFF);
                frame[31] = (byte)((crc >> 8) & 0xFF);

                DatagramPacket outPacket = new DatagramPacket(frame, frame.length, InetAddress.getByName(SERVER_IP_ADDRESS), 3333);
                m_socket.send(outPacket);

                ++m_txCount;
                m_txCountLive.postValue(m_txCount);

                m_systemStatus.postValue(rand.nextInt(255));
                m_moduleStatus.postValue(rand.nextInt(255));

                Thread.sleep(1000);
            }
        }
        catch (Exception e) {
            Log.e("SWLP", "send thread exception: " + e);
        }
    }

    private void recvResponseLoop() {
        try {
            while (true) {
                byte[] recvBuffer = new byte[128];
                DatagramPacket inPacket = new DatagramPacket(recvBuffer, recvBuffer.length);
                m_socket.receive(inPacket);
                Log.e("SWLP", "RECV! " + inPacket.getLength());

                ++m_rxCount;
                m_rxCountLive.postValue(m_rxCount);
            }
        }
        catch (Exception e) {
            Log.e("SWLP", "recv thread exception: " + e);
        }
    }

    private int calculateChecksum(byte[] frame) {
        int checksum = 0;
        for (int i = 0; i < frame.length - 2; ++i) {
            checksum += (int)(frame[i] & 0xFF);
        }
        return checksum & 0xFFFF;
    }
}