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
	
int Codec::open_l()
{
	AVCodec* codec;
	
	mTracksPtr = mSource->getTracksPtr();
	if (!mTracksPtr) {
		LOGE("Invalid tracks");
		return -1;
	}
	
	mAVFmtCtxPtr = mSource->getFmtCtxPtr();	
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
	
	codec = avcodec_find_decoder(mCodecPtr->codec_id);
	if (!codec) {
		LOGE("Can't find decoder");
		return -1;
	}

	LOGD("Video/Audio codec id %d", mCodecPtr->codec_id);

	if (avcodec_open2(mCodecPtr.get(), codec, NULL) < 0) {
		LOGE("open decoder failed");
		return -1;
	}

	initEvents();
	
	// start event queue
	mQueue.start();	
	
	return 0;
}

int Codec::start()
{
	onWaitEvent();
	return 0;
}

int Codec::stop()
{
	mQueue.stop();
	return 0;
}	

void Codec::clear()
{
	mQueue.postEvent(mEvents[EVENT_CLEAR]);
}

void Codec::clear_l()
{
	while (!mFrameQueue.empty()) {
		mFrameQueue.pop();
	}

	avcodec_flush_buffers(mCodecPtr.get());
}

void Codec::timeScaleToUs(FrameBuffer &frmbuf)
{
	if (mSource) {
		AVRational time_base = mSource->getTimeScaleOfTrack(mStreamId);
		frmbuf.setPts(frmbuf.getPts()*US_IN_SECOND*time_base.num/time_base.den);
	}
}

void Codec::initEvents()
{
	mEvents[EVENT_WAIT] = shared_ptr<TimedEventQueue::Event> (new MediaEvent<Codec>(
															  this, &Codec::onWaitEvent));
	mEvents[EVENT_WORK] = shared_ptr<TimedEventQueue::Event> (new MediaEvent<Codec>(
															  this, &Codec::onWorkEvent));
	mEvents[EVENT_CLEAR] = shared_ptr<TimedEventQueue::Event> (new MediaEvent<Codec>(
															  this, &Codec::onClearEvent));	
	mEvents[EVENT_EXIT] = shared_ptr<TimedEventQueue::Event> (new MediaEvent<Codec>(
															  this, &Codec::onExitEvent));
}
	
void Codec::onWaitEvent()
{
	if (waiting()) {
		this_thread::sleep_for(chrono::milliseconds(10));		
		mQueue.postEvent(mEvents[EVENT_WAIT]);
	} else {
		mQueue.postEvent(mEvents[EVENT_WORK]);
	}
}

void Codec::onWorkEvent()
{
	int ret = 0;

	ret = decode();
	if (ret == ERR_AGAIN) {
		this_thread::sleep_for(chrono::milliseconds(10));		
	}

	mQueue.postEvent(mEvents[EVENT_WORK]);
}

void Codec::onClearEvent()
{
	mQueue.cancelEvent(mEvents[EVENT_WORK]->eventID());
	
	clear_l();

	halt();
	mQueue.postEvent(mEvents[EVENT_WAIT]);
	
	mListener->mediaNotify(IMediaListener::DECODER_CLEAR_COMPLETE, mStreamId);
}

void Codec::onExitEvent()
{

}

AudioDecoder::AudioDecoder()
: mSampleRate(0)
, mChannels(0)
, mSampleFmt(0)
{
}

