#include "guis/knulli/UsbService.h"
#include "utils/Platform.h"
#include "Paths.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include <stdio.h>
#include <sys/wait.h>
#include <fstream>

const std::string USB_SERVICE_NAME = "/etc/init.d/S07adb";
const std::string SEPARATOR = " ";
const std::string START = "start";
const std::string STOP = "stop";
const std::string RESTART = "restart";

void UsbService::start()
{
	call(START);
}

void UsbService::stop()
{
	call(STOP);
}

void UsbService::restart()
{
	call(RESTART);
}

void UsbService::call(std::string argument)
{
	if (Utils::FileSystem::exists(USB_SERVICE_NAME)) {
		system((USB_SERVICE_NAME + SEPARATOR + argument).c_str());
	}
}