package com.example.mmdagent;

import android.app.NativeActivity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.lang.Thread;

public class MainNativeActivity extends NativeActivity implements Runnable
{
	static MainNativeActivity jni;
    private Thread mLooper;
    private Handler mHandler;
    
	static {
		System.loadLibrary("main");
	}
	
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        jni =new MainNativeActivity();
        
        mHandler =  new Handler(){
            //���b�Z�[�W��M
            public void handleMessage(Message message) {
            	String msgStr = (String) message.obj;
            	
                Log.d("MainNativeActivity", msgStr);
                //���b�Z�[�W�̎�ނɉ����ăC�x���g����
            };
        };
        Log.d("MainNativeActivity", "onCreate");
    }
    
    @Override
    public void onStart(){
        super.onStart();
        jni.onStartToJNI();
        
        mLooper = new Thread(this);
        //�X���b�h�������J�n
        if(mLooper != null ){
            mLooper.start();
        }
    }
     
    @Override
    public void onStop(){
        jni.onStopToJNI();
        //�X���b�h���폜
        mLooper = null;
        super.onStop();
    }
    
	@Override
	public void run() {
		Object resStr;
        while (mLooper != null) {
        	resStr = jni.receiveMessageFromJNI();
       		if ( resStr != null ){
            	//Message msg = Message.obtain();
                Message msg = mHandler.obtainMessage();
                msg.obj = resStr;
                //�n���h���ւ̃��b�Z�[�W���M
                mHandler.sendMessage(msg);
            }
        }
	}
    
	public void sendMessage(String setStr) {
    	jni.sendMessageToJNI(setStr);
	}
	
    public native int onStartToJNI();
    public native int onStopToJNI();
    public native Object receiveMessageFromJNI();
    public native int sendMessageToJNI(String str);
}
