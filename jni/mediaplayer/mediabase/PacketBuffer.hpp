/*
 * PacketBuffer.hpp
 *
 *  Created on: 2016Äê5ÔÂ25ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_PACKETBUFFER_H_
#define JNI_MEDIAPLAYER_MEDIABASE_PACKETBUFFER_H_

#include "MediaBuffer.hpp"

extern "C" {
#include "libavcodec/avcodec.h"
}

namespace whitebean {

class PacketBuffer : MediaBuffer<AVPacket> {
public:
	PacketBuffer() {
		av_init_packet(&data);
	}

	PacketBuffer(const AVPacket &pkt) {
		av_init_packet(&data);
		if (av_packet_ref(&data, &pkt) != 0) {
			data.size = 0;
			data.data = nullptr;
		}
	}

	PacketBuffer(const PacketBuffer &pktbuf) {
		av_init_packet(&data);
		if (av_packet_ref(&data, &(pktbuf.data)) != 0) {
			data.size = 0;
			data.data = nullptr;
		}
	}

	PacketBuffer& operator=(PacketBuffer &rhs) {
		if (data.data != rhs.data.data) {
			av_packet_unref(&data);
			if (av_packet_ref(&data, &(rhs.data)) != 0) {
				data.size = 0;
				data.data = nullptr;			
			}
		}
		return *this;
	}

	~PacketBuffer() {
		av_packet_unref(&data);
	}

	const AVPacket& getData() const {
		return data;
	}

	const AVPacket *getDataPtr() const {
		return &data;
	}

	AVPacket *getDataPtr() {
		return &data;
	}

	bool empty() const{
		return data.size == 0 || data.data == nullptr;
	}
};
	
}

#endif
