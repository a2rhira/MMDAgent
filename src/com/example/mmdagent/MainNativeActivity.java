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
            //メッセージ受信
            public void handleMessage(Message message) {
            	String msgStr = (String) message.obj;
            	
                Log.d("MainNativeActivity", msgStr);
                //メッセージの種類に応じてイベント制御
            };
        };
        Log.d("MainNativeActivity", "onCreate");
    }
    
    @Override
    public void onStart(){
        super.onStart();
        jni.onStartToJNI();
        
        mLooper = new Thread(this);
        //スレッド処理を開始
        if(mLooper != null ){
            mLooper.start();
        }
    }
     
    @Override
    public void onStop(){
        jni.onStopToJNI();
        //スレッドを削除
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
                //ハンドラへのメッセージ送信
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
