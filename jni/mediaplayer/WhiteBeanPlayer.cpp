/*
 * WhiteBeanPlayer.cpp
 *
 *  Created on: 2016Äê5ÔÂ21ÈÕ
 *      Author: loushuai
 */

#include "log.hpp"
#include "WhiteBeanPlayer.hpp"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfiltergraph.h"	
}

using namespace std;

namespace whitebean {

struct WhiteBeanEvent : public TimedEventQueue::Event {
	WhiteBeanEvent(WhiteBeanPlayer *player,
				   void (WhiteBeanPlayer::*method)())
	: mPlayer(player), mMethod(method) {}
	
	virtual ~WhiteBeanEvent() {}
protected:
	virtual void fire(TimedEventQueue *queue, int64_t /* now_us */) {
		(mPlayer->*mMethod)();
	}
private:
	WhiteBeanPlayer *mPlayer;
	void (WhiteBeanPlayer::*mMethod)();
};

WhiteBeanPlayer::WhiteBeanPlayer()
: mQueueStarted(false)
, mFlags(0)
, mIsAsyncPrepare(false)
{
	LOGD("WhiteBeanPlayer()");
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();	
}

WhiteBeanPlayer::~WhiteBeanPlayer()
{
	LOGD("~WhiteBeanPlayer()");
}

int WhiteBeanPlayer::setDataSource(const string uri)
{
	reset_l();

	mUri = uri;
	
	{
		unique_lock<mutex> autoLock(mStateLock);
		mStats.mURI = mUri;
	}

	return 0;
}

int WhiteBeanPlayer::prepareAsync()
{
	unique_lock<mutex> autoLock(mLock);

    if (mFlags & PREPARING) {
		return -1;
	}

    mIsAsyncPrepare = true;
    return prepareAsync_l();	
}

void WhiteBeanPlayer::reset()
{
	unique_lock<mutex> autoLock(mLock);
	reset_l();
}

int WhiteBeanPlayer::prepare()
{
	unique_lock<mutex> autoLock(mLock);	

    if (mFlags & PREPARED) {
        return 0;
    }

    if (mFlags & PREPARING) {
        return -1;
    }

	mIsAsyncPrepare = false;
	int err = prepareAsync_l();
	if (err != 0) {
		return err;
	}

	// wait prepare finish
	while (mFlags & PREPARING) {
		mPreparedCondition.wait(autoLock);
	}

	LOGD("Prepare finish");
	
	return 0;
}

int WhiteBeanPlayer::prepareAsync_l()
{
    if (mFlags & PREPARING) {
		return -1;
	}

    if (!mQueueStarted) {
        mQueue.start();
        mQueueStarted = true;
    }

    modifyFlags(PREPARING, SET);
    mAsyncPrepareEvent = shared_ptr<TimedEventQueue::Event> (new WhiteBeanEvent(
									   this, &WhiteBeanPlayer::onPrepareAsyncEvent));

    mQueue.postEvent(mAsyncPrepareEvent);	
	
	return 0;
}

void WhiteBeanPlayer::onPrepareAsyncEvent()
{
	LOGD("onPrepareAsyncEvent");
	int ret = 0;
	unique_lock<mutex> autoLock(mLock);

	mSourcePtr = shared_ptr<MediaSource>(new MediaSource);
	ret = mSourcePtr->open(mUri);
	if (ret != 0) {
		// cancel prepare
		LOGD("Set datasource error");
		goto out;
	}
	
	mSourcePtr->start();

	modifyFlags((PREPARING|PREPARE_CANCELLED|PREPARING_CONNECTED), CLEAR);
	modifyFlags(PREPARED, SET);
 out:
	mPreparedCondition.notify_all();
}

void WhiteBeanPlayer::reset_l()
{
	mUri = "";
}

int WhiteBeanPlayer::play()
{
	LOGD("Play enter");
	
	if (!(mFlags & PREPARED)) {
		return -1;
	}

	modifyFlags(PLAYING, SET);

	if (!mSourcePtr) {
		return -1;
	}

	if (mSourcePtr->hasAudio()) {
		LOGD("Has audio");
		if (!mAudioPlayerPtr) {
			mAudioPlayerPtr = shared_ptr<AudioPlayer>(new AudioPlayer);
			if (!mAudioPlayerPtr) {
				return -1;
			}

			mAudioPlayerPtr->setSource(mSourcePtr);
			mAudioPlayerPtr->start();
		}
	}

	return 0;
}

void WhiteBeanPlayer::modifyFlags(unsigned value, FlagMode mode)
{
    switch (mode) {
        case SET:
            mFlags |= value;
            break;
        case CLEAR:
			//            if ((value & CACHE_UNDERRUN) && (mFlags & CACHE_UNDERRUN)) {
			//                notifyListener_l(MEDIA_INFO, MEDIA_INFO_BUFFERING_END);
			//            }
            mFlags &= ~value;
            break;
        case ASSIGN:
            mFlags = value;
            break;
        default:
			break;
    }

	{
		unique_lock<mutex> autoLock(mStateLock);
		mStats.mFlags = mFlags;
	}
}
	
}
