/*
 * DataSource.hpp
 *
 *  Created on: 2016Äê5ÔÂ25ÈÕ
 *      Author: loushuai
 */
#ifndef JNI_MEDIAPLAYER_MEDIABASE_DATASOURCE_H_
#define JNI_MEDIAPLAYER_MEDIABASE_DATASOURCE_H_

#include <memory>


extern "C" {
#include "libavformat/avformat.h"
}

namespace whitebean {

class MetaData;

class DataSource {
public:
	DataSource(){}
	virtual ~DataSource(){}

	virtual int read() = 0;
	virtual std::unique_ptr<MetaData> getMetaData() = 0;
};
	
}

#endif
