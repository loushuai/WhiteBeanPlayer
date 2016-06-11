#include <catch.hpp>
#include "AudioPlayer.hpp"

using namespace std;
using namespace whitebean;

MediaDecoder decoder;

size_t sinkCallBack(unique_ptr<uint8_t[]> &buf, void *cookie) {
	size_t size = 0;
	FrameBuffer frmbuf;

 retry:
	if (decoder.read(frmbuf)) {		
		size = frmbuf.size();
		unique_ptr<uint8_t[]> pcmbuf(new uint8_t[size]);
		memcpy(pcmbuf.get(), frmbuf.getData().data[0], size);
		buf = std::move(pcmbuf);
	} else {
		this_thread::sleep_for(chrono::milliseconds(10));
		goto retry;
	}

	return size;	
}

TEST_CASE("AudioPlayer")
{
	int ret = 0;
	
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();

	shared_ptr<MediaSource> source(new MediaSource);

	ret = source->open("/data/local/tmp/video.mp4");
	REQUIRE(ret == 0);
	source->start();

#if 1
	SECTION("Play")
	{
		AudioPlayer audioPlayer;
		audioPlayer.setSource(source);
		audioPlayer.start();

		while(1);
	}
#endif
#if 0
	SECTION("Components")
	{
		int audioid = source->getAudioStreamId();
		printf("audioid %d\n", audioid);		

		ret = decoder.open(source, audioid);
		REQUIRE(ret == 0);
		
		decoder.start();

		std::unique_ptr<AudioSink> sink(new OpenslSink);
		ret = sink->open(44100, 2, PCM_FORMAT_FIXED_16, sinkCallBack, nullptr);
		REQUIRE(ret == 0);

		sink->start();

		while(1);
	}
#endif
}
