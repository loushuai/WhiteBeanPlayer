/*
 * VideoSink.hpp
 *
 * Created on: 2016Äê7ÔÂ3ÈÕ
 *     Author: loushuai
 */

#ifndef JNI_MEDIASINK_VIDEOSINK_VIDEOSINK_H_
#define JNI_MEDIASINK_VIDEOSINK_VIDEOSINK_H_

#include "../../mediabase/FrameBuffer.hpp"

namespace whitebean
{

enum {
	VIDEO_SINK_TYPE_NORMAL,
	VIDEO_SINK_TYPE_PANORAMIC,
};

class VideoSink {
public:
	virtual ~VideoSink() {}
	virtual int init(int type = VIDEO_SINK_TYPE_NORMAL) = 0;
	virtual int display(FrameBuffer &frm) = 0;
	virtual void onTouchMoveEvent(float dx, float dy) = 0;
};	
	
}

#endif
