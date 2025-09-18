  struct LightBar 
  {
      cv::Point2f center;              // 灯条的中心点
      cv::Point2f top;                 // 灯条上方的点
      cv::Point2f bottom;              // 灯条下方的点
      cv::Point2f top2bottom;          // 灯条从上到下的方向向量
  };
  
struct Armor 
  {
      cv::Rect Box;                    // 方形位置，使用cv::Rect来表示装甲板的矩形框
      float confidence;                // ResNet识别的置信度
      cv::Scalar color;                // 装甲板的颜色，使用cv::Scalar表示（BGR格式）
      int detect_id;                   // 自动分配的装甲板ID
      int car_num;                     // 根据ResNet识别结果得到的装甲板数字
      LightBar left_LightBar;          // 左方灯条，类型为LightBar
      LightBar right_LightBar;         // 右方灯条，类型为LightBar
      float priority;                     // 评分系统给出的打击评分
      float yaw;                       // pnp解算出的偏航角
      cv::Point3f p_camera;            // pnp解算出的三维位置信息，包含(x, y, z)
  };
