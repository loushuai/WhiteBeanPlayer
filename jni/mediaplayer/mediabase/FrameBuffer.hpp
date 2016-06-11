/*
 * FrameBuffer.hpp
 *
 *  Created on: 2016Äê6ÔÂ3ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_FRAMEBUFFER_H_
#define JNI_MEDIAPLAYER_MEDIABASE_FRAMEBUFFER_H_

#include "MediaBuffer.hpp"

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

	int32_t size() const {
		return data.channels
		     * av_get_bytes_per_sample((enum AVSampleFormat) data.format)
			 * data.nb_samples;
	}
};
	
}

#endif
