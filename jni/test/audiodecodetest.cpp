/*
 * audiodecodetest.cpp
 *
 *  Created on: 2016年6月7日
 *      Author: leon
 */

#include <catch.hpp>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#define IN_FILE_NAME "/data/local/tmp/video.mp4"
#define OUT_FILE_NAME "/data/local/tmp/test.pcm"

static FILE *pf;

TEST_CASE("AudioDecode")
{
	int ret = 0;
	int audioStreamId = -1;
	int sampleFmt = -1, channels = -1;

	av_register_all();
	avcodec_register_all();

	AVFormatContext *fmtptr = nullptr;
	ret = avformat_open_input(&fmtptr, IN_FILE_NAME, NULL, NULL);
	REQUIRE(ret == 0);
	REQUIRE(fmtptr != nullptr);

	ret = avformat_find_stream_info(fmtptr, NULL);
	REQUIRE(ret == 0);

	for (int i = 0; i < fmtptr->nb_streams; i++) {
		if (AVMEDIA_TYPE_AUDIO == fmtptr->streams[i]->codec->codec_type) {
			printf("audio stream %d\n", i);
			audioStreamId = i;
			break;
		}
	}

	REQUIRE(audioStreamId != -1);

	AVCodec* codec = nullptr;
	AVCodecContext *codecCtx = fmtptr->streams[audioStreamId]->codec;

	codec = avcodec_find_decoder(codecCtx->codec_id);
	REQUIRE(codec != nullptr);

	ret = avcodec_open2(codecCtx, codec, NULL);
	REQUIRE(ret == 0);

	sampleFmt = codecCtx->sample_fmt;
	channels = codecCtx->channels;
	printf("sample fmt %d, channels %d\n", sampleFmt, channels);

	int completed = 0;
	int size = 0;

	pf = fopen(OUT_FILE_NAME, "wb");
	for (int i = 0; i < 100; ++i) {
		AVPacket packet;
		AVFrame frame;

		av_init_packet(&packet);
		memset(&frame, 0, sizeof(AVFrame));
		av_frame_unref(&frame);

		ret = av_read_frame(fmtptr, &packet);
		REQUIRE(ret == 0);

		if (packet.stream_index != audioStreamId) {
			continue;
		}

		ret = avcodec_decode_audio4(codecCtx, &frame, &completed, &packet);
		REQUIRE(ret > 0);

		size = frame.channels * av_get_bytes_per_sample((enum AVSampleFormat) frame.format) * frame.nb_samples;
		fwrite(frame.data[0], size, 1, pf);

		av_frame_unref(&frame);
		av_packet_unref(&packet);
	}
}
