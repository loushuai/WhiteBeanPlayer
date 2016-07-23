#include "log.hpp"
#include "WhiteBeanPlayer.hpp"

using namespace whitebean;

int main()
{
	WhiteBeanPlayer player;
	int ret = 0;

	ret = player.setDataSource("/data/local/tmp/video.mp4");
	if (ret != 0) {
		LOGD("Set datasource failed");
		return -1;
	}

	ret = player.prepare();
	if (ret != 0) {
		LOGD("Set prepare failed");
		return -1;		
	}

	ret = player.play();
	if (ret != 0) {
		LOGD("Play failed");
		return -1;
	}

	while(1);

	return 0;
}
