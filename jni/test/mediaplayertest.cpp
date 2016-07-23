#include <catch.hpp>
#include "WhiteBeanPlayer.hpp"

using namespace std;
using namespace whitebean;

TEST_CASE("WhiteBeanPlayer")
{
	WhiteBeanPlayer player;
	int ret = 0;

	SECTION("Prepare")
	{
		ret = player.setDataSource("/data/local/tmp/test.mp4");
		REQUIRE(ret == 0);

		ret = player.prepareAsync();
		REQUIRE(ret == 0);

		while(1);
	}
}

