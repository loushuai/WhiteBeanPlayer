/*
 * WhiteBeanPlayer.cpp
 *
 *  Created on: 2016Äê5ÔÂ21ÈÕ
 *      Author: loushuai
 */

#include "log.hpp"
#include "WhiteBeanPlayer.hpp"
#include "mediasink/videosink/egl/EglSink.hpp"

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
, mVideoEventPending(false)
, mVideoPosition(0)
, mDurationUs(0)
{
	LOGD("WhiteBeanPlayer()");
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();

	mVideoEvent = shared_ptr<WhiteBeanEvent>(new WhiteBeanEvent(this, &WhiteBeanPlayer::onVideoEvent));
}

WhiteBeanPlayer::~WhiteBeanPlayer()
{
	LOGD("~WhiteBeanPlayer()");
}

void WhiteBeanPlayer::setListener(shared_ptr<MediaPlayerListener> &listener)
{
	unique_lock<mutex> autoLock(mLock);
	mListener = listener;
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

int WhiteBeanPlayer::initVideoDecoder()
{
	int ret = 0;

	ret = mVideoDecoder.open(mSourcePtr, mSourcePtr->getVideoStreamId());
	if (ret < 0) {
		LOGD("Open video decoder failed");
		return -1;
	}

	mVideoDecoder.start();
	
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

	finishAsync_l();

	notifyListener(MEDIA_PREPARED);
	
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

void WhiteBeanPlayer::finishAsync_l()
{
	int64_t durationUs;
	if (mSourcePtr->getFormat()->findInt64(kKeyDuration, durationUs)) {
		mDurationUs = durationUs;
	}
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

	if (mSourcePtr->hasVideo()) {
		initVideoDecoder();
	}

	modifyFlags((PREPARING|PREPARE_CANCELLED|PREPARING_CONNECTED), CLEAR);
	modifyFlags(PREPARED, SET);
 out:
	mPreparedCondition.notify_all();
}

void WhiteBeanPlayer::onVideoEvent()
{
	unique_lock<mutex> autoLock(mLock);

	LOGD("onVideoEvent");

	mVideoEventPending = false;

	if (!mVideoSinkPtr) {		
		initRenderer_l();
	}

	if (mVideoSinkPtr) {
		bool ret = false;

		if (mVideoBuffer.empty()) {
			ret = mVideoDecoder.read(mVideoBuffer);
		}

		if (!mVideoBuffer.empty() && videoNeedRender(mVideoBuffer)) {
			mVideoSinkPtr->display(mVideoBuffer);
			mVideoPosition = mVideoBuffer.getPts();
			ret = mVideoDecoder.read(mVideoBuffer);
		}
	}

	postVideoEvent_l();
}

void WhiteBeanPlayer::postVideoEvent_l(int64_t delayUs)
{
	if (mVideoEventPending) {
		return;
	}

	mVideoEventPending = true;
	mQueue.postEventWithDelay(mVideoEvent, delayUs < 0 ? 10000 : delayUs);
}
	
void WhiteBeanPlayer::reset_l()
{
	mUri = "";
}

void WhiteBeanPlayer::stop()
{
	LOGD("WhiteBean stop");
	
    if (mQueueStarted) {
        mQueue.stop();
        mQueueStarted = false;
    }

	if (mSourcePtr) {
		mSourcePtr->stop();
	}
	
	if (mAudioPlayerPtr) {
		mAudioPlayerPtr->stop();
	}

	mVideoDecoder.stop();
}

void WhiteBeanPlayer::release()
{
	
}

void WhiteBeanPlayer::initRenderer_l()
{
	if (!mNativeWindow) {
		LOGE("Native window is null");
		return;
	}

	mVideoSinkPtr = shared_ptr<VideoSink>(new EglSink(mNativeWindow));

	if (mVideoSinkPtr->init()) {
		LOGE("Video sink init failed");
		return;
	}
}

int WhiteBeanPlayer::play()
{
	LOGD("Play enter");

	unique_lock<mutex> autoLock(mLock);

    if (mFlags & PLAYING) {
        return 0;
    }
	
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
		}
		mAudioPlayerPtr->start();
	}

	if (mSourcePtr->hasVideo()) {
		postVideoEvent_l();
	}

	return 0;
}

int WhiteBeanPlayer::pause()
{
	unique_lock<mutex> autoLock(mLock);
	
	if (!(mFlags & PLAYING)) {
		return 0;
	}

	if (mAudioPlayerPtr) {
		mAudioPlayerPtr->pause();
	}
	cancelPlayerEvents();

	modifyFlags(PLAYING, CLEAR);

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

/* 
 *   return: 1 render, 0 not render
 */	
int WhiteBeanPlayer::videoNeedRender(FrameBuffer &frm)
{
	int64_t audioTimeUs = -1, videoTimeUs = -1;
	if (mAudioPlayerPtr) {
		audioTimeUs = mAudioPlayerPtr->getCurTime();
	}

	videoTimeUs = frm.getPts();
	int64_t latenessUs = videoTimeUs - audioTimeUs;

	//debug
	LOGD("Time stames: video %lld, audio %lld, lateness %lld",
		 videoTimeUs, audioTimeUs, latenessUs);
	
	if (latenessUs < -10000) {
		return 1;
	}

	return 0;
}

void WhiteBeanPlayer::notifyListener(int msg, int ext1, int ext2)
{
	LOGD("notifyListener enter");
	if (mListener) {
		LOGD("notify");
		mListener->notify(msg, ext1, ext2);
	}
}

bool WhiteBeanPlayer::isPlaying() const
{
	return mFlags & PLAYING;
}

int WhiteBeanPlayer::seekTo(int64_t msec)
{
	LOGD("Seek to %lld", msec);
	unique_lock<mutex> autoLock(mLock);

	mSourcePtr->seekTo(msec);
	
	return 0;
}

int WhiteBeanPlayer::getCurrentPosition()
{
	unique_lock<mutex> autoLock(mLock);
	int64_t curPos = 0;

	if (mAudioPlayerPtr) {
		curPos = mAudioPlayerPtr->getCurTime();
	} else {
		curPos = mVideoPosition;
	}

	//Curpos = mVideoPosition > curPos ? mVideoPosition : curPos;

	return curPos/1000;
}

int WhiteBeanPlayer::getDuration()
{
	unique_lock<mutex> autoLock(mLock);
	if (mDurationUs >= 0) {
		return mDurationUs/1000;
	}

	return -1;
}

void WhiteBeanPlayer::cancelPlayerEvents()
{
	mQueue.cancelEvent(mVideoEvent->eventID());
	mVideoEventPending = false;
}
	
}
