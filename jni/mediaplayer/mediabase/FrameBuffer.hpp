/*
 * FrameBuffer.hpp
 *
 *  Created on: 2016Äê6ÔÂ3ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_FRAMEBUFFER_H_
#define JNI_MEDIAPLAYER_MEDIABASE_FRAMEBUFFER_H_

#include "MediaBuffer.hpp"

extern "C" {
#include "libavformat/avformat.h"	
#include "libavutil/imgutils.h"	
}

namespace whitebean {

class FrameBuffer : MediaBuffer<AVFrame> {
public:
	FrameBuffer() {
		::memset(&data, 0, sizeof(AVFrame));
		av_frame_unref(&data);				
	}
	
	FrameBuffer(const AVFrame &frame) {
		::memset(&data, 0, sizeof(AVFrame));
		av_frame_unref(&data);		
		if (av_frame_ref(&data, &frame) != 0) {
			data.data[0] = nullptr;
		}
	}

	FrameBuffer(const FrameBuffer &frmbuf) {
		::memset(&data, 0, sizeof(AVFrame));
		av_frame_unref(&data);		
		if (av_frame_ref(&data, &(frmbuf.getData())) != 0) {
			data.data[0] = nullptr;			
		}
	}

	FrameBuffer& operator=(const FrameBuffer &rhs) {
		if (data.data[0] != rhs.data.data[0]) {
			av_frame_unref(&data);
			if (av_frame_ref(&data, &(rhs.getData())) != 0) {
				data.data[0] = nullptr;			
			}
		}
		return *this;
	}

	~FrameBuffer() {
		av_frame_unref(&data);
	}

	void reset() {
		av_frame_unref(&data);		
		::memset(&data, 0, sizeof(AVFrame));		
	}

	const AVFrame& getData() const {
		return data;
	}

	const AVFrame* getDataPtr() const {
		return &data;
	}

	AVFrame* getDataPtr() {
		return &data;
	}

	bool empty() const {
		return data.data[0] == nullptr;
	}

	int32_t asize() const {
		return data.channels
		     * av_get_bytes_per_sample((enum AVSampleFormat) data.format)
			 * data.nb_samples;
	}

	int32_t vsize() const {
		return av_image_get_buffer_size((enum AVPixelFormat)data.format, data.width, data.height, 1);
	}

	int getNumDataPlanes() const {
		return AV_NUM_DATA_POINTERS;
	}

	uint8_t *getDataPlane(int plane) const {
		return data.data[plane];
	}

	int getLineSize(int plane) const {
		return data.linesize[plane];
	}

	int getWidth() const {
		return data.width;
	}

	int getHeight() const {
		return data.height;
	}

	int getFormat() const {
		return data.format;
	}

	int64_t getPts() const {
		return data.pkt_pts;
	}

	void setPts(int64_t pts) {
		data.pkt_pts = pts;
	}
};
	
}

#endif
