/*
 * MediaSource.hpp
 *
 *  Created on: 2016Äê5ÔÂ21ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_MEDIASOURCE_H_
#define JNI_MEDIAPLAYER_MEDIABASE_MEDIASOURCE_H_

#include <string>
#include <thread>
#include <memory>
#include "MediaTracks.hpp"
#include "MediaThread.hpp"
#include "MediaBase.hpp"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"	
}

namespace whitebean {

class MediaSource: public MediaBase {
public:
	MediaSource(): mTracksPtr(new MediaTracks)
				 , mVideoStreamId(-1)
				 , mAudioStreamId(-1)
				 , mEof(false)
				 , mSeeking(0)
				 , mSeekTimeMs(-1)
				 , mVideoReady(0)
				 , mAudioReady(0)
	{

	}
	
	virtual ~MediaSource();

	/*
	 * @brief open from a uri
	 */
	int open(const std::string uri);

	int start();
	int stop();

	std::shared_ptr<AVFormatContext> getFmtCtxPtr() const {
		return mAVFmtCtxPtr;
	}

	std::shared_ptr<MediaTracks> getTracksPtr() const {
		return mTracksPtr;
	}

	std::shared_ptr<MetaData> getFormat() const {
		return mFormat;
	}

	int getVideoStreamId() const {
		return mVideoStreamId;
	}

	int getAudioStreamId() const {
		return mAudioStreamId;
	}

	bool hasVideo() const {
		return mVideoStreamId >= 0;
	}

	bool hasAudio() const {
		return mAudioStreamId >= 0;
	}

	bool eof() const {
		return mEof;
	}

	int seekTo(int64_t msec);
	int seekTo_l(int64_t msec);

	AVRational getTimeScaleOfTrack(int idx) {
		return mAVFmtCtxPtr->streams[idx]->time_base;
	}
private:
	virtual void initEvents();
	void onWaitEvent();
	void onWorkEvent();
	void onExitEvent();

	int readPacket();

	std::shared_ptr<TimedEventQueue::Event> mSourceEvent;
	
	std::shared_ptr<AVFormatContext> mAVFmtCtxPtr;
	std::shared_ptr<MediaTracks>     mTracksPtr;
	std::shared_ptr<MetaData>        mFormat;
	int mVideoStreamId;
	int mAudioStreamId;
	bool mEof;
	int mSeeking;
	int64_t mSeekTimeMs; // msec
	int mVideoReady;
	int mAudioReady;
};
	
}

#endif
