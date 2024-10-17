#ifndef JET_POSITION_CHECKER_HPP
#define JET_POSITION_CHECKER_HPP

#include "Plant.hpp"
#include "LaserBehavior.hpp"
#include <opencv2/opencv.hpp>

namespace idl 
{
    class JetPositionChecker 
    {
    public:
        laserBehavior isOnPlant(const Plant& p, const cv::Point& jet); // Vérifier si le jet est sur la plante
    };
}

#endif // JET_POSITION_CHECKER_HPP
