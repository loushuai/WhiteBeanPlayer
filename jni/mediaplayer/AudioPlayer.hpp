/*
 * AudioPlayer.hpp
 *
 *  Created on: 2016Äê6ÔÂ9ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_AUDIOPLAYER_H_
#define JNI_MEDIAPLAYER_AUDIOPLAYER_H_

#include <memory>
#include "mediabase/MediaCodec.hpp"
#include "mediasink/audiosink/opensl/openslsink.hpp"

namespace whitebean {

class AudioPlayer {
public:
	AudioPlayer(): mAbout(false),
				   mPaused(0) {}
	~AudioPlayer() {}

	void setSource(std::shared_ptr<MediaSource> source);
	int start();
	int pause();
	void stop();
	int64_t getCurTime() const; // in us
private:
	static size_t audioSinkCallBack(std::unique_ptr<uint8_t[]> &buf, void *cookie = nullptr);
	size_t fillBuffer(std::unique_ptr<uint8_t[]> &buf);
	
	MediaDecoder mDecoder;
	std::shared_ptr<MediaSource> mSourcePtr;
	std::shared_ptr<AudioSink> mSinkPtr;
    int64_t mCurTimeUs; // in us
	int mAbout;
	int mPaused;
};
	
}

#endif
