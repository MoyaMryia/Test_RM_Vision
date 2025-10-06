#include "../include/tools.hpp"
#include "../include/robotCalculations.hpp"

Robot robotCalculations::getRobotArmors(const std::vector<Armor> &inputArmor, const Robot &inputRobot)
{
    cv::Rect botrect = inputRobot.robotrect;
    Robot outputBot;
    for (const auto &armor : inputArmor)
    {
        if ((armor.Box & botrect) == armor.Box)
        {
            outputBot.armor.push_back(armor);
        }
    }
    outputBot.robotrect = botrect;
    outputBot.color = inputRobot.color;
    outputBot.carorwatcher = inputRobot.carorwatcher;
    return outputBot;
}