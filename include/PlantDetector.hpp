#ifndef PLANT_DETECTOR_HPP
#define PLANT_DETECTOR_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include "Plant.hpp"

namespace idl 
{
    class PlantDetector 
    {
    public:
        std::vector<Plant> detectPlants(const cv::Mat& img); // Détection des plantes
    };
}

#endif // PLANT_DETECTOR_HPP
