package com.application.aiwm;

import android.util.Log;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

public class Swlp extends Thread {
    private final static String SERVER_IP_ADDRESS = "111.111.111.111";
    private final static int SERVER_PORT = 3333;
    private DatagramSocket m_socket = null;


    /*private static Swlp m_instance = null;
    public static Swlp getInstance() {
        if (m_instance == null) {
            Log.e("SWLP", "create instance");
            m_instance = new Swlp();
        }
        return m_instance;
    }*/

    volatile private int m_curvature = 0;
    volatile private int m_distance = 0;
    volatile private int m_stepHeight = 0;

    volatile private int m_height = 0;

    public void setCurvature(float v) {
        m_curvature = Math.round(v);
    }
    public void setDistance(float v) {
        m_distance = Math.round(v);
    }
    public void setStepHeight(float v) {
        m_stepHeight = Math.round(v);
    }
    public void setHeight(float v) {
        m_height = Math.round(v);
    }

    @Override public void run() {
        byte[] recvBuffer = new byte[128];
        try {
            Log.e("SWLP", "----->");
            m_socket = new DatagramSocket(SERVER_PORT);

            DatagramPacket inPacket = new DatagramPacket(recvBuffer, recvBuffer.length);


            while (true) {
                Log.e("SWLP", String.format("LOOP: %d %d %d", m_curvature, m_distance, m_height));

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
                frame[14] = (byte)((m_height >> 0) & 0xFF); // surface_point_y
                frame[15] = (byte)((m_height >> 8) & 0xFF);

                int crc = calculateChecksum(frame);
                Log.e("SWLP", String.format("CRC: %d %x", frame.length, crc));
                frame[30] = (byte)((crc & 0x00FF) >> 0);
                frame[31] = (byte)((crc & 0xFF00) >> 8);

                DatagramPacket outPacket = new DatagramPacket(frame, frame.length, InetAddress.getByName("111.111.111.111"), 3333);
                m_socket.send(outPacket);

                Thread.sleep(1000);
                if (this.isInterrupted()) {
                    break;
                }
            }
            Log.e("SWLP", "<-----");
        } catch (SocketException e) {
            Log.e("SWLP", "SocketException! " + e);
        } catch (UnknownHostException e) {
            Log.e("SWLP", "UnknownHostException! " + e);
        } catch (IOException e) {
            Log.e("SWLP", "IOException! " + e);
        } catch (InterruptedException e) {
            Log.e("SWLP", "InterruptedException! " + e);
        }
    }

    @Override public void interrupt() {
        super.interrupt();
        Log.e("SWLP", "call interrupt");
    }

    private int calculateChecksum(byte[] frame) {
        int checksum = 0;
        for (int i = 0; i < frame.length - 2; ++i) {
            checksum += (int)(frame[i] & 0xFF);
        }
        return checksum & 0xFFFF;
    }
}
