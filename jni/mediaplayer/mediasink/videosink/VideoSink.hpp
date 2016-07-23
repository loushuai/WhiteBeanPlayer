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

class VideoSink {
public:
	virtual ~VideoSink() {}
	virtual int init() = 0;
	virtual int display(FrameBuffer &frm) = 0;
};	
	
}

#endif
