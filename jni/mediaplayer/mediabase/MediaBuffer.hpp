/*
 * DataSource.hpp
 *
 *  Created on: 2016Äê5ÔÂ25ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_MEDIABUFFER_H_
#define JNI_MEDIAPLAYER_MEDIABASE_MEDIABUFFER_H_

#include <memory>
#include "MetaData.hpp"

namespace whitebean {

class MetaData;

template <typename T>	
class MediaBuffer {
public:
	virtual ~MediaBuffer(){}

	virtual const T& getData() const = 0;
	virtual const T* getDataPtr() const = 0;
	virtual T* getDataPtr() = 0;

protected:
	T data;
};
	
}

#endif
