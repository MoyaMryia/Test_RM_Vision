#include "../include/afterDetect.hpp"

DigitTemplates MatchNumber::loadTemplates(const std::string &template_dir, int mode)
{
    DigitTemplates templates;
    std::string dir = template_dir;
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\')
    {
        dir += "/";
    }
    try
    {
        for (int i = 0; i <= 8; ++i)
        {
            std::string filename = dir + std::to_string(i * mode) + ".jpg";
            if (!fs::exists(filename))
            {
                continue;
            }
            cv::Mat templ = cv::imread(filename, cv::IMREAD_GRAYSCALE);
            if (!templ.empty())
            {
                templates[i] = templ;
            }
            else
            {
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
    }

    if (templates.empty())
    {
    }

    return templates;
}

MatchResult MatchNumber::recognizeSingleDigitByFeature(
    const cv::Mat &frame,
    const DigitTemplates &templates,
    double threshold,
    double min_scale,
    double max_scale,
    double step_scale)
{
    if (frame.empty() || templates.empty())
    {
        return MatchResult();
    }

    cv::Mat frame_gray_te, frame_gray;
    cv::cvtColor(frame, frame_gray_te, cv::COLOR_BGR2GRAY);
    cv::threshold(frame_gray_te, frame_gray, 53, 255, cv::THRESH_BINARY);
    MatchResult best_match;

    // 遍历所有尺度 (Scale)
    for (double scale = max_scale; scale >= min_scale; scale -= step_scale)
    {

        // 确保缩放后的模板不会比目标帧大
        if (templates.begin()->second.cols * scale > frame_gray.cols ||
            templates.begin()->second.rows * scale > frame_gray.rows)
        {
            continue;
        }

        // 遍历所有数字模板
        for (const auto &pair : templates)
        {
            int digit = pair.first;
            const cv::Mat &original_templ = pair.second;

            cv::Mat scaled_templ;
            // 1. 缩放模板（关键步骤）
            // 模板匹配应该在目标图像上滑动，所以我们缩放模板到目标数字的大小
            cv::resize(original_templ, scaled_templ, cv::Size(), scale, scale, cv::INTER_LINEAR);

            // 2. 模板匹配
            cv::Mat result;
            try
            {
                cv::matchTemplate(frame_gray, scaled_templ, result, cv::TM_CCOEFF_NORMED);
            }
            catch (...)
            {
                std::cerr << "Wrong Picture!" << std::endl;
            }

            // 3. 找到当前尺度和模板的最佳匹配
            double minVal, maxVal;
            cv::Point minLoc, maxLoc;
            cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
            /*std::cout << "当前匹配度: 数字 " << digit
                      << " (得分: " << maxVal
                      << ", 缩放: " << scale << ")" << std::endl;*/
            // 4. 更新全局最佳匹配
            if (maxVal > best_match.score)
            {
                best_match.score = maxVal;
                best_match.digit = digit;
                best_match.scale = scale;
                // 注意：匹配矩形的位置和尺寸是基于当前缩放后的模板
                best_match.location = cv::Rect(maxLoc.x, maxLoc.y, scaled_templ.cols, scaled_templ.rows);
            }
        }
    }

    // 5. 最终判断
    if (best_match.score >= threshold)
    {
        std::cout << "匹配度最高: 数字 " << best_match.digit
                  << " (得分: " << best_match.score
                  << ", 缩放: " << best_match.scale << ")" << std::endl;
        return best_match;
    }
    else
    {
        std::cout << "未找到满足阈值 (" << threshold << ") 的有效数字。" << std::endl;
        return MatchResult();
    }
}