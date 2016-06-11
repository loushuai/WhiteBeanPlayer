/*
 * metadatatest.cpp
 *
 *  Created on: 2016Äê5ÔÂ22ÈÕ
 *      Author: loushuai
 */

#include <catch.hpp>
#include <iostream>
#include <string>
#include "MetaData.hpp"

using namespace std;
using namespace whitebean;

TEST_CASE("MetaData")
{
	MetaData metaData;
	bool ret;

	SECTION("int32") {
		int32_t val;
		ret = metaData.setInt32(kKeyWidth, 1234);
		CHECK(ret == false);
		ret = metaData.findInt32(kKeyWidth, val);
		CHECK(ret == true);
		CHECK(val == 1234);
	}

	SECTION("int32 overwrite") {
		int32_t val;
		ret = metaData.setInt32(kKeyWidth, 1234);
		CHECK(ret == false);
		ret = metaData.findInt32(kKeyWidth, val);
		CHECK(ret == true);
		CHECK(val == 1234);
		ret = metaData.setInt32(kKeyWidth, 5678);
		CHECK(ret == true);
		ret = metaData.findInt32(kKeyWidth, val);
		CHECK(ret == true);
		CHECK(val == 5678);
	}

	SECTION("int64") {
		int64_t val;
		ret = metaData.setInt64(kKeyWidth, 1234);
		CHECK(ret == false);
		ret = metaData.findInt64(kKeyWidth, val);
		CHECK(ret == true);
		CHECK(val == 1234);
	}

	SECTION("int64 overwrite") {
		int64_t val;
		ret = metaData.setInt64(kKeyWidth, 1234);
		CHECK(ret == false);
		ret = metaData.findInt64(kKeyWidth, val);
		CHECK(ret == true);
		CHECK(val == 1234);
		ret = metaData.setInt64(kKeyWidth, 5678);
		CHECK(ret == true);
		ret = metaData.findInt64(kKeyWidth, val);
		CHECK(ret == true);
		CHECK(val == 5678);
	}

	SECTION("float") {
		float val;
		ret = metaData.setFloat(kKeyWidth, 0.1234);
		CHECK(ret == false);
		ret = metaData.findFloat(kKeyWidth, val);
		CHECK(ret == true);
		CHECK(val == 0.1234f);
	}

	SECTION("float overwrite") {
		float val;
		ret = metaData.setFloat(kKeyWidth, 0.1234);
		CHECK(ret == false);
		ret = metaData.findFloat(kKeyWidth, val);
		CHECK(ret == true);
		CHECK(val == 0.1234f);
		ret = metaData.setFloat(kKeyWidth, 0.5678);
		CHECK(ret == true);
		ret = metaData.findFloat(kKeyWidth, val);
		CHECK(ret == true);
		CHECK(val == 0.5678f);
	}

	SECTION("string") {
		string val;
		ret = metaData.setString(kKeyMIMEType, "Hello World!");
		CHECK(ret == false);
		ret = metaData.findString(kKeyMIMEType, val);
		CHECK(ret == true);
		CHECK(val.compare("Hello World!") == 0);
	}

	SECTION("string overwrite") {
		string val;
		ret = metaData.setString(kKeyMIMEType, "Hello World!");
		CHECK(ret == false);
		ret = metaData.findString(kKeyMIMEType, val);
		CHECK(ret == true);
		CHECK(val.compare("Hello World!") == 0);
		ret = metaData.setString(kKeyMIMEType, "Hello World Again!");
		CHECK(ret == true);
		ret = metaData.findString(kKeyMIMEType, val);
		CHECK(ret == true);
		CHECK(val.compare("Hello World Again!") == 0);
	}

	metaData.clear();
}
