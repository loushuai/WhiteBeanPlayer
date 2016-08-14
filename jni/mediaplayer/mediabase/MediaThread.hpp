/*
 * MediaThread.hpp
 *
 *  Created on: 2016Äê5ÔÂ31ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_MEDIATHREAD_H_
#define JNI_MEDIAPLAYER_MEDIABASE_MEDIATHREAD_H_

#include <thread>

namespace whitebean {

class MediaThread {
public:
	MediaThread(): mStopped(false), mRunning(false) {}
	virtual ~MediaThread() {}

	int start() {
		if (mRunning) {
			return 0;
		}

		mThread = std::thread(ThreadWrapper, this);

		mRunning = true;
		
		return 0;
	}

	void stop() {
		mStopped = true;
		mThread.join();
		mRunning = false;
	}

	bool isRunning() const {
		return mRunning;
	}

protected:
	static void *ThreadWrapper(void *me) {
		MediaThread *th = (MediaThread*)me;
		th->threadEntry();
		return nullptr;
	}
	
	virtual void threadEntry() = 0;

	std::thread mThread;
	bool mStopped;
	bool mRunning;
};
	
}

#endif
