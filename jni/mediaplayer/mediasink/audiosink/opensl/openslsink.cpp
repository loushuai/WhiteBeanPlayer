/*
 * OpenslSink.cpp
 *
 * Created on: 2016年5月15日
 *     Author: loushuai
 */

#include <stdlib.h>
#include "openslsink.hpp"
#include "log.h"

namespace bigbean
{

OpenslSink::OpenslSink()
	:mBuffer(nullptr)
	,mBufferSize(0)
{

}

OpenslSink::~OpenslSink()
{
	//释放混合器
	if (mOutputMixObject != NULL) {
		(*mOutputMixObject)->Destroy(mOutputMixObject);
		mOutputMixObject = NULL;
	}

	//释放音频引擎
	if (mEngineObject != NULL) {
		(*mEngineObject)->Destroy(mEngineObject);
		mEngineObject = NULL;
		mEngine = NULL;
	}
}

int OpenslSink::open( uint32_t sampleRate, int channelCount,
                      pcm_format_t format, AudioCallback cb)
{	
	int ret = -1;

	LOGD("open enter");
	
	if (!cb) {
		return -1;
	}
	
	mCallBack = cb;
	if (CreateEngine() < 0) {
		return -1;
	}
	
	if (createBufferQueueAudioPlayer(sampleRate, channelCount, format) < 0) {
		return -1;
	}

	LOGD("open exit");
	
	return 0;
}

int OpenslSink::start()
{
	LOGD("start enter");
	AudioPlayerCallback(mPlayerBufferQueue, this);
	LOGD("start exit");
	return 0;
}

int OpenslSink::CreateEngine()
{
	SLresult result;
	const SLInterfaceID ids[1] = {SL_IID_VOLUME};
	const SLboolean req[1] = {SL_BOOLEAN_FALSE};

	// create engine
	result = slCreateEngine(&mEngineObject, 0, NULL, 0, NULL, NULL);
	if(SL_RESULT_SUCCESS != result){
		LOGE("slCreateEngine failed");
		mEngineObject = NULL;
		return -1;
	}

	// realize the engine
    result = (*mEngineObject)->Realize(mEngineObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
    	LOGE("(*mEngineObject)->Realize failed");
    	goto failed;
    }

	// get the engine interface, which is needed in order to create other objects
    result = (*mEngineObject)->GetInterface(mEngineObject, SL_IID_ENGINE, &mEngine);
    if(SL_RESULT_SUCCESS != result){
       	LOGE("Get SL_IID_ENGINE failed");
       	goto failed;
    }

	// realize the output mix
	result = (*mEngine)->CreateOutputMix(mEngine, &mOutputMixObject, 1, ids, req);
	if(SL_RESULT_SUCCESS != result){
		LOGE("CreateOutputMix failed");
		mOutputMixObject = NULL;
		goto failed;
	}

	result = (*mOutputMixObject)->Realize(mOutputMixObject, SL_BOOLEAN_FALSE);
	if(SL_RESULT_SUCCESS != result){
		LOGE("(*mOutputMixObject)->Realize");
		goto failed;
	}
	
	return 0;
 failed:
	if (mEngineObject) {
    	(*mEngineObject)->Destroy(mEngineObject);
    	mEngineObject = NULL;
	}

	if (mOutputMixObject) {
		(*mOutputMixObject)->Destroy(mOutputMixObject);
		mOutputMixObject = NULL;
	}

	mEngine = NULL;	
	
	return -1;
}

int OpenslSink::createBufferQueueAudioPlayer(uint32_t sampleRate,
											 int channelCount,
											 pcm_format_t format)
{
	SLresult result;
	
	if (!mEngine || !mOutputMixObject) {
		LOGE("Engine not ready now");
		return -1;
	}

    //配置音频播放参数
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 1};
    SLDataFormat_PCM pcm;
    //数据源格式
    pcm.formatType = SL_DATAFORMAT_PCM;
    //声道数
    pcm.numChannels = channelCount;
    //每秒采样数
    pcm.samplesPerSec = sampleRate * 1000;
    //精度
    pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    if(pcm.numChannels == 2) {
    	pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    } else {
    	pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

	// 2 seconds buffer
	mBufferSize = 2 * pcm.numChannels * pcm.samplesPerSec * pcm.bitsPerSample / 8;
	mBuffer = malloc(mBufferSize);
	if (!mBuffer) {
		return -1;
	}

	SLDataSource audioSrc = {&loc_bufq, &pcm};

	//音频sink参数
	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, mOutputMixObject};
	SLDataSink audioSnk = {&loc_outmix, NULL};

	//创建音频播放器
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
								  SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
							  SL_BOOLEAN_TRUE};

	result = (*mEngine)->CreateAudioPlayer(mEngine, &mPlayerObject, &audioSrc, &audioSnk,
			sizeof( ids ) / sizeof( *ids ), ids, req);

	if(SL_RESULT_SUCCESS != result){
		LOGE("CreateAudioPlayer failed");
		goto failed;
	}

	//调整播放器
	result = (*mPlayerObject)->Realize(mPlayerObject, SL_BOOLEAN_FALSE);
	if(SL_RESULT_SUCCESS != result){
		LOGE("(*mPlayerObject)->Realize failed");
		goto failed;
	}

	//获得播放接口
	result = (*mPlayerObject)->GetInterface(mPlayerObject, SL_IID_PLAY, &mPlayerPlay);
	if(SL_RESULT_SUCCESS != result){
		LOGE("(*mPlayerObject)->GetInterface SL_IID_PLAY failed");
		goto failed;
	}

	result = (*mPlayerObject)->GetInterface(mPlayerObject, SL_IID_BUFFERQUEUE,
			&mPlayerBufferQueue);
	if(SL_RESULT_SUCCESS != result){
		LOGE("(*mPlayerObject)->GetInterface SL_IID_BUFFERQUEUE failed");
		goto failed;
	}

	// get volume interface
	result = (*mPlayerObject)->GetInterface(mPlayerObject, SL_IID_VOLUME, &mPlayerVolume);
	if (SL_RESULT_SUCCESS != result) {
		LOGE("(*mPlayerObject)->GetInterface SL_IID_VOLUME failed");
		goto failed;
	}
	
	//注册回调函数
	result = (*mPlayerBufferQueue)->RegisterCallback(mPlayerBufferQueue, AudioPlayerCallback, this);
	if(SL_RESULT_SUCCESS != result){
		LOGE("mPlayerBufferQueue RegisterCallback failed");
		goto failed;
	}
	
	//设置播放状态
	result = (*mPlayerPlay)->SetPlayState(mPlayerPlay, SL_PLAYSTATE_PLAYING);
	if(SL_RESULT_SUCCESS != result){
		LOGE("SetPlayState failed");
		goto failed;
	}	
	
	return 0;
 failed:
	if (mBuffer) {
		free(mBuffer);
		mBuffer = NULL;
	}
	
	if (mPlayerObject) {
		(*mPlayerObject)->Destroy(mPlayerObject);
		mPlayerObject = NULL;
	}
	return -1;
}

void OpenslSink::AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	SLresult result;
	int32_t size;

	LOGD("audio player callback enter");

	OpenslSink *self = (OpenslSink *)context;
	if (!self) {
		return;
	}

	if (!bq) {
		LOGD("bq is NULL");
		return;
	}
	
	size = self->mCallBack(&(self->mBuffer), &(self->mBufferSize));
	result = (*bq)->Enqueue(bq, self->mBuffer, size);
	if(SL_RESULT_SUCCESS != result){
		LOGE("Enqueue failed");
	}

	LOGD("audio player callback exit");
}

  
}
