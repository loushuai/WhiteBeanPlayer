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

class IMediaListener {
public:
	IMediaListener() {}
	virtual ~IMediaListener() {}

	enum MESSAGE {
		SOURCE_SEEK_COMPLETE = 0,
		DECODER_CLEAR_COMPLETE,
	};

	virtual int mediaNotify(int msg, int arg1 = 0, int arg2 = 0) = 0;
};
	
class MediaBase {
public:
	MediaBase(): mEvents(EVENT_NUM)
			   , mWaiting(0)
			   , mListener(nullptr)
	{};
	virtual ~MediaBase() {};

	enum EVENT {
		EVENT_WAIT = 0,
		EVENT_WORK,
	    EVENT_SEEK,
		EVENT_CLEAR,
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

	virtual int waiting() const {
		std::unique_lock<std::mutex> autoLock(mBaseLock);
		return mWaiting;
	}
	
	virtual void halt() {
		std::unique_lock<std::mutex> autoLock(mBaseLock);
		mWaiting = 1;
	}

	virtual void resume() {
		std::unique_lock<std::mutex> autoLock(mBaseLock);
		mWaiting = 0;
	}

	void setListener(IMediaListener *listener) {
		std::unique_lock<std::mutex> autoLock(mBaseLock);
		mListener = listener;
	}
protected:
	virtual void initEvents() {}
	
	TimedEventQueue mQueue;
	int mWaiting;
	std::vector<std::shared_ptr<TimedEventQueue::Event> > mEvents;
	mutable std::mutex mBaseLock;
	IMediaListener *mListener;
};
	
}

#endif
