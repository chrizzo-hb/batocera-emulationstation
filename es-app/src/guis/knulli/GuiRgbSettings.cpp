#include "guis/knulli/GuiRgbSettings.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"
#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "SystemConf.h"
#include "ApiSystem.h"
#include "Scripting.h"
#include "InputManager.h"
#include "AudioManager.h"
#include <SDL_events.h>
#include <algorithm>
#include <memory>
#include <string>
#include "RgbService.h"

#include "Log.h"


constexpr const char* MENU_EVENT_NAME = "rgb-changed";

constexpr char RGB_DELIMITER = ' ';
constexpr const char* DEFAULT_LED_MODE = "1";
constexpr float DEFAULT_COLOR_RED = 148;
constexpr float DEFAULT_COLOR_GREEN = 255;
constexpr float DEFAULT_COLOR_BLUE = 0;
constexpr float DEFAULT_BRIGHTNESS = 100;
constexpr float DEFAULT_SPEED = 15;
constexpr float DEFAULT_LOW_BATTERY_THRESHOLD = 20;
constexpr const char* DEFAULT_SWITCH_ON = "1";

// Constructor creates a new GuiRgbSettings menu.
GuiRgbSettings::GuiRgbSettings(Window* window) : GuiSettings(window, _("RGB LED SETTINGS").c_str())
{

    LOG(LogError) << "GuiRgbSettings constructor";

    // Temporary disable RgbService to be able to interact with the RGB LEDs directly
    RgbService::stop();

    LOG(LogError) << "GuiRgbSettings RGB service stopped";

    addGroup(_("REGULAR LED MODE AND COLOR"));

    LOG(LogError) << "GuiRgbSettings Group Regular added";

    // LED Mode Options
    optionListMode = createModeOptionList();

    LOG(LogError) << "optionListMode created";

    // LED Brightness Slider
    sliderLedBrightness = createSlider("BRIGHTNESS", 0.f, 255.f, 1.f, "", "");
    
    LOG(LogError) << "sliderLedBrightness created";
    
    setConfigValueForSlider(sliderLedBrightness, DEFAULT_BRIGHTNESS, "led.brightness");

    LOG(LogError) << "sliderLedBrightness initialized";

    // Adaptive Brightness switch
    switchAdaptiveBrightness = createSwitch("ADAPTIVE BRIGHTNESS", "led.brightness.adaptive", "Automatically adapts LED brightness to screen brightness (based on the brightness setting above).");

    LOG(LogError) << "switchAdaptiveBrightness created";

    // LED Speed Slider
    sliderLedSpeed = createSlider("SPEED", 1.f, 255.f, 1.f, "", "Not applicable for all devices/modes. Warning: High speed may cause seizures for people with photosensitive epilepsy.");
    setConfigValueForSlider(sliderLedSpeed, DEFAULT_SPEED, "led.speed");

    LOG(LogError) << "sliderLedSpeed created";

    // LED Colour Sliders
    std::array<float, 3> rgbValues = getRgbValues();
    sliderLedRed = createSlider("RED", 0.f, 255.f, 1.f, "", "");
    sliderLedRed->setValue(rgbValues[0]);
    sliderLedGreen = createSlider("GREEN", 0.f, 255.f, 1.f, "", "");
    sliderLedGreen->setValue(rgbValues[1]);
    sliderLedBlue = createSlider("BLUE", 0.f, 255.f, 1.f, "", "");
    sliderLedBlue->setValue(rgbValues[2]);

    LOG(LogError) << "sliderLed R/G/B created";

    addGroup(_("BATTERY CHARGE INDICATION"));

    // Low battery threshold slider
    sliderLowBatteryThreshold = createSlider("LOW BATTERY THRESHOLD", 1.f, 100.f, 5.f, "%", "Show yellow/red breathing when battery is below this threshold. Set to 0 to disable.");
    setConfigValueForSlider(sliderLowBatteryThreshold, DEFAULT_LOW_BATTERY_THRESHOLD, "led.battery.low");
    switchBatteryCharging = createSwitch("BATTERY CHARGING", "led.battery.charging", "Show green breathing while device is charging.");

    addGroup(_("RETRO ACHIEVEMENT INDICATION"));
    switchRetroAchievements = createSwitch("ACHIEVEMENT EFFECT", "led.retroachievements", "Honor your retro achievements with a LED effect.");

    LOG(LogError) << "now adding save func";

    addSaveFunc([this] {
        // Read all variables from the respective UI elements and set the respective values in batocera.conf
        SystemConf::getInstance()->set("led.mode", optionListMode->getSelected());
        SystemConf::getInstance()->set("led.brightness", std::to_string((int) sliderLedBrightness->getValue()));
        //SystemConf::getInstance()->set("led.brightness.adaptive", (switchAdaptiveBrightness->getState() ? DEFAULT_SWITCH_ON : "0"));
        SystemConf::getInstance()->set("led.speed", std::to_string((int) sliderLedSpeed->getValue()));
        setRgbValues(sliderLedRed->getValue(), sliderLedGreen->getValue(), sliderLedBlue->getValue());
        SystemConf::getInstance()->set("led.battery.low", std::to_string((int) sliderLowBatteryThreshold->getValue()));
        SystemConf::getInstance()->set("led.battery.charging", (switchBatteryCharging->getState() ? DEFAULT_SWITCH_ON : "0"));
        SystemConf::getInstance()->set("led.retroachievements", (switchRetroAchievements->getState() ? DEFAULT_SWITCH_ON : "0"));
		SystemConf::getInstance()->saveSystemConf();
		Scripting::fireEvent(MENU_EVENT_NAME);

        // Reactivate the RGB Service
        RgbService::start();
    });

    LOG(LogError) << "save func added";
}

