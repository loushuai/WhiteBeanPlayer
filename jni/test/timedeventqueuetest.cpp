/*
 * timedeventqueuetest.cpp
 *
 *  Created on: 2016Äê5ÔÂ8ÈÕ
 *      Author: loushuai
 */

#include <catch.hpp>
#include <iostream>
#include <string>
#include "TimedEventQueue.h"

using namespace std;

namespace bigbean
{

TEST_CASE("TimedEventQueue")
{
	TimedEventQueue eventQueue;
	eventQueue.start();

	struct TestEvent : public TimedEventQueue::Event {
		TestEvent(string str): msg(str) {}

		virtual void fire(TimedEventQueue *queue, int64_t now_us) {
			cout << msg << endl;
		}

		string msg;
	};

	SECTION("Stop")
	{
		eventQueue.stop(false);
	}

	SECTION("PostEvent")
	{
	    eventQueue.postEvent(shared_ptr<TimedEventQueue::Event> (new TestEvent("PostEvent")));
	    eventQueue.stop(true);
	}

	SECTION("PostEventWithDelay")
	{
	    eventQueue.postEventWithDelay(shared_ptr<TimedEventQueue::Event> (new TestEvent("PostEventWithDelay")), 2000000);
	    eventQueue.stop(true);
	}

	SECTION("CancelEvent")
	{
	    eventQueue.postEventWithDelay(shared_ptr<TimedEventQueue::Event> (new TestEvent("CancelEvent")), 2000000);
	    eventQueue.cancelEvent(1);
	    eventQueue.stop(true);
	}
}


}
