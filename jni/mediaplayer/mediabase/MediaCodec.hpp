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
#include "MediaBase.hpp"
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
	
class Codec: public MediaBase {
public:
    Codec():mFrameQueue(16)
		   {}
	virtual ~Codec() {}

	virtual int open(std::shared_ptr<MediaSource> source) = 0;
	virtual bool read(FrameBuffer &frmbuf) = 0;
	virtual void timeScaleToUs(FrameBuffer &frmbuf);
	virtual void clear();
	virtual int start();
	virtual int stop();

	virtual MetaData& getMetaData() {
		return mMetaData;
	}
protected:	
	virtual int initFilters() {return 0;}
	int open_l();
	void clear_l();
    virtual int decode() { return 0;}
	virtual void initEvents();
	virtual void onWaitEvent();
	virtual void onWorkEvent();
	virtual void onClearEvent();
	virtual void onExitEvent();	

	std::shared_ptr<MediaSource>     mSource;
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

	virtual int open(std::shared_ptr<MediaSource> source) override;	
	virtual bool read(FrameBuffer &frmbuf) override;

private:
	virtual int initFilters() override;
	virtual int decode() override;
	
	int mSampleRate;
	int mChannels;
	int mSampleFmt;
};

class VideoDecoder : public Codec {
public:
	VideoDecoder();
	virtual int open(std::shared_ptr<MediaSource> source) override;
	virtual bool read(FrameBuffer &frmbuf) override;

private:	
	virtual int initFilters() override;
	virtual int decode() override;

	int mWidth;
	int mHeight;
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

	void stop() {
		if (mDelegatePtr) {
			mDelegatePtr->stop();
		}
	}

	int seekTo(int64_t msec) {
		if (mDelegatePtr) {
			mDelegatePtr->clear();
		}
		return 0;
	}

	// resume frome halt
	void resume() {
		mDelegatePtr->resume();
	}

	MetaData& getMetaData () {
		return mDelegatePtr->getMetaData();
	}

	void setListener(IMediaListener *listener) {
		mDelegatePtr->setListener(listener);
	}
private:
	std::unique_ptr<Codec> mDelegatePtr;
};
	
}

#endif
