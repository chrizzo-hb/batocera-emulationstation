#include "guis/knulli/RgbService.h"
#include "utils/Platform.h"
#include "Paths.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include <stdio.h>
#include <sys/wait.h>

std::string RGB_SERVICE_NAME = "/usr/bin/analog_stick_led_daemon.sh"
std::string RGB_COMMAND_NAME = "/usr/bin/analog_stick_led.sh"

void RgbService::start()
{
	system(RGB_SERVICE_NAME + " start");
}

void RgbService::stop()
{
	system(RGB_SERVICE_NAME + " stop");
}

void RgbService::setRgb(int mode, int brightness, int speed, int r, int g, int b) {

}
