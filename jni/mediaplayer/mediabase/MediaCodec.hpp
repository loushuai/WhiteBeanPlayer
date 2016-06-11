/*
 * MediaCodec.hpp
 *
 *  Created on: 2016Äê5ÔÂ31ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_MEDIACODEC_H_
#define JNI_MEDIAPLAYER_MEDIABASE_MEDIACODEC_H_

#include <memory>
#include <queue>
#include <mutex>
#include "MediaSource.hpp"
#include "MediaThread.hpp"
#include "FrameBuffer.hpp"

extern "C" {
#include "libavformat/avformat.h"	
#include "libavcodec/avcodec.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/opt.h"
}

namespace whitebean {

struct FilterContext {
	std::shared_ptr<AVFilterGraph>   filterGraph;
	AVFilterContext *bufferSinkCtx;
	AVFilterContext *bufferSrcCtx;	
};
	
class Codec: public MediaThread {
public:
    Codec():mFrameQueue(16) {}
	virtual ~Codec() {}

	virtual int open(std::shared_ptr<MediaSource> source) = 0;	
	virtual bool read(FrameBuffer &frmbuf) = 0;

	virtual MetaData& getMetaData() {
		return mMetaData;
	}
protected:	
	virtual void threadEntry() {}
	virtual int initFilters() {return 0;}
	int open(std::shared_ptr<AVCodecContext> codecCtxPtr);

	std::shared_ptr<AVFormatContext> mAVFmtCtxPtr;
	std::shared_ptr<AVCodecContext>  mCodecPtr;
	std::shared_ptr<MediaTracks>     mTracksPtr;
	MediaBufferQueue<FrameBuffer>    mFrameQueue;
	FilterContext mFilterCtx;
	MetaData mMetaData;
	int mStreamId;
};

class AudioDecoder : public Codec {
public:
	AudioDecoder();

	virtual int open(std::shared_ptr<MediaSource> source);	
	virtual bool read(FrameBuffer &frmbuf);

private:
	virtual void threadEntry();
	virtual int initFilters();
	
	int mSampleRate;
	int mChannels;
	int mSampleFmt;
};

class VideoDecoder : public Codec {
public:
	VideoDecoder() {}
	virtual int open(std::shared_ptr<MediaSource> source);
	virtual bool read(FrameBuffer &frmbuf);
	virtual void threadEntry();
};

class MediaDecoder {
public:
	MediaDecoder() {}
	virtual ~MediaDecoder() {}

	int open(std::shared_ptr<MediaSource> source, int streamid);
	
	bool read(FrameBuffer &frmbuf) {
		return mDelegatePtr->read(frmbuf);		
	}
	
	int start() {
		if (mDelegatePtr) {
			mDelegatePtr->start();
		}
		
		return -1;
	}

	MetaData& getMetaData () {
		return mDelegatePtr->getMetaData();
	}
private:
	std::unique_ptr<Codec> mDelegatePtr;
};
	
}

#endif
