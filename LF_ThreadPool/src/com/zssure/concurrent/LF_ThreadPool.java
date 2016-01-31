package com.zssure.concurrent;

import java.sql.Time;

import javax.print.attribute.standard.DateTimeAtCompleted;

public class LF_ThreadPool
{
   
   // Attributes ----------------------------------------------------
   private final Handler handler;
   private boolean shutdown = false;
   private Thread leader = null;
   private Object mutex = new Object();
   private int waiting = 0;
   private int running = 0;
   private int maxRunning = 0;
   private int maxWaiting = -1;
   private final int instNo = ++instCount;
   private int threadNo = 0;
   private final String name;
   
   // Static --------------------------------------------------------
   private static int instCount = 0;
   
   // Constructors --------------------------------------------------
   public LF_ThreadPool(Handler handler, String name) {
      if (handler == null)
         throw new NullPointerException();
      
      this.handler = handler;
      this.name = name;
   }
   
   // Public --------------------------------------------------------
   public int waiting()
   {
      return waiting;
   }
   
   public int running()
   {
      return running;
   }
   
   public boolean isShutdown()
   {
      return shutdown;
   }
   
   public int getMaxRunning()
   {
      return maxRunning;
   }
   
   public void setMaxRunning(int maxRunning)
   {
      if (maxRunning < 0)
         throw new IllegalArgumentException("maxRunning: " + maxRunning);
            
      this.maxRunning = maxRunning;
   }
   
   public int getMaxWaiting()
   {
      return maxWaiting;
   }
   
   public void setMaxWaiting(int maxWaiting)
   {
      if (maxWaiting < -1)
         throw new IllegalArgumentException("maxWaiting: " + maxWaiting);
            
      this.maxWaiting = maxWaiting;
   }
   
   public String toString()
   {
      return "LF_ThreadPool-" + instNo + "[leader:"
            + (leader == null ? "null" : leader.getName())
            + ", waiting:" + waiting
            + ", running: " + running + "(" + maxRunning
            + "), shutdown: " + shutdown + "]";
   }
   
   public void join()
   {
      System.out.println("Thread: " + Thread.currentThread().getName() + " JOIN ThreadPool " + name+Thread.currentThread().getId());
      try {
	      while (!shutdown && (running == 0 || maxWaiting == -1 || waiting < maxWaiting)
	              && (maxRunning == 0 || (waiting + running) < maxRunning))
	      {
	    	 System.out.println("Outer While in JOIN  "+Thread.currentThread().getName()+"running is "+running+" waiting is "+waiting);
	         synchronized (mutex)
	         {
	            while (leader != null)
	            {
	                  System.out.println("" + this + " - "
	                     + Thread.currentThread().getName() + " enter wait()");
	               ++waiting;
	               try { mutex.wait(); }
	               catch (InterruptedException ie)
	               {
	                  System.out.println(ie.getMessage());
	               }
	               finally { --waiting; }
	               System.out.println("" + this + " - "
	                     + Thread.currentThread().getName() + " awaked");
	            }
	            if (shutdown)
	               return;
	
	            leader = Thread.currentThread();
	            System.out.println("" + this + " - New Leader"); 
	            ++running;
	         }
	         try {  
	            do {
	               handler.run(this);
	            } while (!shutdown && leader == Thread.currentThread());
	         } catch (Throwable th) {
	        	 System.out.println("Exception thrown in " + Thread.currentThread().getName()+th.getMessage());
	            shutdown();
	         } finally { synchronized (mutex) { --running; } }
	      }
      } finally {
    	  System.out.println("Thread: " + Thread.currentThread().getName() + " LEFT ThreadPool " + name+" running "+running+" waiting is "+waiting);
      }
   }
   
   public boolean promoteNewLeader()
   {
      if (shutdown)
         return false;
      
      // only the current leader can promote the next leader
      if (leader != Thread.currentThread())
         throw new IllegalStateException();
      
      leader = null;
      
      // notify (one) waiting thread in join()
      synchronized (mutex) {
         if (waiting > 0)
         {
        	 System.out.println("" + this + " - promote new leader by notify"); 
            mutex.notify();
            return true;
         }
      }
            
      // if there is no waiting thread,
      // and the maximum number of running threads is not yet reached,
      if (maxRunning != 0 && running >= maxRunning) {
    	  System.out.println("" + this + " - Max number of threads reached"); 
         return false;
      }
      
      // start a new one
      System.out.println("" + this + " - promote new leader by add new Thread");
      addThread(
         new Runnable() {
            public void run() { join(); }
         }
      );
      
      return true;
   }
   
   public void shutdown() {
	   System.out.println("" + this + " - shutdown"); 
      shutdown = true;
      leader = null;
      synchronized (mutex)
      {
         mutex.notifyAll();
      }
   }
         
   // Package protected ---------------------------------------------
   
   // Protected -----------------------------------------------------
   // may be overloaded to take new thread from convential thread pool
   protected void addThread(Runnable r) {
       new Thread(r, name + "-" + (++threadNo)).start();
   }
   
   // Private -------------------------------------------------------
   
   // Inner classes -------------------------------------------------
   public interface Handler {
      void run(LF_ThreadPool pool);
   }
}