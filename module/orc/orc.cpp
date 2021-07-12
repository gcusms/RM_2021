#include "orc.hpp"

namespace Orc
{
    OrcNumber::OrcNumber(){}

    OrcNumber::OrcNumber(std::string Orc_config_)
    {
        cv::FileStorage Orc_armor(Orc_config_, cv::FileStorage::READ);
        Orc_armor["H_MIN_NUM"] >> num_cfg_.h_min_num;
        Orc_armor["H_MAX_NUM"] >> num_cfg_.h_max_num;
        Orc_armor["S_MIN_NUM"] >> num_cfg_.s_min_num;
        Orc_armor["S_MAX_NUM"] >> num_cfg_.s_max_num;
        Orc_armor["V_MIN_NUM"] >> num_cfg_.v_min_num;
        Orc_armor["V_MAX_NUM"] >> num_cfg_.v_max_num;
        Orc_armor["CONFIDENT"] >> num_cfg_.Confident;
        Orc_armor["KERNEL_SIZE"] >> num_cfg_.kennerl_size;

        Orc_armor["NUMBER_SWITCH"] >> num_cfg_.switch_number;
    }
    OrcNumber::~OrcNumber(){}


} // namespace Orc

