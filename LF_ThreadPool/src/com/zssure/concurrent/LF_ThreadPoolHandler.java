package com.zssure.concurrent;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class LF_ThreadPoolHandler implements LF_ThreadPool.Handler {

	public static final int PORT=1024;
	public Object controlMutex=new Object();
	public ServerSocket ss=null;
	public LF_ThreadPoolHandler()
	{
		try {
			ss=new ServerSocket(PORT);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	public void handler(Socket s)
	{
		try{
			//System.out.println(s.getLocalPort());
			//System.out.println(s.getReceiveBufferSize());
			//System.out.println(s.getSoLinger());
			System.out.println(s.getLocalPort()+"   "+s.getPort());
			Thread.sleep(2*1000);
		}catch(Exception e)
		{
			e.printStackTrace();
		}
		
	}
	public void run(LF_ThreadPool pool)
	{
		if(ss==null)
			return;
		Socket s=null;
		try{
			s=ss.accept();
			//System.out.println("handle socket -"+s.toString());
			pool.promoteNewLeader();
			handler(s);
		}catch(Exception e)
		{
			e.printStackTrace();
		}
	}
}
