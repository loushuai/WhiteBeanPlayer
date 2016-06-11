/*
 * MetaData.cpp
 *
 *  Created on: 2016Äê5ÔÂ21ÈÕ
 *      Author: loushuai
 */

#include "MetaData.hpp"

using namespace std;

namespace whitebean {

MetaData::MetaData()
{

}

MetaData::~MetaData()
{
	clear();
}

void MetaData::clear()
{
	mItems.clear();
}

bool MetaData::remove(uint32_t key)
{
	if (mItems.erase(key) == 0) {
		return false;
	} else {
		return true;
	}
}

bool MetaData::setInt32(uint32_t key, int32_t value)
{
	return setData(key, TYPE_INT32, reinterpret_cast<const uint8_t *>(&value), sizeof(value));
}

bool MetaData::setInt64(uint32_t key, int64_t value)
{
	return setData(key, TYPE_INT64, reinterpret_cast<const uint8_t *>(&value), sizeof(value));
}

bool MetaData::setFloat(uint32_t key, float value)
{
	return setData(key, TYPE_FLOAT, reinterpret_cast<const uint8_t *>(&value), sizeof(value));
}

bool MetaData::setString(uint32_t key, const char *value)
{
	return setData(key, TYPE_STRING, reinterpret_cast<const uint8_t *>(value), strlen(value) + 1);
}

bool MetaData::findInt32(uint32_t key, int32_t &value)
{
	uint32_t type;
	shared_ptr<uint8_t> spData;
	size_t size;
	if (!findData(key, type, spData, size) || type != TYPE_INT32
		|| size != sizeof(value)) {
		return false;
	}

	value = *reinterpret_cast<int32_t *>(spData.get());

	return true;
}

bool MetaData::findInt64(uint32_t key, int64_t &value)
{
	uint32_t type;
	shared_ptr<uint8_t> spData;
	size_t size;
	if (!findData(key, type, spData, size) || type != TYPE_INT64
		|| size != sizeof(value)) {
		return false;
	}

	value = *reinterpret_cast<int64_t *>(spData.get());

	return true;
}

bool MetaData::findFloat(uint32_t key, float &value)
{
	uint32_t type;
	shared_ptr<uint8_t> spData;
	size_t size;
	if (!findData(key, type, spData, size) || type != TYPE_FLOAT
		|| size != sizeof(value)) {
		return false;
	}

	value = *reinterpret_cast<float *>(spData.get());

	return true;
}

bool MetaData::findString(uint32_t key, string &value)
{
	uint32_t type;
	shared_ptr<uint8_t> spData;
	size_t size;
	if (!findData(key, type, spData, size) || type != TYPE_STRING) {
		return false;
	}

	value = string(reinterpret_cast<char *>(spData.get()));

	return true;
}

bool MetaData::setData(uint32_t key, uint32_t type,
					   const uint8_t *data, size_t size)
{
    bool overwrote_existing = true;
	typed_data item;
	auto ret = mItems.insert(pair<uint32_t, typed_data>(key, item));
	if (ret.second == true) {
		overwrote_existing = false;
	}

	auto it = ret.first;
	it->second.setData(type, data, size);

	return overwrote_existing;
}
	
bool MetaData::findData(uint32_t key, uint32_t &type,
						shared_ptr<uint8_t> &spData, size_t &size)
{
	map<uint32_t, typed_data>::iterator it;
	it = mItems.find(key);
	if (it == mItems.end()) {
		return false;
	}

	it->second.getData(type, spData, size);
	return true;
}

MetaData::typed_data::typed_data()
: mType(0)
, mSize(0)
{
}

MetaData::typed_data::~typed_data()
{
	clear();
}

void MetaData::typed_data::clear()
{
	mSize = 0;	
	mType = 0;
	mSpData.reset();
}

void MetaData::typed_data::setData(uint32_t type, const uint8_t *data, size_t size)
{
	mSpData.reset(new uint8_t[size], [](uint8_t *p) {delete[] p;});
	if (!mSpData) {
		mType = 0;
		mSize = 0;
		return;
	}

	uninitialized_copy_n(data, size, mSpData.get());
	mType = type;
	mSize = size;
}

void MetaData::typed_data::getData(uint32_t &type, shared_ptr<uint8_t> &spData, size_t &size){
	type = mType;
	size = mSize;
	spData = mSpData;
}
	
}
