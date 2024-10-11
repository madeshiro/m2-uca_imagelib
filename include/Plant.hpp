#ifndef PLANT_HPP
#define PLANT_HPP

#include <opencv2/opencv.hpp>
#include "species.hpp"

class Plant {
public:
    cv::Vec2d center;       // Centre de la plante
    cv::Vec2d position;     // Position de la plante
    cv::Rect boundingBox;   // Boîte englobante de la plante
    cv::Mat plantImg;       // Image de la plante
    cv::Mat mask;           // Masque de la plante
    species plantSpecies;   // Espèce de la plante (wheat, advantis, etc.)
};

#endif // PLANT_HPP
