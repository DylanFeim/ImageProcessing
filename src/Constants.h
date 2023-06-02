#pragma once

#include "ofMain.h"

namespace Constants {
	static const int APP_WINDOW_WIDTH		= 1920;
	static const int APP_WINDOW_HEIGHT		= 1080;
	static const int APP_DESIRED_FRAMERATE	= 60;

	static const std::string VIDEO_PATH_GRAYSCALE	= "fingers.mov";
	static const std::string VIDEO_PATH_COLOR_ONE	= "tupperware.mov";
	static const int VIDEO_WIDTH	= 320;	//both videos are very low resolution (change this if you use a diff video)
	static const int VIDEO_HEIGHT	= 240;

	static const int VIDEO_BORDER_SIZE	= 10;	//keep track of space added between videos

	//adding in the ability to use live camera if wanted
	static const bool USE_LIVE_VIDEO	= true;
	static const int CAM_WIDTH			= 320;
	static const int CAM_HEIGHT			= 240;

	static const int MAX_OBJECTS		= 5;
	static const int MAX_CHAR			= 32;

	# define M_PI           3.14159265358979323846

	//to keep track of what state the app is in
	enum APP_MODE {
		FIND_DIFFERENCE,
		FIND_COLOR
	};

	//switch (appMode) {
	//	case GAME_START:
	//		//run start menu logic
	//	break
	//	case GAME_RUNNING:
	//		//run game running logic
	//	break;
	//	case GAME_OVER:
	//		//run game over logic
	//	break;
	//}
};
