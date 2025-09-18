  #include "armor.hpp"

  struct Robot 
  {
      Armor armor;                     // 机器人所包含的Armor类
      float omega;                     // 机器人旋转的角速度
      float vx;                        // 机器人在水平方向的线速度
      float vz;                        // 机器人在垂直方向的线速度
  };