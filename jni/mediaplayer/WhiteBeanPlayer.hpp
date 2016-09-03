/*
 * WhiteBeanPlayer.hpp
 *
 *  Created on: 2016Äê5ÔÂ21ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_WHITEBEANPLAYER_H_
#define JNI_MEDIAPLAYER_WHITEBEANPLAYER_H_

#include <string>
#include <mutex>
#include <memory>
#include <android/native_window_jni.h>
#include "TimedEventQueue.h"
#include "AudioPlayer.hpp"
#include "mediasink/videosink/VideoSink.hpp"
#include "mediasink/videosink/egl/EglSink.hpp"

namespace whitebean {

class MediaPlayerListener
{
public:
    virtual void notify(int msg, int ext1, int ext2) = 0;
};
	
class WhiteBeanPlayer: public IMediaListener {
public:
	WhiteBeanPlayer();
	~WhiteBeanPlayer();

	void setListener(std::shared_ptr<MediaPlayerListener> &listener);
	int setDataSource(const std::string uri);
	int prepare();
	int prepareAsync();
	void reset();
	void stop();
	void release();
	int play();
	int pause();
	void notifyListener(int msg, int ext1 = 0, int ext2 = 0);
	int getCurrentPosition();
	int getDuration();

	void setVideoSurface(ANativeWindow *nativeWindow) {
		mNativeWindow = nativeWindow;
	}
	bool isPlaying() const;
	int seekTo(int64_t msec);

private:
	friend struct WhiteBeanEvent;
	
    enum {
        PLAYING             = 0x01,
        LOOPING             = 0x02,
        FIRST_FRAME         = 0x04,
        PREPARING           = 0x08,
        PREPARED            = 0x10,
        AT_EOS              = 0x20,
        PREPARE_CANCELLED   = 0x40,
        CACHE_UNDERRUN      = 0x80,
        AUDIO_AT_EOS        = 0x0100,
        VIDEO_AT_EOS        = 0x0200,
        AUTO_LOOPING        = 0x0400,

        // We are basically done preparing but are currently buffering
        // sufficient data to begin playback and finish the preparation phase
        // for good.
        PREPARING_CONNECTED = 0x0800,

        // We're triggering a single video event to display the first frame
        // after the seekpoint.
        SEEK_PREVIEW        = 0x1000,

        AUDIO_RUNNING       = 0x2000,
        AUDIOPLAYER_STARTED = 0x4000,

        INCOGNITO           = 0x8000,

        TEXT_RUNNING        = 0x10000,
        TEXTPLAYER_INITIALIZED  = 0x20000,

        SLOW_DECODER_HACK   = 0x40000,
		};

	enum media_event_type {
		MEDIA_NOP               = 0, // interface test message
		MEDIA_PREPARED          = 1,
		MEDIA_PLAYBACK_COMPLETE = 2,
		MEDIA_BUFFERING_UPDATE  = 3,
		MEDIA_SEEK_COMPLETE     = 4,
		MEDIA_SET_VIDEO_SIZE    = 5,
		MEDIA_STARTED           = 6,
		MEDIA_PAUSED            = 7,
		MEDIA_STOPPED           = 8,
		MEDIA_SKIPPED           = 9,
		MEDIA_TIMED_TEXT        = 99,
		MEDIA_ERROR             = 100,
		MEDIA_INFO              = 200,
		MEDIA_SUBTITLE_DATA     = 201,
	};

	std::shared_ptr<MediaPlayerListener> mListener;
	mutable std::mutex mLock;
	mutable std::mutex mStateLock;
	std::condition_variable mPreparedCondition;

	ANativeWindow *mNativeWindow;
    TimedEventQueue mQueue;
    bool mQueueStarted;
	std::shared_ptr<MediaSource> mSourcePtr;
	std::shared_ptr<AudioPlayer> mAudioPlayerPtr;
	std::shared_ptr<VideoSink>   mVideoSinkPtr;
	MediaDecoder mVideoDecoder;
	FrameBuffer mVideoBuffer;
	
    enum FlagMode {
        SET,
        CLEAR,
        ASSIGN
    };
	void modifyFlags(unsigned value, FlagMode mode);

	std::string mUri;
	uint32_t mFlags;
	bool mIsAsyncPrepare;

	std::shared_ptr<TimedEventQueue::Event> mAsyncPrepareEvent;
	std::shared_ptr<TimedEventQueue::Event> mVideoEvent;
	bool mVideoEventPending;

	int64_t mVideoPosition;
	int64_t mDurationUs;
	
	struct Stats {
		std::string mURI;
		uint32_t mFlags;	
	} mStats;

	void postVideoEvent_l(int64_t delayUs = -1);
	void cancelPlayerEvents();
	
	int prepareAsync_l();
	void finishAsync_l();
	void onVideoEvent();
	void onPrepareAsyncEvent();
	void onSeekComplete();
	void reset_l();
	void initRenderer_l();
	int initVideoDecoder();
	int videoNeedRender(FrameBuffer &frm);
	int mediaNotify(int msg, int arg1 = 0, int arg2 = 0);
};

	
}

#endif
