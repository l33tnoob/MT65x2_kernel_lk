package com.mediatek.bluetooth.map;

import java.util.ArrayList;

import android.bluetooth.BluetoothDevice;

import com.mediatek.bluetooth.map.cache.EventReport;

interface EventChannel {
		boolean sendReport(BluetoothDevice device, EventReport report);
}


public class EventReportManager implements Instance.Listener {	
	private ArrayList<Event>  mEventlist;

	private EventChannel mChannel;

	private boolean mChannelBusy = false;

	

	private class Event {
		BluetoothDevice mDevice;
		EventReport 	mReport;

		public Event(BluetoothDevice device, EventReport report) {
			mDevice = device;
			mReport = report;
		}
	}

	public EventReportManager(EventChannel channel)
	{
		mEventlist = new ArrayList<Event>();
		mChannel = channel;
	}

	public void reset()	{
		synchronized (mEventlist){
			mEventlist.clear();
		}
		mChannelBusy = false;
	}

	private void addEvent(BluetoothDevice device, EventReport event) {
		synchronized (mEventlist) {
			mEventlist.add(new Event(device, event));
		}
	}
	private Event retrieveEvent() {		
		synchronized (mEventlist) {
			if (mEventlist.size() > 0) {
				return mEventlist.remove(0); 
			} else {
				return null;
			}
		}
	}
	private void removeEvent(int masId) {
		int index;
		Event current;
		
		synchronized (mEventlist) {
			for (index = 0; index < mEventlist.size(); ) {
				current = mEventlist.get(index);
				if (current.mReport.match(masId))
				{
					mEventlist.remove(index);
				}
				else
				{
					index ++;
				}
			}
		}
	}

	public synchronized void retrieveAndSend()
	{
		if (mChannelBusy) 
		{
			return;
		}
		
		Event event = retrieveEvent();
		if (event != null &&
			mChannel != null &&
			mChannel.sendReport(event.mDevice, event.mReport) == true) {
			mChannelBusy = true;
		}		
	}	

	public void onInstanceChanged(BluetoothDevice device, EventReport report){
		addEvent(device,  report);
		retrieveAndSend();
	}

	public void onInstanceReportDisabled(int masId)
	{
		removeEvent(masId);
	}

	public void onEventSentCompleted() {
		synchronized(this)
		{
			mChannelBusy = false;
		}
		retrieveAndSend();
	}
	
}

