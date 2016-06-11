/*
 * OpenslSink.cpp
 *
 * Created on: 2016年5月15日
 *     Author: loushuai
 */

#include <stdlib.h>
#include "openslsink.hpp"
#include "log.hpp"

namespace whitebean
{

OpenslSink::OpenslSink()
	:mBuffer(nullptr)
	,mCookie(nullptr)
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
                      pcm_format_t format, AudioCallback cb,
					  void *cookie)
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

	mCookie = cookie;

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
    pcm.samplesPerSec = openslSampleRate(sampleRate);
	if (pcm.samplesPerSec == 0) {
		LOGE("Invalid sample rate");
		return -1;
	}
    //精度
    pcm.bitsPerSample = openslPcmFormat(format);
	if (pcm.bitsPerSample == 0) {
		LOGE("Invalid pcm format");
		return -1;
	}
	pcm.containerSize = pcm.bitsPerSample;
    
    if(pcm.numChannels == 2) {
    	pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    } else {
    	pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

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
	
	size = self->mCallBack(self->mBuffer, self->mCookie);

	LOGD("================================size %d", size);
	
	result = (*bq)->Enqueue(bq, self->mBuffer.get(), size);
	if(SL_RESULT_SUCCESS != result){
		LOGE("Enqueue failed");
	}

	LOGD("audio player callback exit");
}

SLuint32 OpenslSink::openslSampleRate(int32_t sr)
{
	SLuint32 ret = 0;
	
	switch(sr) {
	case 8000:
		ret = SL_SAMPLINGRATE_8;
		break;
	case 11025:
		ret = SL_SAMPLINGRATE_11_025;
		break;
	case 12000:
		ret = SL_SAMPLINGRATE_12;
		break;
	case 16000:
		ret = SL_SAMPLINGRATE_16;
		break;
	case 22050:
		ret = SL_SAMPLINGRATE_22_05;
		break;
	case 24000:
		ret = SL_SAMPLINGRATE_24;
		break;
	case 32000:
		ret = SL_SAMPLINGRATE_32;
		break;
	case 44100:
		ret = SL_SAMPLINGRATE_44_1;
		break;
	case 48000:
		ret = SL_SAMPLINGRATE_48;
		break;
	case 64000:
		ret = SL_SAMPLINGRATE_64;
		break;
	case 88200:
		ret = SL_SAMPLINGRATE_88_2;
		break;
	case 96000:
		ret = SL_SAMPLINGRATE_96;
		break;
	case 192000:
		ret = SL_SAMPLINGRATE_192;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

SLuint16 OpenslSink::openslPcmFormat(pcm_format_t format)
{
	SLuint16 ret = 0;

	switch(format) {
	case PCM_FORMAT_FIXED_8:
		ret = SL_PCMSAMPLEFORMAT_FIXED_8;
		break;
	case PCM_FORMAT_FIXED_16:
		ret = SL_PCMSAMPLEFORMAT_FIXED_16;
		break;
	case PCM_FORMAT_FIXED_20:
		ret = SL_PCMSAMPLEFORMAT_FIXED_20;
		break;
	case PCM_FORMAT_FIXED_24:
		ret = SL_PCMSAMPLEFORMAT_FIXED_24;
		break;
	case PCM_FORMAT_FIXED_28:
		ret = SL_PCMSAMPLEFORMAT_FIXED_28;
		break;
	case PCM_FORMAT_FIXED_32:
		ret = SL_PCMSAMPLEFORMAT_FIXED_32;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}
  
}
