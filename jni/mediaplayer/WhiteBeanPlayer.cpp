/*
 * WhiteBeanPlayer.cpp
 *
 *  Created on: 2016Äê5ÔÂ21ÈÕ
 *      Author: loushuai
 */

#include "WhiteBeanPlayer.hpp"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfiltergraph.h"	
}

namespace whitebean {

WhiteBeanPlayer::WhiteBeanPlayer()
{
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();	
}

WhiteBeanPlayer::~WhiteBeanPlayer()
{

}
	
}
