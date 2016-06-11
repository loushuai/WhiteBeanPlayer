/*
 * MediaCodec.cpp
 *
 *  Created on: 2016Äê5ÔÂ31ÈÕ
 *      Author: loushuai
 */

#include <cstring>
#include "MediaCodec.hpp"
#include "log.hpp"

using namespace std;

namespace whitebean {

int Codec::open(std::shared_ptr<AVCodecContext> codecCtxPtr)
{
	AVCodec* codec;
	
	if (!codecCtxPtr) {
		LOGE("Input codec is NULL");
		return -1;
	}
	
	codec = avcodec_find_decoder(codecCtxPtr->codec_id);
	if (!codec) {
		LOGE("Can't find decoder");
		return -1;
	}

	LOGD("Audio codec id %d", codecCtxPtr->codec_id);

	if (avcodec_open2(codecCtxPtr.get(), codec, NULL) < 0) {
		LOGE("open decoder failed");
		return -1;
	}

	return 0;
}

AudioDecoder::AudioDecoder()
: mSampleRate(0)
, mChannels(0)
, mSampleFmt(0)
{
}

int AudioDecoder::open(shared_ptr<MediaSource> source)
{
	mStreamId = source->getAudioStreamId();

	mTracksPtr = source->getTracksPtr();
	if (!mTracksPtr) {
		LOGE("Invalid tracks");
		return -1;
	}
	
	mAVFmtCtxPtr = source->getFmtCtxPtr();	
	if (mStreamId < 0 || mStreamId >= mAVFmtCtxPtr->nb_streams) {
		LOGE("Invalid stream");
		return -1;
	}

	mCodecPtr = shared_ptr<AVCodecContext>(mAVFmtCtxPtr->streams[mStreamId]->codec,
										   [](AVCodecContext *p){avcodec_close(p);});
	if(!mCodecPtr) {
		LOGE("Invalid codec");
		return -1;
	}

	if (Codec::open(mCodecPtr) < 0) {
		LOGE("Open codec failed");
		return -1;
	}

	mChannels = mCodecPtr->channels;
	mSampleRate = mCodecPtr->sample_rate;
	mSampleFmt = mCodecPtr->sample_fmt;

	mMetaData.setInt32(kKeySampleRate, mSampleRate);
		
	if (initFilters() < 0) {
		LOGE("Init filters failed");
		return -1;
	}

	return 0;
}

bool AudioDecoder::read(FrameBuffer &frmbuf)
{
	if (mFrameQueue.empty()) {
		return false;
	}

	frmbuf = mFrameQueue.front();
	mFrameQueue.pop();
	
	return true;
}

int AudioDecoder::initFilters()
{
	char args[512];
	int ret = 0;
    AVFilter *abuffersrc  = avfilter_get_by_name("abuffer");
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    static const enum AVSampleFormat out_sample_fmts[] = { AV_SAMPLE_FMT_S16, static_cast<AVSampleFormat>(-1) };
    static const int64_t out_channel_layouts[] = { AV_CH_LAYOUT_MONO, AV_CH_LAYOUT_STEREO, -1 };
    static const int out_sample_rates[] = { mSampleRate, -1 };
    AVRational time_base = mAVFmtCtxPtr->streams[mStreamId]->time_base;
	
	mFilterCtx.filterGraph = shared_ptr<AVFilterGraph>(avfilter_graph_alloc(),
											 [](AVFilterGraph *p){avfilter_graph_free(&p);});
	if (!outputs || !inputs || !mFilterCtx.filterGraph) {
		ret = -1;
		goto end;
	}

	if (!mCodecPtr->channel_layout) {
		mCodecPtr->channel_layout = av_get_default_channel_layout(mChannels);
	}

    snprintf(args, sizeof(args),
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llx",
             time_base.num, time_base.den, mCodecPtr->sample_rate,
             av_get_sample_fmt_name(mCodecPtr->sample_fmt), mCodecPtr->channel_layout);
	LOGD("audio filter input desc %s", args);
    ret = avfilter_graph_create_filter(&mFilterCtx.bufferSrcCtx, abuffersrc, "in",
                                       args, NULL, mFilterCtx.filterGraph.get());
	if (ret < 0) {
		goto end;
	}

    ret = avfilter_graph_create_filter(&mFilterCtx.bufferSinkCtx, abuffersink, "out",
                                       NULL, NULL, mFilterCtx.filterGraph.get());
	if (ret < 0) {
		goto end;
	}

    ret = av_opt_set_int_list(mFilterCtx.bufferSinkCtx, "sample_fmts", out_sample_fmts, -1,
                              AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		goto end;
	}

    ret = av_opt_set_int_list(mFilterCtx.bufferSinkCtx, "channel_layouts", out_channel_layouts, -1,
							  AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		goto end;
	}

    ret = av_opt_set_int_list(mFilterCtx.bufferSinkCtx, "sample_rates", out_sample_rates, -1,
                              AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		goto end;
	}

    outputs->name       = av_strdup("in");
    outputs->filter_ctx = mFilterCtx.bufferSrcCtx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = mFilterCtx.bufferSinkCtx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

	//args is reused
	snprintf(args, sizeof(args),
			 "aresample=%d,aformat=sample_fmts=s16:channel_layouts=stereo",
			 mSampleRate);
	LOGD("audio filter output desc %s", args);
    if ((ret = avfilter_graph_parse_ptr(mFilterCtx.filterGraph.get(), args,
                                        &inputs, &outputs, NULL)) < 0) {
		goto end;
	}

    if ((ret = avfilter_graph_config(mFilterCtx.filterGraph.get(), NULL)) < 0) {
        goto end;
	}
end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
	
	return ret;
}

void AudioDecoder::threadEntry()
{
	int ret = 0;
	int gotframe = 0;

	LOGD("Audio decoder loop enter");
	
	for (;;) {
		if (mStopped) {
			break;
		}

		if (mFrameQueue.full()) {
			LOGD("Audio decoder frame queue full");
			this_thread::sleep_for(chrono::milliseconds(10));
			continue;			
		}

		PacketBuffer pktbuf;
		if (!mTracksPtr->readAudio(pktbuf) || pktbuf.empty()) {
			LOGD("Audio decoder read packet failed");
			this_thread::sleep_for(chrono::milliseconds(10));
			continue;
		}

		LOGD("Audio decoder read packet ok");

		FrameBuffer frmbuf, filtfrmbuf;
		
		ret = avcodec_decode_audio4(mCodecPtr.get(), frmbuf.getDataPtr(), &gotframe, pktbuf.getDataPtr());
		if (ret < 0) {
			LOGE("Audio decode error %d", ret);
			continue;
		}
		if (!gotframe) {
			LOGE("Audio decode failed");
			continue;
		}

		LOGD("Audio decoder frame success");

		if (av_buffersrc_add_frame_flags(mFilterCtx.bufferSrcCtx, frmbuf.getDataPtr(), 0) < 0) {
			LOGE("Error while feeding the audio filtergraph");
			break;
		}

		LOGD("Audio add filter frame success");

		while (1) {
			ret = av_buffersink_get_frame(mFilterCtx.bufferSinkCtx, filtfrmbuf.getDataPtr());
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;
			if (ret < 0)
				goto end;
		}

		LOGD("Audio filter frame success");

		mFrameQueue.push(filtfrmbuf);
	}
 end:
	LOGD("Audio decoder loop exit");
}

bool VideoDecoder::read(FrameBuffer &frmbuf)
{

	return 0;
}

void VideoDecoder::threadEntry()
{

}

int MediaDecoder::open(shared_ptr<MediaSource> source, int streamid)
{
	if (streamid == source->getAudioStreamId()) {
		mDelegatePtr = unique_ptr<Codec>(new AudioDecoder);
	} else if (streamid == source->getVideoStreamId()) {
		//	   	mDelegatePtr = unique_ptr<Codec>(new VideoDecoder);
	} else {
		LOGE("Unknown stream");
		return -1;
	}

	if (!mDelegatePtr) {
		return -1;
	}

	if (mDelegatePtr->open(source) < 0) {
		LOGE("Open decoder failed");
		return -1;
	}

	return 0;
}
	
}
