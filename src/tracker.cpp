#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include "../include/include_all.hpp"
static int nextArmorTrackId = 0;

struct ArmorTrack {
    int id;               
    int color;            
    int lostCount;        
    cv::KalmanFilter kf_pos;    
    //姿态存储
    cv::Vec3d current_rVec;//当前最优估计
    cv::Vec3d previous_rVec;//上一帧
    static constexpr int STATE_SIZE = 6;  // [x, y, z, vx, vy, vz]^T
    static constexpr int MEASUREMENT_SIZE = 3; // [x, y, z]^T
    ArmorTrack(int newId, const Armor& armor) 
        : id(newId), color(armor.colorInt), lostCount(0), 
          current_rVec(armor.rVec), previous_rVec(armor.rVec) // 初始姿态
    {
        initKalmanFilter(armor.tVec);
    }
    void initKalmanFilter(const cv::Vec3d& initialTvec);
    cv::Vec3d predict(); 
    void update(const cv::Vec3d& currentTvec, const cv::Vec3d& currentRvec); 
};

void ArmorTrack::initKalmanFilter(const cv::Vec3d& initialTvec) {
    kf_pos.init(STATE_SIZE, MEASUREMENT_SIZE, 0);
    // 状态转移矩阵 A (匀速运动模型)
    // A = [ I(3x3)  dt*I(3x3) ]
    //     [ 0(3x3)  I(3x3)    ]
    float dt = 1.000000/30.000000; // 假设帧间时间差为 1，实际应精确测量
    kf_pos.transitionMatrix = (cv::Mat_<float>(STATE_SIZE, STATE_SIZE) <<
        1, 0, 0, dt, 0, 0,
        0, 1, 0, 0, dt, 0,
        0, 0, 1, 0, 0, dt,
        0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 0, 1
    );
    // 测量矩阵 H: 只测量位置 [x, y, z]
    kf_pos.measurementMatrix = (cv::Mat_<float>(MEASUREMENT_SIZE, STATE_SIZE) <<
        1, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0
    );
    // 初始状态 X_0 = [x, y, z, 0, 0, 0]^T
    kf_pos.statePost.at<float>(0) = initialTvec[0];
    kf_pos.statePost.at<float>(1) = initialTvec[1];
    kf_pos.statePost.at<float>(2) = initialTvec[2];
    // 初始速度设为 0
    cv::setIdentity(kf_pos.processNoiseCov, cv::Scalar::all(1e-2));
    cv::setIdentity(kf_pos.measurementNoiseCov, cv::Scalar::all(1e-1));
    cv::setIdentity(kf_pos.errorCovPost, cv::Scalar::all(1));
}

cv::Vec3d ArmorTrack::predict() {
    cv::Mat prediction = kf_pos.predict();
    return cv::Vec3d(prediction.at<float>(0), prediction.at<float>(1), prediction.at<float>(2));
}

void ArmorTrack::update(const cv::Vec3d& currentTvec, const cv::Vec3d& currentRvec) {
    cv::Mat measurement = (cv::Mat_<float>(MEASUREMENT_SIZE, 1) <<
        currentTvec[0],
        currentTvec[1],
        currentTvec[2]
    );
    kf_pos.correct(measurement);
    this->previous_rVec = this->current_rVec;
    this->current_rVec = currentRvec; // 直接使用 PnP 测量值
}

/**
 * @brief 使用 3D 预测（卡尔曼）和 3D 距离进行多目标跟踪
 * @param currentArmors 当前帧检测到的所有 Armor 结构体 (包含 tVec 和 rVec)
 * @param activeTracks 引用，存储所有当前活动的 ArmorTrack
 * @param maxDistanceThreshold 3D 欧式距离的最大匹配阈值 (例如 0.5 米)
 * @param maxLostFrames 轨迹被删除前的最大丢失帧数
 */