// Creates a new mode option list
std::shared_ptr<OptionListComponent<std::string>> GuiRgbSettings::createModeOptionList()
{
    auto optionsLedMode = std::make_shared<OptionListComponent<std::string>>(mWindow, _("MODE"), false);

    std::string selectedLedMode = SystemConf::getInstance()->get("led.mode");
    if (selectedLedMode.empty())
        selectedLedMode = DEFAULT_LED_MODE;

    // TODO: Retrieve board-specific mode list somehow
    optionsLedMode->add(_("NONE"), "0", selectedLedMode == "0");
    optionsLedMode->add(_("STATIC"), "1", selectedLedMode == "1");
    optionsLedMode->add(_("BREATHING (FAST)"), "2", selectedLedMode == "2");
    optionsLedMode->add(_("BREATHING (MEDIUM)"), "3", selectedLedMode == "3");
    optionsLedMode->add(_("BREATHING (SLOW)"), "4", selectedLedMode == "4");
    optionsLedMode->add(_("SINGLE RAINBOW"), "5", selectedLedMode == "5");
    optionsLedMode->add(_("MULTI RAINBOW"), "6", selectedLedMode == "6");

    optionsLedMode->setSelectedChangedCallback([this](std::string value) { applyValues(); });

    addWithDescription(_("MODE"), _("Not every mode is available on every device."), optionsLedMode);
    return optionsLedMode;
}

// Creates a new slider
std::shared_ptr<SliderComponent> GuiRgbSettings::createSlider(std::string label, float min, float max, float step, std::string unit, std::string description)
{
    std::shared_ptr<SliderComponent> slider = std::make_shared<SliderComponent>(mWindow, min, max, step, unit);
    slider->setOnValueChanged([this](float value)
    {
        LOG(LogError) << "applying values";
        applyValues();
        LOG(LogError) << "applied values";
    });
    if (description.empty()) {
        addWithLabel(label, slider);
    } else {
        addWithDescription(label, description, slider);
    }
    return slider;
}

// Sets an initial value to a slider, either from default value or from variable if a batocera.conf variable for this slider has been set
void GuiRgbSettings::setConfigValueForSlider(std::shared_ptr<SliderComponent> slider, float defaultValue, std::string variable)
{

    LOG(LogError) << "setConfigValueForSlider called";
    float selectedValue = defaultValue;
    LOG(LogError) << "selectedValue initialized";
    std::string configuredValue = SystemConf::getInstance()->get(variable);
    LOG(LogError) << "configuredValue initialized";
    if (!configuredValue.empty()) {
        LOG(LogError) << "updating selectedValue";
        selectedValue = Utils::String::toFloat(configuredValue);
        LOG(LogError) << "updated selectedValue";
    }
    LOG(LogError) << "setting initial value";
    slider->setValue(selectedValue);
    LOG(LogError) << "set initial value";
}

// Creates a new switch
std::shared_ptr<SwitchComponent> GuiRgbSettings::createSwitch(std::string label, std::string variable, std::string description)
{
    std::shared_ptr<SwitchComponent> switchComponent = std::make_shared<SwitchComponent>(mWindow);
    std::string selected = SystemConf::getInstance()->get(variable);
    if (selected.empty())
        selected = DEFAULT_SWITCH_ON;

    switchComponent->setState(selected == DEFAULT_SWITCH_ON);
    addWithDescription(label, description, switchComponent);
    return switchComponent;
}

// Retrieves RGB value settings from batocera.conf as an array of floats
std::array<float, 3> GuiRgbSettings::getRgbValues()
{
    std::string colour = SystemConf::getInstance()->get("led.colour");
    if (colour.empty()) {
        return {0, 0, 0};
    }

    std::vector<std::string> rgbValues;
    std::stringstream stringStream(colour);
    std::string item;

    while (getline(stringStream, item, RGB_DELIMITER)) {
        rgbValues.push_back(item);
    }

    int red = std::stoi(rgbValues[0]);
    int green = std::stoi(rgbValues[1]);
    int blue = std::stoi(rgbValues[2]);

    return {red, green, blue};
}

// Concatenates the RGB values and stores them in batocera.conf.
void GuiRgbSettings::setRgbValues(float red, float green, float blue)
{
    std::string colour = std::to_string((int) red) + RGB_DELIMITER + std::to_string((int) green) + RGB_DELIMITER + std::to_string((int) blue);
    SystemConf::getInstance()->set("led.colour", colour);
}

void GuiRgbSettings::applyValues()
{
    LOG(LogError) << "apply values called";
    std::string selectedMode = optionListMode->getSelected();
    LOG(LogError) << "var selectedMode read";
    int selectedBrightness = (int) sliderLedBrightness->getValue();
    LOG(LogError) << "var selectedBrightness read";
    int selectedSpeed = (int) sliderLedSpeed->getValue();
    LOG(LogError) << "var selectedSpeed read";
    int selectedRed = (int) sliderLedRed->getValue();
    LOG(LogError) << "var selectedRed read";
    int selectedGreen = (int) sliderLedGreen->getValue();
    LOG(LogError) << "var selectedGreen read";
    int selectedBlue = (int) sliderLedBlue->getValue();
    LOG(LogError) << "var selectedBlue read";
    LOG(LogError) << "all vars read for applying";
    RgbService::setRgb(std::stoi(selectedMode), selectedBrightness, selectedSpeed, selectedRed, selectedGreen, selectedBlue);
    LOG(LogError) << "all vars applied";
}
