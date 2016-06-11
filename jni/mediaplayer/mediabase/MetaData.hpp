/*
 * MetaData.hpp
 *
 *  Created on: 2016Äê5ÔÂ21ÈÕ
 *      Author: loushuai
 */

#ifndef JNI_MEDIAPLAYER_MEDIABASE_METADATA_H_
#define JNI_MEDIAPLAYER_MEDIABASE_METADATA_H_

#include <string>
#include <map>
#include <memory>

namespace whitebean {
// The following keys map to int32_t data unless indicated otherwise.
enum {
	kKeyMIMEType          = 'mime',  // cstring
    kKeyWidth             = 'widt',  // int32_t, image pixel
    kKeyHeight            = 'heig',  // int32_t, image pixel
    kKeyChannelCount      = '#chn',  // int32_t
    kKeySampleRate        = 'srte',  // int32_t (audio sampling rate Hz)
    kKeyFrameRate         = 'frmR',  // int32_t (video frame rate fps)
    kKeyBitRate           = 'brte',  // int32_t (bps)
    kKeyColorFormat       = 'colf',
	kKeyTime              = 'time',  // int64_t (usecs)
};

class MetaData {
public:
	MetaData();
	~MetaData();

    enum Type {
        TYPE_NONE     = 'none',
        TYPE_STRING   = 'cstr',
        TYPE_INT32    = 'in32',
        TYPE_INT64    = 'in64',
        TYPE_FLOAT    = 'floa',
        TYPE_POINTER  = 'ptr ',
    };	

	//
	void clear();

	bool remove(uint32_t key);

	bool setInt32(uint32_t key, int32_t value);
	bool setInt64(uint32_t key, int64_t value);
	bool setFloat(uint32_t key, float value);
	bool setString(uint32_t key, const char *value);

	bool findInt32(uint32_t key, int32_t &value);
	bool findInt64(uint32_t key, int64_t &value);
	bool findFloat(uint32_t key, float &value);
	bool findString(uint32_t key, std::string &value);

	bool setData(uint32_t key, uint32_t type,
				 const uint8_t *data, size_t size);
	bool findData(uint32_t key, uint32_t &type,
				  std::shared_ptr<uint8_t> &spData, size_t &size);

private:
	struct typed_data {
        typed_data();
        ~typed_data();

        void clear();
        void setData(uint32_t type, const uint8_t *data, size_t size);
        void getData(uint32_t &type, std::shared_ptr<uint8_t> &spData, size_t &size);
	private:
		uint32_t mType;
        size_t mSize;

		std::shared_ptr<uint8_t> mSpData;

	};
	
	std::map<uint32_t, typed_data> mItems;
};
	
}

#endif
