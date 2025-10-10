#pragma once
#ifndef PNP_SOLVER_HPP
#define PNP_SOLVER_HPP
#include "tools.hpp"

class afterDetections
{
public:
    static std::vector<Armor> detectNumbers(const std::vector<Armor> &inputArmors, const cv::Mat inputFrame);
    static std::vector<Armor> pnpCalculations(const std::vector<Armor> &inputArmors, const cv::Mat inputFrame);
    static std::vector<Armor> mainCalculations(const std::vector<Armor> &inputArmors, const cv::Mat inputFrame);
};

#endif