int AudioDecoder::open(shared_ptr<MediaSource> source)
{
	mSource = source;
	mStreamId = mSource->getAudioStreamId();

	if (open_l() < 0) {
		LOGE("Open audio codec failed");
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

int AudioDecoder::decode()
{
	int ret = 0;
	int gotframe = 0;

	LOGD("Decode Audio");

	if (mFrameQueue.full()) {
		LOGD("Audio decoder frame queue full");
		this_thread::sleep_for(chrono::milliseconds(10));
		return ERR_AGAIN;
	}

	PacketBuffer pktbuf;
	if (!mTracksPtr->readAudio(pktbuf) || pktbuf.empty()) {
		LOGD("Audio decoder read packet failed");
		this_thread::sleep_for(chrono::milliseconds(10));
		return ERR_AGAIN;
	}

	LOGD("Audio decoder read packet ok");

	FrameBuffer frmbuf, filtfrmbuf;

	ret = avcodec_decode_audio4(mCodecPtr.get(), frmbuf.getDataPtr(), &gotframe, pktbuf.getDataPtr());
	if (ret < 0) {
		LOGE("Audio decode error %d", ret);
		return ERR_AGAIN;
	}
	if (!gotframe) {
		LOGE("Audio decode failed");
		return ERR_AGAIN;
	}

	LOGD("Audio decoder frame success");

	if (av_buffersrc_add_frame_flags(mFilterCtx.bufferSrcCtx, frmbuf.getDataPtr(), 0) < 0) {
		LOGE("Error while feeding the audio filtergraph");
		return ERR_INVALID;
	}

	LOGD("Audio add filter frame success");

	while (1) {
		ret = av_buffersink_get_frame(mFilterCtx.bufferSinkCtx, filtfrmbuf.getDataPtr());
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		if (ret < 0)
			return ERR_INVALID;
	}

	LOGD("Audio filter frame success");

	timeScaleToUs(filtfrmbuf);
	mFrameQueue.push(filtfrmbuf);	
	
	return 0;
}

VideoDecoder::VideoDecoder()
: mWidth(0)
, mHeight(0)  
{

}

int VideoDecoder::open(shared_ptr<MediaSource> source)
{
	mSource = source;
	mStreamId = mSource->getVideoStreamId();

	if (open_l() < 0) {
		LOGE("Open video codec failed");
		return -1;
	}

	mWidth = mCodecPtr->width;
	mHeight = mCodecPtr->height;

	if (initFilters() < 0) {
		LOGD("Init video filter failed");
		return -1;
	}

	return 0;
}

bool VideoDecoder::read(FrameBuffer &frmbuf)
{
	if (mFrameQueue.empty()) {
		frmbuf.reset();
		return false;
	}

	frmbuf = mFrameQueue.front();
	mFrameQueue.pop();
	
	return true;
}

int VideoDecoder::decode()
{
	int ret = 0;
	int gotframe = 0;

	LOGD("Decode Video");

	if (mFrameQueue.full()) {
		LOGD("Video decoder frame queue full");
		this_thread::sleep_for(chrono::milliseconds(10));
		return ERR_AGAIN;
	}

	PacketBuffer pktbuf;
	if (!mTracksPtr->readVideo(pktbuf) || pktbuf.empty()) {
		LOGD("Video decoder read packet failed");
		this_thread::sleep_for(chrono::milliseconds(10));
		return ERR_AGAIN;
	}

	LOGD("Audio decoder read packet ok");

	FrameBuffer frmbuf, filtfrmbuf;

	ret = avcodec_decode_video2(mCodecPtr.get(), frmbuf.getDataPtr(), &gotframe, pktbuf.getDataPtr());
	if (ret < 0) {
		LOGE("Video decode error %d", ret);
		return ERR_AGAIN;
	}
	if (!gotframe) {
		LOGE("Video decode failed");
		return ERR_AGAIN;
	}

	LOGD("Video decoder frame success");

	if (av_buffersrc_add_frame_flags(mFilterCtx.bufferSrcCtx, frmbuf.getDataPtr(), 0) < 0) {
		LOGE("Error while feeding the audio filtergraph");
		return ERR_INVALID;
	}

	LOGD("Video add filter frame success");

	while (1) {
		ret = av_buffersink_get_frame(mFilterCtx.bufferSinkCtx, filtfrmbuf.getDataPtr());
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		if (ret < 0)
			return ERR_INVALID;
	}

	LOGD("Video filter frame success");

	timeScaleToUs(filtfrmbuf);
	mFrameQueue.push(filtfrmbuf);	
	
	return 0;
}	

int VideoDecoder::initFilters()
{
	char args[512];
    int ret = 0;
    AVFilter *vbuffersrc  = avfilter_get_by_name("buffer");
    AVFilter *vbuffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

	mFilterCtx.filterGraph = shared_ptr<AVFilterGraph>(avfilter_graph_alloc(),
											 [](AVFilterGraph *p){avfilter_graph_free(&p);});
    if (!inputs || !outputs || !mFilterCtx.filterGraph) {
    	LOGE("Alloc filter graph failed");
		ret = -1;
    	goto end;
    }

	snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			 mCodecPtr->width, mCodecPtr->height,
			 mCodecPtr->pix_fmt,
			 mCodecPtr->time_base.num,
			 mCodecPtr->time_base.den,
			 mCodecPtr->sample_aspect_ratio.num,
			 mCodecPtr->sample_aspect_ratio.den);
	LOGD("Buffer args: %s", args);

    ret = avfilter_graph_create_filter(&mFilterCtx.bufferSrcCtx, vbuffersrc, "in",
                                       args, NULL, mFilterCtx.filterGraph.get());
	if (ret < 0) {
		LOGE("Create filter in failed");
		goto end;
	}

    ret = avfilter_graph_create_filter(&mFilterCtx.bufferSinkCtx, vbuffersink, "out",
                                       NULL, NULL, mFilterCtx.filterGraph.get());
	if (ret < 0) {
		LOGE("Create filter out failed");		
		goto end;
	}

    ret = av_opt_set_int_list(mFilterCtx.bufferSinkCtx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
    	LOGE("Cannot set output pixel format");
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
	snprintf(args, sizeof(args), "format=pix_fmts=yuv420p");
	LOGD("audio filter output desc %s", args);
    if ((ret = avfilter_graph_parse_ptr(mFilterCtx.filterGraph.get(), args,
                                        &inputs, &outputs, NULL)) < 0) {
		LOGE("Graph parse failed");
		goto end;
	}

    if ((ret = avfilter_graph_config(mFilterCtx.filterGraph.get(), NULL)) < 0) {
		LOGE("Graph config failed");
        goto end;
	}	
end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
	
	return ret;
}

int MediaDecoder::open(shared_ptr<MediaSource> source, int streamid)
{
	if (streamid == source->getAudioStreamId()) {
		mDelegatePtr = unique_ptr<Codec>(new AudioDecoder);
	} else if (streamid == source->getVideoStreamId()) {
	   	mDelegatePtr = unique_ptr<Codec>(new VideoDecoder);
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
