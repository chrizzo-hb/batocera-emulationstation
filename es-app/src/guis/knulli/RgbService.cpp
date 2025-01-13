#include "guis/knulli/RgbService.h"
#include "utils/Platform.h"
#include "Paths.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include <stdio.h>
#include <sys/wait.h>
#include <fstream>

const std::string RGB_SERVICE_NAME = "/usr/bin/analog_stick_led_daemon.sh";
const std::string RGB_COMMAND_NAME = "/usr/bin/analog_stick_led.sh";
const std::string SEPARATOR = "";
const std::string START = "start";
const std::string STOP = "stop";

void RgbService::start()
{
	if (Utils::FileSystem::exists(RGB_SERVICE_NAME)) {
		system((RGB_SERVICE_NAME + SEPARATOR + START).c_str());
	}
}

void RgbService::stop()
{
	if (Utils::FileSystem::exists(RGB_SERVICE_NAME)) {
		system((RGB_SERVICE_NAME + SEPARATOR + STOP).c_str());
	}
}

// TODO: This is a prototype. First improve the RGB bash scripts, then adopt the changes here.
void RgbService::setRgb(int mode, int brightness, int speed, int r, int g, int b) {
	
	modeString = std::to_string(mode);
	brightnessString = std::to_string(brightness);
	speedString = std::to_string(speed);
	rString = std::to_string(r);
	gString = std::to_string(g);
	bString = std::to_string(b);

	if (mode == 0) {
		system((RGB_COMMAND_NAME + SEPARATOR + modeString).c_str());
	}
	else if (mode < 5) {
		system((RGB_COMMAND_NAME
			 + SEPARATOR + modeString
			 + SEPARATOR + brightnessString
			 + SEPARATOR + rString
			 + SEPARATOR + gString
			 + SEPARATOR + bString
			).c_str());
	} else {
		system((RGB_COMMAND_NAME
			+ SEPARATOR + modeString
			+ SEPARATOR + brightnessString
			+ SEPARATOR + speedString
			).c_str());
	}

}
