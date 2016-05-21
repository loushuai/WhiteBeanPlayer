# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPPFLAGS += -std=c++11 -D__STDC_CONSTANT_MACROS 

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_MODULE := bigbean
LOCAL_SRC_FILES += mediaplayer/TimedEventQueue.cpp \
                   mediaplayer/mediasink/audiosink/opensl/openslsink.cpp

LOCAL_LDLIBS    += -lOpenSLES

include $(BUILD_SHARED_LIBRARY)

#build test
ifeq ($(TEST), 1)
include $(CLEAR_VARS)
LOCAL_CPPFLAGS += -std=c++11 -D__STDC_CONSTANT_MACROS

LOCAL_C_INCLUDES += $(LOCAL_PATH)/test \
					$(LOCAL_PATH)/mediaplayer \
				    $(LOCAL_PATH)/mediaplayer/mediasink/audiosink/opensl

LOCAL_MODULE := test
LOCAL_SRC_FILES := test/testrunner.cpp \
			       test/opensltest.cpp

LOCAL_SHARED_LIBRARIES += libbigbean

include $(BUILD_EXECUTABLE)
endif
