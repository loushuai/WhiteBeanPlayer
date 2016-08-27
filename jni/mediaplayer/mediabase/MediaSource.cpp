/*
 * MediaSource.cpp
 *
 *  Created on: 2016Äê5ÔÂ22ÈÕ
 *      Author: loushuai
 */

#include <errno.h>
#include "log.hpp"
#include "MediaSource.hpp"

using namespace std;

namespace whitebean {
	
MediaSource::~MediaSource()
{

}

int MediaSource::open(const string uri)
{
	AVFormatContext *fmtptr = nullptr;
	
	mFormat = make_shared<MetaData>();
	if (!mFormat) {
		goto failed;
	}
	
	if ((avformat_open_input(&fmtptr, uri.c_str(), NULL, NULL) < 0)
	|| (fmtptr == nullptr)) {
		LOGE("open input failed %s", strerror(errno));
		goto failed;
	}

	if (avformat_find_stream_info(fmtptr, NULL) < 0) {
		LOGE("find stream info failed");
		goto failed;
	}

	// find video stream
	for (int i = 0; i < fmtptr->nb_streams; i++) {
		if (AVMEDIA_TYPE_VIDEO == fmtptr->streams[i]->codec->codec_type) {
			LOGD("video stream %d", i);
			mVideoStreamId = i;
			mTracksPtr->setVideoStream(i);
			break;
		}
	}
	// find audio stream
	for (int i = 0; i < fmtptr->nb_streams; i++) {
		if (AVMEDIA_TYPE_AUDIO == fmtptr->streams[i]->codec->codec_type) {
			LOGD("audio stream %d", i);
			mAudioStreamId = i;
			mTracksPtr->setAudioStream(i);
			break;
		}
	}

	mAVFmtCtxPtr = shared_ptr<AVFormatContext>(fmtptr,
				   [](AVFormatContext *p){avformat_close_input(&p);});

	LOGD("Duration %lld", mAVFmtCtxPtr->duration);
	mFormat->setInt64(kKeyDuration, mAVFmtCtxPtr->duration);

	initEvents();
		
	// start event queue
	mQueue.start();
	
	LOGD("Media source open success");
	
	return 0;

 failed:

	return -1;
}

int MediaSource::start()
{
	mVideoReady = 1;
	mAudioReady = 1;

	onWaitEvent();	

	return 0;
}

int MediaSource::stop()
{
	mQueue.stop();
	return 0;
}	

int MediaSource::seekTo(int64_t msec)
{
	mSeeking = 1;
	mSeekTimeMs = msec;

	return 0;
}

int MediaSource::seekTo_l(int64_t msec)
{
	int ret = 0;
	int seekStream = -1;
	
	mTracksPtr->clear();

	if (hasVideo()) {
		seekStream = mVideoStreamId;
	} else {
		seekStream = mAudioStreamId;
	}

	if (seekStream < 0) {
		return -1;
	}

	int64_t ts = 0;
	ts = msec / av_q2d(getTimeScaleOfTrack(seekStream)) / 1000;
	ret = av_seek_frame(mAVFmtCtxPtr.get(), seekStream, ts, AVSEEK_FLAG_BACKWARD);
	if (ret < 0) {
		LOGE("av_seek_frame failed (%d)", ret);
		return -1;
	}
	
	return 0;
}

int MediaSource::readPacket()
{
	int ret = 0;

	if (!mTracksPtr) {		
		return ERR_INVALID;
	}

	if (mTracksPtr->full()) {
		LOGD("Media tracks full");
		return ERR_AGAIN;
	}	

	AVPacket packet;
	av_init_packet(&packet);

	ret = av_read_frame(mAVFmtCtxPtr.get(), &packet);
	if (ret < 0) {
		if (ret == AVERROR_EOF) {
			LOGD("av_read_frame EOF");
			mEof = true;
			return ERR_EOF;
		}
		LOGD("av_read_frame error %d", ret);
		return ERR_AGAIN;
	}

	PacketBuffer pktbuf(packet);
	mTracksPtr->packetIn(pktbuf);
	av_packet_unref(&packet); // 	

	return 0;
}

void MediaSource::initEvents()
{
	mEvents[EVENT_WAIT] = shared_ptr<TimedEventQueue::Event> (new MediaEvent<MediaSource>(
															  this, &MediaSource::onWaitEvent));
	mEvents[EVENT_WORK] = shared_ptr<TimedEventQueue::Event> (new MediaEvent<MediaSource>(
															  this, &MediaSource::onWorkEvent));
	mEvents[EVENT_EXIT] = shared_ptr<TimedEventQueue::Event> (new MediaEvent<MediaSource>(
															  this, &MediaSource::onExitEvent));
}

void MediaSource::onWaitEvent()
{
	LOGD("Media source waiting...");

	if ((hasVideo() && (!mVideoReady))
		|| (hasAudio() && (!mAudioReady))) {
		this_thread::sleep_for(chrono::milliseconds(100));
		mQueue.postEvent(mEvents[EVENT_WAIT]);
	} else {
		mQueue.postEvent(mEvents[EVENT_WORK]);
	}

}

void MediaSource::onWorkEvent()
{
	int ret = 0;
	
	LOGD("Media source working...");

	ret = readPacket();
	if (ret == 0) {
		//
	}else if (ret == ERR_AGAIN) {
		this_thread::sleep_for(chrono::milliseconds(100));		
	} else {
		//
	}

	mQueue.postEvent(mEvents[EVENT_WORK]);
}

void MediaSource::onExitEvent()
{

}
	
}
