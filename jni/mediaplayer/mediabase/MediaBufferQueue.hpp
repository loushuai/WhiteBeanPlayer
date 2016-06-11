/*
 * MediaBufferQueue.hpp
 *
 *  Created on: 2016Äê6ÔÂ4ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_MEDIABUFFERQUEUE_H_
#define JNI_MEDIAPLAYER_MEDIABASE_MEDIABUFFERQUEUE_H_

#include <queue>
#include <mutex>
#include <memory>

namespace whitebean {

#define DEFAULT_BUFFER_SLOTS 256

class QueueSlots {
public:
	QueueSlots(int n = DEFAULT_BUFFER_SLOTS): mNum(n) {}
	virtual ~QueueSlots() {}

	bool get() {
		std::lock_guard<std::mutex> lock(mMutex);
		if (mNum <= 0) {
			return false;
		}

		mNum--;
		return true;
	}

	void put() {
		std::lock_guard<std::mutex> lock(mMutex);
		mNum++;
	}

	bool full() const {
		return mNum <= 0;
	}

private:
	std::mutex mMutex;
	int mNum;
};
	
template <typename T>	
class MediaBufferQueue {
public:
	MediaBufferQueue(): mSlots(new QueueSlots) {}
	MediaBufferQueue(int slotnum): mSlots(new QueueSlots(slotnum)) {}
	MediaBufferQueue(std::shared_ptr<QueueSlots> slots): mSlots(slots) {}
	virtual ~MediaBufferQueue() {}

	bool push(T &val) {
		std::lock_guard<std::mutex> lock(mMutex);
		if (mSlots->get() == false) {
			return false;
		}

		mQueue.push(val);

		return true;
	}

	T &front() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mQueue.front();
	}
	
	void pop() {
		std::lock_guard<std::mutex> lock(mMutex);

		if (mQueue.empty()) {
			return;
		}

		mQueue.pop();
		mSlots->put();
	}
	
	int size() const {
		std::lock_guard<std::mutex> lock(mMutex);
		return mQueue.size();
	}

	bool empty() const {
		std::lock_guard<std::mutex> lock(mMutex);
		return mQueue.empty();
	}

	bool full() const {
		std::lock_guard<std::mutex> lock(mMutex);
		return mSlots->full();
	}
	
private:
	std::shared_ptr<QueueSlots> mSlots;
	std::queue<T> mQueue;
	mutable std::mutex mMutex;
};
	
}

#endif
