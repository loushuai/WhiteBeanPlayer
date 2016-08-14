/*
 * AudioPlayer.cpp
 *
 *  Created on: 2016Äê6ÔÂ9ÈÕ
 *      Author: loushuai
 */

#include "log.hpp"
#include "AudioPlayer.hpp"

using namespace std;

namespace whitebean {

void AudioPlayer::setSource(shared_ptr<MediaSource> source)
{
	mSourcePtr = source;
}

int AudioPlayer::start()
{
	int ret = 0;
	int audioid = mSourcePtr->getAudioStreamId();

	if (audioid < 0) {
		LOGE("No audio stream");
		return -1;
	}

	if (mPaused) {
		mPaused = 0;
		return 0;
	}

	ret = mDecoder.open(mSourcePtr, audioid);
	if (ret < 0) {
		LOGE("Open decoder failed");
		return -1;
	}

	mDecoder.start();

	mSinkPtr = shared_ptr<AudioSink>(new OpenslSink);
	if (!mSinkPtr) {
		return -1;
	}

	int sr;
	if (mDecoder.getMetaData().findInt32(kKeySampleRate, sr) != true) {
		LOGE("Can't find samplerate");
		return -1;
	}
	
	ret = mSinkPtr->open(sr, 2, PCM_FORMAT_FIXED_16, audioSinkCallBack, this);
	if (ret < 0) {
		LOGE("Open audio sink failed");
		return -1;
	}

	mSinkPtr->start();
	
	return 0;
}

int AudioPlayer::pause()
{
	mPaused = 1;

	return 0;
}

void AudioPlayer::stop()
{
	LOGD("AudioPlayer stop");
	mAbout = true;
	mDecoder.stop();
	mSinkPtr->stop();
	LOGD("AudioPlayer stop exit");	
}

size_t AudioPlayer::fillBuffer(std::unique_ptr<uint8_t[]> &buf)
{
	size_t size = 0;
	FrameBuffer frmbuf;
 retry:
	if (mAbout) {		
		return 0;
	}

	if (mPaused) {
		this_thread::sleep_for(chrono::milliseconds(10));
		goto retry;
	}
	
	if (mDecoder.read(frmbuf)) {
		size = frmbuf.asize();
		
		LOGD("audio player get pcm ok, size %d, pts %lld", size, frmbuf.getPts());
		
		unique_ptr<uint8_t[]> pcmbuf(new uint8_t[size]);
		memcpy(pcmbuf.get(), frmbuf.getData().data[0], size);
		buf = std::move(pcmbuf);

		mCurTimeUs = frmbuf.getPts();
	} else {
		this_thread::sleep_for(chrono::milliseconds(10));
		goto retry;
	}

	return size;
}

int64_t AudioPlayer::getCurTime() const
{
	return mCurTimeUs;
}

//static
size_t AudioPlayer::audioSinkCallBack(unique_ptr<uint8_t[]> &buf, void *cookie)
{
	AudioPlayer *me = (AudioPlayer*)cookie;

	if (me) {
		return me->fillBuffer(buf);
	}

	return 0;
}
	
}