void trackArmorsAdvanced(
    const std::vector<Armor>& currentArmors,
    std::vector<ArmorTrack>& activeTracks,
    double maxDistanceThreshold = 0.5,
    int maxLostFrames = 5) 
{
    // --- 步骤 1: 轨迹预测 ---
    std::vector<cv::Vec3d> predictedTvecs; 
    for (ArmorTrack& track : activeTracks) {
        predictedTvecs.push_back(track.predict()); // 执行卡尔曼预测
    }

    // --- 步骤 2: 3D 距离数据关联（Cost Matrix）---
    cv::Mat costMatrix = cv::Mat::zeros(currentArmors.size(), activeTracks.size(), CV_64F);

    for (int i = 0; i < currentArmors.size(); ++i) {
        for (int j = 0; j < activeTracks.size(); ++j) {
            
            // 匹配条件 1: 颜色必须一致
            if (currentArmors[i].colorInt != activeTracks[j].color) {
                costMatrix.at<double>(i, j) = std::numeric_limits<double>::max(); 
                continue;
            }

            // 匹配条件 2: 计算 3D 欧式距离 (位置代价)
            cv::Vec3d delta = currentArmors[i].tVec - predictedTvecs[j];
            double distance = cv::sqrt(delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2]);
            
            if (distance > maxDistanceThreshold) {
                costMatrix.at<double>(i, j) = std::numeric_limits<double>::max();
            } else {
                costMatrix.at<double>(i, j) = distance; // 3D 距离作为主要代价
            }
        }
    }
    std::vector<int> detectionMatches(currentArmors.size(), -1);
    std::vector<bool> trackMatched(activeTracks.size(), false);
    
    // 循环寻找代价最小的未匹配对
    for (int k = 0; k < std::min(currentArmors.size(), activeTracks.size()); ++k) {
        double minCost = std::numeric_limits<double>::max();
        int bestDetIndex = -1;
        int bestTrackIndex = -1;

        for (int i = 0; i < currentArmors.size(); ++i) {
            if (detectionMatches[i] != -1) continue; 
            for (int j = 0; j < activeTracks.size(); ++j) {
                if (trackMatched[j]) continue; 
                
                if (costMatrix.at<double>(i, j) < minCost) {
                    minCost = costMatrix.at<double>(i, j);
                    bestDetIndex = i;
                    bestTrackIndex = j;
                }
            }
        }
        
        if (bestDetIndex != -1 && minCost < std::numeric_limits<double>::max()) {
            detectionMatches[bestDetIndex] = bestTrackIndex;
            trackMatched[bestTrackIndex] = true;
        } else {
            break; 
        }
    }
    //处理已匹配
    for (size_t i = 0; i < currentArmors.size(); ++i) {
        int trackIndex = detectionMatches[i];
        if (trackIndex != -1) {
            //更新卡尔曼滤波器和姿态
            activeTracks[trackIndex].update(currentArmors[i].tVec, currentArmors[i].rVec); 
            activeTracks[trackIndex].lostCount = 0;
            //更新2D
            activeTracks[trackIndex].kf_pos.statePost.copyTo(activeTracks[trackIndex].kf_pos.statePre);
        }
    }

    activeTracks.erase(
        std::remove_if(activeTracks.begin(), activeTracks.end(), 
            [&](ArmorTrack& track) {
                size_t trackIndex = std::distance(activeTracks.begin(), static_cast<std::vector<ArmorTrack>::iterator>(&track) );
                
                if (!trackMatched[trackIndex]) {
                    track.lostCount++;
                    // 这里应该用kf_pos.predict()的结果来更新轨迹的 3D/2D 预测位置
                    if (track.lostCount > maxLostFrames) {
                        std::cout << "Track ID " << track.id << " lost and removed." << std::endl;
                        return true; // 删除
                    }
                }
                return false; // 保留
            }),
        activeTracks.end());


    for (size_t i = 0; i < currentArmors.size(); ++i) {
        if (detectionMatches[i] == -1) {
            int newId = nextArmorTrackId++;
            std::cout << "New Track ID " << newId << " created." << std::endl;
            activeTracks.emplace_back(newId, currentArmors[i]);
        }
    }
}