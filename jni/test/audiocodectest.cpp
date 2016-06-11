#include <catch.hpp>
#include "MediaSource.hpp"
#include "MediaCodec.hpp"

using namespace std;
using namespace whitebean;

TEST_CASE("AudioDecoder")
{
	int ret = 0;
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();

	shared_ptr<MediaSource> source(new MediaSource);

	ret = source->open("/data/local/tmp/video.mp4");
	REQUIRE(ret == 0);
	source->start();
#if 0	
	SECTION("Audiodecoder")
	{
		AudioDecoder  audioDecoder;
		ret = audioDecoder.open(source);
		REQUIRE(ret == 0);
		audioDecoder.start();

		FILE *pf = fopen("/data/local/tmp/test.pcm", "wb");
		for (;;) {
			FrameBuffer frmbuf;
			if (audioDecoder.read(frmbuf)) {
				printf("read frame size %d\n", frmbuf.size());
				fwrite(frmbuf.getData().data[0], frmbuf.size(), 1, pf);				
			}

			this_thread::sleep_for(chrono::milliseconds(10));
		}
		
		this_thread::sleep_for(chrono::seconds(100));

		audioDecoder.stop();
		source.stop();
	}
#endif	

	SECTION("Mediadecoder")
	{
		MediaDecoder mediaDecoder;
		ret = mediaDecoder.open(source, 0);
		REQUIRE(ret == 0);
		mediaDecoder.start();

		FILE *pf = fopen("/data/local/tmp/test.pcm", "wb");
		for (;;) {
			FrameBuffer frmbuf;
			if (mediaDecoder.read(frmbuf)) {
				printf("read frame size %d\n", frmbuf.size());
				fwrite(frmbuf.getData().data[0], frmbuf.size(), 1, pf);				
			}

			this_thread::sleep_for(chrono::milliseconds(10));
		}
		
		this_thread::sleep_for(chrono::seconds(100));		
	}
}
