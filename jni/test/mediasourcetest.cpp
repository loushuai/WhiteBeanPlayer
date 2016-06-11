#include <catch.hpp>
#include <MediaSource.hpp>

using namespace std;
using namespace whitebean;

TEST_CASE("MediaSource")
{
	av_register_all();
	MediaSource source;

	SECTION("open")
	{
		source.open("/data/local/tmp/video.mp4");
		source.start();
		this_thread::sleep_for(chrono::seconds(100));
		source.stop();
	}
}
