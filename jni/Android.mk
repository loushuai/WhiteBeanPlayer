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
LOCAL_MODULE    := avfilter
LOCAL_SRC_FILES := libffmpeg/libavfilter.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avutil
LOCAL_SRC_FILES := libffmpeg/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avcodec
LOCAL_SRC_FILES := libffmpeg/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avformat
LOCAL_SRC_FILES := libffmpeg/libavformat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := swscale
LOCAL_SRC_FILES := libffmpeg/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := swresample
LOCAL_SRC_FILES := libffmpeg/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_CPPFLAGS += -std=c++11 -D__STDC_CONSTANT_MACROS 

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include \
					$(LOCAL_PATH)/include/ffmpeg

LOCAL_MODULE := whitebean
LOCAL_SRC_FILES += mediaplayer/WhiteBeanPlayer.cpp \
				   mediaplayer/AudioPlayer.cpp \
				   mediaplayer/TimedEventQueue.cpp \
				   mediaplayer/mediabase/MetaData.cpp \
				   mediaplayer/mediabase/MediaSource.cpp \
				   mediaplayer/mediabase/MediaTracks.cpp \
                   mediaplayer/mediabase/MediaCodec.cpp \
                   mediaplayer/mediasink/audiosink/opensl/openslsink.cpp \
				   jni/whitebean_media_MediaPlayer.cpp

LOCAL_STATIC_LIBRARIES := avformat avcodec avutil swresample avfilter swscale
LOCAL_LDLIBS    += -lOpenSLES -lz -llog

include $(BUILD_SHARED_LIBRARY)

#build test
ifeq ($(TEST), 1)
include $(CLEAR_VARS)
LOCAL_CPPFLAGS += -std=c++11 -D__STDC_CONSTANT_MACROS -fexceptions

LOCAL_C_INCLUDES += $(LOCAL_PATH)/test \
					$(LOCAL_PATH)/mediaplayer \
				    $(LOCAL_PATH)/mediaplayer/mediasink/audiosink/opensl \
					$(LOCAL_PATH)/mediaplayer/mediabase \
					$(LOCAL_PATH)/include \
				    $(LOCAL_PATH)/include/ffmpeg

LOCAL_MODULE := test
LOCAL_SRC_FILES := test/testrunner.cpp 
#LOCAL_SRC_FILES += test/timedeventqueuetest.cpp
#LOCAL_SRC_FILES += test/opensltest.cpp
#LOCAL_SRC_FILES += test/metadatatest.cpp
#LOCAL_SRC_FILES += test/mediasourcetest.cpp
#LOCAL_SRC_FILES += test/audiocodectest.cpp
LOCAL_SRC_FILES += test/audioplayer.cpp

LOCAL_SHARED_LIBRARIES += libwhitebean

include $(BUILD_EXECUTABLE)
endif
