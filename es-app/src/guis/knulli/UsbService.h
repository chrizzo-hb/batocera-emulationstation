#pragma once

class UsbService
{
public:
        static void start();
        static void stop();
        static void restart();

private:
        static void call(std::string argument);
};
