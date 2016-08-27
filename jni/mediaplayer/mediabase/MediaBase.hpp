/*
 * MediaBase.hpp
 *
 *  Created on: 2016Äê8ÔÂ23ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_MEDIABASE_H_
#define JNI_MEDIAPLAYER_MEDIABASE_MEDIABASE_H_

#include <memory>
#include <mutex>
#include <vector>
#include "../TimedEventQueue.h"

namespace whitebean {

#define US_IN_SECOND 1000000

template<class T>	
struct MediaEvent : public TimedEventQueue::Event {
	MediaEvent(T *contex,
			   void (T::*method)())
	: mContex(contex), mMethod(method) {}
	
	virtual ~MediaEvent() {}
protected:
	virtual void fire(TimedEventQueue *queue, int64_t /* now_us */) {
		(mContex->*mMethod)();
	}
private:
	T *mContex;
	void (T::*mMethod)();
};	
	
class MediaBase {
public:
	MediaBase(): mEvents(EVENT_NUM) {};
	virtual ~MediaBase() {};

	enum EVENT {
		EVENT_WAIT = 0,
		EVENT_WORK,
	    EVENT_SEEK,
		EVENT_EXIT,
		EVENT_NUM,
	};

	enum ERRCODE {
		NO_ERR = 0,
		ERR_EOF = -1,
		ERR_INVALID = -2,
		ERR_AGAIN = -3,
	};

	virtual int start() { return 0; };
	virtual int stop() { return 0; };
protected:
	virtual void initEvents() {}
	
	TimedEventQueue mQueue;
	std::vector<std::shared_ptr<TimedEventQueue::Event> > mEvents;
	mutable std::mutex mStateLock;
};
	
}

#endif
