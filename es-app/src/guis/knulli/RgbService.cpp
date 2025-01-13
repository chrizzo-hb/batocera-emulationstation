#include "guis/knulli/RgbService.h"
#include "utils/Platform.h"
#include "Paths.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include <stdio.h>
#include <sys/wait.h>

const std::string RGB_SERVICE_NAME = "/usr/bin/analog_stick_led_daemon.sh";
const std::string RGB_COMMAND_NAME = "/usr/bin/analog_stick_led.sh";

void RgbService::start()
{
	system((RGB_SERVICE_NAME + " start").c_str());
}

void RgbService::stop()
{
	system((RGB_SERVICE_NAME + " stop").c_str());
}

void RgbService::setRgb(int mode, int brightness, int speed, int r, int g, int b) {

}
