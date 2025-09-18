  struct LightBar 
  {
      cv::Point2f center;              // 灯条的中心点
      cv::Point2f top;                 // 灯条上方的点
      cv::Point2f bottom;              // 灯条下方的点
      cv::Point2f top2bottom;          // 灯条从上到下的方向向量
  };