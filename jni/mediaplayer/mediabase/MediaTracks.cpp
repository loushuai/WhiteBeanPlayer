/*
 * MediaTrack.cpp
 *
 *  Created on: 2016Äê5ÔÂ28ÈÕ
 *      Author: loushuai
 */


#include "MediaTracks.hpp"
#include "log.hpp"

using namespace std;

namespace whitebean {

MediaTracks::MediaTracks()
: mSlots(new QueueSlots)
, mVideoQueue(mSlots)
, mAudioQueue(mSlots)
, mVideoStreamId(-1)
, mAudioStreamId(-1)
{

}

void MediaTracks::packetIn(PacketBuffer &pktbuf)
{
	if (pktbuf.getData().stream_index == mVideoStreamId) {
		LOGD("Packet in video packet %lld", pktbuf.getData().pts);
		//		mVideoQueue.push(pktbuf);
	} else if (pktbuf.getData().stream_index == mAudioStreamId) {
		LOGD("Packet in audio packet %lld", pktbuf.getData().pts);
		mAudioQueue.push(pktbuf);
	} else {
		return;
	}

	return;
}

bool MediaTracks::readVideo(PacketBuffer &pktbuf)
{
	if (!mVideoQueue.empty()) {
		pktbuf = mVideoQueue.front();
		mVideoQueue.pop();
		return true;
	}
	return false;
}

bool MediaTracks::readAudio(PacketBuffer &pktbuf)
{
	LOGD("Read audio packet");
	if (!mAudioQueue.empty()) {
		pktbuf = mAudioQueue.front();
		mAudioQueue.pop();
		LOGD("Read audio pts %lld", pktbuf.getData().pts);
		return true;
	}
	LOGD("Read audio packet end");
	return false;
}

bool MediaTracks::full()
{
	return mSlots->full();
}
	
}
