#ifndef PLANT_HPP
#define PLANT_HPP

#include <opencv2/opencv.hpp>
#include "Species.hpp"

namespace idl 
{
    class Plant {
    public:
        cv::Vec2d center;       // Centre de la plante
        cv::Vec2d position;     // Position de la plante
        cv::Rect boundingBox;   // Boîte englobante de la plante
        cv::Mat plantImg;       // Image de la plante
        cv::Mat mask;           // Masque de la plante
        Species plantSpecies;   // Espèce de la plante (wheat, advantis, etc.)
        float area;
        float score;
    };
}

#endif // PLANT_HPP
