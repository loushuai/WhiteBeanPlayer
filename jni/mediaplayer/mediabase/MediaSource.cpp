/*
 * MediaSource.cpp
 *
 *  Created on: 2016Äê5ÔÂ22ÈÕ
 *      Author: loushuai
 */

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
	if ((avformat_open_input(&fmtptr, uri.c_str(), NULL, NULL) < 0)
	|| (fmtptr == nullptr)) {
		LOGE("open input failed");
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

	LOGD("Media source open success");
	
	return 0;

 failed:

	return -1;
}

void MediaSource::threadEntry()
{
	int ret = 0;
	
	LOGD("Media source loop enter");

	if (!mTracksPtr) {
		return;
	}
	
	for (;;) {
		if (mStopped) {
			break;
		}

		if (mTracksPtr->full()) {
			LOGD("Media tracks full");
			this_thread::sleep_for(chrono::milliseconds(100));
			continue;
		}

		AVPacket packet;
		av_init_packet(&packet);
		
		ret = av_read_frame(mAVFmtCtxPtr.get(), &packet);
		if (ret < 0) {
			if (ret == AVERROR_EOF) {
				LOGD("EOF");
				break;
			}

			LOGD("Error");
		}

//		LOGD("Packet in pts %lld", packet.pts);
		PacketBuffer pktbuf(packet);
		mTracksPtr->packetIn(pktbuf);
		av_packet_unref(&packet);
	}

	LOGD("Media source loop exit");
}
	
}
