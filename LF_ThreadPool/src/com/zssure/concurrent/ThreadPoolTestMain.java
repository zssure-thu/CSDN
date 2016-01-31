package com.zssure.concurrent;

import java.io.IOException;

public class ThreadPoolTestMain {

	public static final LF_ThreadPoolHandler handler=new LF_ThreadPoolHandler();
	public LF_ThreadPool pool=new LF_ThreadPool(ThreadPoolTestMain.handler,"Main");
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		ThreadPoolTestMain main=new ThreadPoolTestMain();
		main.initLF_ThreadPool();
		for(int i=0;i<20;++i)
		{
			new Thread(new Runnable(){
			@Override
			public void run()
			{
				main.pool.join();
			}
			}).start();
		}
		try {
			System.in.read();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private void initLF_ThreadPool()
	{
		pool.setMaxRunning(5);
		pool.setMaxWaiting(3);
	}

}
