package com.zssure.concurrent.client;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.Socket;

public class LF_ThreadPoolClient {

	private static final int PORT=1024;
	private static final String IP_ARRD="localhost";
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		try{
			for(int i=0;i<15;++i)
			{
				Socket client=new Socket(IP_ARRD,PORT);
				DataOutputStream out = new DataOutputStream(client.getOutputStream());    
                String str = "client:"+i;
                out.writeUTF(str);
                out.close();
			}
			
		}catch(Exception e)
		{
			e.printStackTrace();
		}

	}

}
