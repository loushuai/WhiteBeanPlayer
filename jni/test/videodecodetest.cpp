#include <catch.hpp>
#include "MediaSource.hpp"
#include "MediaCodec.hpp"

using namespace std;
using namespace whitebean;

void write_yuv_frame(AVFrame *frame, const char *name)
{
	FILE *pf = fopen(name, "wb");
	uint8_t *dataPtr = nullptr;

	dataPtr = frame->data[0];
	for (int i = 0; i < frame->height; ++i) {
		if (fwrite(dataPtr, frame->width, 1, pf) != 1) {
			printf("Write file failed %s\n", strerror(errno));
			goto failed;
		}
		dataPtr += frame->linesize[0];
	}

	dataPtr = frame->data[1];
	for (int i = 0; i < frame->height/2; ++i) {
		if (fwrite(dataPtr, frame->width/2, 1, pf) != 1) {
			printf("Write file failed %s\n", strerror(errno));
			goto failed;
		}
		dataPtr += frame->linesize[1];
	}

	dataPtr = frame->data[2];
	for (int i = 0; i < frame->height/2; ++i) {
		if (fwrite(dataPtr, frame->width/2, 1, pf) != 1) {
			printf("Write file failed %s\n", strerror(errno));
			goto failed;
		}
		dataPtr += frame->linesize[2];
	}

 failed:	
	fclose(pf);
}

TEST_CASE("VideoDecoder")
{
	int ret = 0;
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();

	shared_ptr<MediaSource> source(new MediaSource);

	ret = source->open("/data/local/tmp/video.mp4");
	REQUIRE(ret == 0);
	source->start();

	SECTION("Mediadeocder")
	{
		MediaDecoder mediaDecoder;
		ret = mediaDecoder.open(source , 1);
		REQUIRE(ret == 0);
		mediaDecoder.start();

		int i = 0;
		for (;;) {
			FrameBuffer frmbuf;
			if (mediaDecoder.read(frmbuf)) {
				printf("read frame size %d\n", frmbuf.vsize());
				if (++i == 100)
					write_yuv_frame(frmbuf.getDataPtr(), "/data/local/tmp/test.yuv");
			}

			this_thread::sleep_for(chrono::milliseconds(10));
		}
		
		this_thread::sleep_for(chrono::seconds(100));		
	}
}
