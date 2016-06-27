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

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"	
}

namespace whitebean {

class MediaSource: public MediaThread {
public:
	MediaSource(): mTracksPtr(new MediaTracks)
				 , mVideoStreamId(-1)
				 , mAudioStreamId(-1)
				 , mEof(false)
	{

	}
	
	virtual ~MediaSource();

	/*
	 * @brief open from a uri
	 */
	int open(const std::string uri);

	std::shared_ptr<AVFormatContext> getFmtCtxPtr() const {
		return mAVFmtCtxPtr;
	}

	std::shared_ptr<MediaTracks> getTracksPtr() const {
		return mTracksPtr;
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
private:
	void threadEntry();
	
	std::shared_ptr<AVFormatContext> mAVFmtCtxPtr;
	std::shared_ptr<MediaTracks>     mTracksPtr;
	int mVideoStreamId;
	int mAudioStreamId;
	bool mEof;
};
	
}

#endif
