#include "../include/afterDetect.hpp"



DigitTemplates MatchNumber::loadTemplates(const std::string& template_dir,int mode) {
    DigitTemplates templates;
    std::string dir = template_dir;
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') {
        dir += "/";
    }
    try {
        for (int i = 0; i <= 8; ++i) {
            std::string filename = dir + std::to_string(i*mode) + ".jpg";
            if (!fs::exists(filename)) {
                continue;
            }
            cv::Mat templ = cv::imread(filename, cv::IMREAD_GRAYSCALE); 
            if (!templ.empty()) {
                templates[i] = templ;
            } else {

            }
        }
    } catch (const fs::filesystem_error& e) {
    }
    
    if (templates.empty()) {
    }

    return templates;
}

MatchResult MatchNumber::recognizeSingleDigitByFeature(
    const cv::Mat& frame,
    const DigitTemplates& templates,
    int min_inlier_count
) {
    
    if (frame.empty() || templates.empty()) {
        return MatchResult();
    }
    
    auto detector = cv::ORB::create(500); 
    auto matcher = cv::BFMatcher::create(cv::NORM_HAMMING);

    cv::Mat frame_gray_te,frame_gray;
    cv::cvtColor(frame, frame_gray_te, cv::COLOR_BGR2GRAY);
    cv::threshold(frame_gray_te,frame_gray,30,255,cv::THRESH_BINARY);
    std::vector<cv::KeyPoint> kp_scene;
    cv::Mat desc_scene;
    detector->detectAndCompute(frame_gray, cv::noArray(), kp_scene, desc_scene);

    if (desc_scene.empty()) {
        std::cerr << "目标图像未检测到特征点！" << std::endl;
        return MatchResult();
    }

    MatchResult best_match;

    for (const auto& pair : templates) {
        int digit = pair.first;
        const cv::Mat& templ = pair.second;
        std::vector<cv::KeyPoint> kp_templ;
        cv::Mat desc_templ;
        detector->detectAndCompute(templ, cv::noArray(), kp_templ, desc_templ);

        if (desc_templ.empty()) continue;

        std::vector<std::vector<cv::DMatch>> knn_matches;
        matcher->knnMatch(desc_templ, desc_scene, knn_matches, 2);
        const float ratio_thresh = 0.75f;
        std::vector<cv::DMatch> good_matches;
        for (size_t i = 0; i < knn_matches.size(); i++) {
            if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance) {
                good_matches.push_back(knn_matches[i][0]);
            }
        }
        if (good_matches.size() >= 4) {
            std::vector<cv::Point2f> pts_templ, pts_scene;
            for (const auto& match : good_matches) {
                pts_templ.push_back(kp_templ[match.queryIdx].pt);
                pts_scene.push_back(kp_scene[match.trainIdx].pt);
            }

            cv::Mat mask;
            cv::Mat H = cv::findHomography(pts_templ, pts_scene, cv::RANSAC, 5.0, mask);
            int inlier_count = cv::countNonZero(mask);

            if (inlier_count >= min_inlier_count && inlier_count > best_match.score) {
                std::vector<cv::Point2f> templ_corners(4);
                templ_corners[0] = cv::Point2f(0, 0);
                templ_corners[1] = cv::Point2f((float)templ.cols, 0);
                templ_corners[2] = cv::Point2f((float)templ.cols, (float)templ.rows);
                templ_corners[3] = cv::Point2f(0, (float)templ.rows);
                std::vector<cv::Point2f> scene_corners(4);
                cv::perspectiveTransform(templ_corners, scene_corners, H);
                float min_x = scene_corners[0].x, max_x = scene_corners[0].x;
                float min_y = scene_corners[0].y, max_y = scene_corners[0].y;
                for (int i = 1; i < 4; i++) {
                    min_x = std::min(min_x, scene_corners[i].x);
                    max_x = std::max(max_x, scene_corners[i].x);
                    min_y = std::min(min_y, scene_corners[i].y);
                    max_y = std::max(max_y, scene_corners[i].y);
                }

                best_match.score = (double)inlier_count;
                best_match.digit = digit;
                best_match.location = cv::Rect((int)min_x, (int)min_y, (int)(max_x - min_x), (int)(max_y - min_y));
            }
        }
    }

    if (best_match.digit != -1) {
        std::cout << "特征匹配成功: 数字 " << best_match.digit 
                  << " (内点数: " << best_match.score << ")" << std::endl;
    } else {
        std::cout << "未找到具有足够内点的有效匹配 (要求: " << min_inlier_count << ")。" << std::endl;
    }
    
    return best_match;
}