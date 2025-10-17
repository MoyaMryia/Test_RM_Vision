#pragma once
#ifndef PNP_SOLVER_HPP
#define PNP_SOLVER_HPP
#include "include_all.hpp"

class afterDetections
{
public:
    static void detectNumbers(std::vector<Armor> &inputArmors, cv::Mat &inputFrame);
    static void pnpCalculations(std::vector<Armor> &inputArmors, cv::Mat &inputFrame);
    static void mainCalculations(std::vector<Armor> &inputArmors, cv::Mat &inputFrame);
    static void getLightBar(std::vector<Armor> &inputArmors, cv::Mat &inputFrame);
};

#endif