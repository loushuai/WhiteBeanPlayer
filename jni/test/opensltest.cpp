#include <catch.hpp>
#include "openslsink.hpp"

using namespace bigbean;

#define BUFFER_SIZE (1 << 14)

FILE *pf;

size_t fillBuffer(void **buf, size_t *size)
{
	int32_t n;

	printf("fill buffer enter\n");
	
	if (*size < BUFFER_SIZE || !(*buf)) {
		*buf = realloc(*buf, BUFFER_SIZE);
		*size = BUFFER_SIZE;
	}

	n = fread(*buf, 1, BUFFER_SIZE, pf);

	printf("fill buffer exit\n");
	
	return n;
}
	
TEST_CASE("OPENSLSINK")
{
	OpenslSink sink;
	
	pf = fopen("./32k16bit1ch.pcm", "rb");

	SECTION("Play")
	{
		sink.open(32000, 1, PCM_FORMAT_FIXED_8, fillBuffer);
		sink.start();
	}

	while (1);

	fclose(pf);
}

