/*
 * AudioSink.h
 *
 * Created on: 2016Äê5ÔÂ15ÈÕ
 *     Author: loushuai
 */

#ifndef JNI_MEDIASINK_AUDIOSINK_AUDIOSINK_H_
#define JNI_MEDIASINK_AUDIOSINK_AUDIOSINK_H_

#include <stdint.h>
#include <memory>

#define DEFAULT_AUDIOSINK_BUFFERCOUNT 4

namespace whitebean
{

typedef enum {
	PCM_FORMAT_FIXED_NONE = -1,
	PCM_FORMAT_FIXED_8,
	PCM_FORMAT_FIXED_16,
	PCM_FORMAT_FIXED_20,
	PCM_FORMAT_FIXED_24,
	PCM_FORMAT_FIXED_28,
	PCM_FORMAT_FIXED_32,
	PCM_FORMAT_FIXED_64,
} pcm_format_t;

class AudioSink {
public:
    // Callback returns the number of bytes actually written to the buffer.
    typedef size_t (*AudioCallback)(std::unique_ptr<uint8_t[]> &buffer);

    virtual             ~AudioSink() {}

    // If no callback is specified, use the "write" API below to submit
    // audio data.
    virtual int open(uint32_t sampleRate, int channelCount,
					 pcm_format_t format=PCM_FORMAT_FIXED_8,
					 AudioCallback cb = NULL) = 0;

	virtual int start() = 0;
};
     
}

#endif /*JNI_MEDIASINK_AUDIOSINK_AUDIOSINK_H_*/
