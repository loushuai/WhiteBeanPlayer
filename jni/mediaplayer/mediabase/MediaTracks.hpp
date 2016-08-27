/*
 * MediaTrack.hpp
 *
 *  Created on: 2016Äê5ÔÂ25ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_MEDIATRACKS_H_
#define JNI_MEDIAPLAYER_MEDIABASE_MEDIATRACKS_H_

#include <queue>
#include <mutex>
#include <memory>
#include "PacketBuffer.hpp"
#include "MediaBufferQueue.hpp"

namespace whitebean {

#define MAX_BUFFER_SLOTS 512
	
class MediaTracks {
public:
	MediaTracks();
	~MediaTracks() {}

	void setVideoStream(int id) {
		mVideoStreamId = id;
	}

	void setAudioStream(int id) {
		mAudioStreamId = id;
	}

	bool readVideo(PacketBuffer &pktbuf);
	bool readAudio(PacketBuffer &pktbuf);

	void packetIn(PacketBuffer &pktbuf);

	bool full();
	void clear();
private:
	int mVideoStreamId;
	int mAudioStreamId;
	std::shared_ptr<QueueSlots> mSlots;
	MediaBufferQueue<PacketBuffer> mVideoQueue;
	MediaBufferQueue<PacketBuffer> mAudioQueue;
};
	
}

#endif
