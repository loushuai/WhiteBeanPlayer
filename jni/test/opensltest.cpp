#include <catch.hpp>
#include <memory>
#include "openslsink.hpp"

using namespace std;
using namespace bigbean;

#define BUFFER_SIZE (1 << 14)

FILE *pf;

size_t fillBuffer(unique_ptr<uint8_t[]> &buf)
{
	int32_t n;

	printf("fill buffer enter\n");

	unique_ptr<uint8_t[]> pcm_buf(new uint8_t[BUFFER_SIZE]);

	n = fread(pcm_buf.get(), 1, BUFFER_SIZE, pf);

	// move unique ptr to caller
	buf = move(pcm_buf);

	printf("fill buffer exit\n");
	
	return n;
}
	
TEST_CASE("OPENSLSINK")
{
	OpenslSink sink;
	
	pf = fopen("./32k16bit1ch.pcm", "rb");

	SECTION("Play")
	{
		sink.open(32000, 1, PCM_FORMAT_FIXED_16, fillBuffer);
		sink.start();
	}

	while (1);

	fclose(pf);
}

