
#ifndef _USE_ORCNUMBER_
#define _USE_ORCNUMBER_


#include <iostream>
#include <opencv2/opencv.hpp>
#include"opencv_onnx_inferring.hpp"


namespace Orc
{
struct Number_Cfg {

  int h_min_num;
  int h_max_num;

  int s_min_num ;
  int s_max_num;

  int v_min_num;
  int v_max_num;

  int Confident;
  int kennerl_size ;
  
  int switch_number;


};

    class OrcNumber
    {
    private:
        /* data */
    public:
        OrcNumber();
        OrcNumber(std::string Orc_config_);

        ~OrcNumber();
        Number_Cfg num_cfg_;

        bool NumberOrc(model model);
    };
} // namespace Orc







#endif