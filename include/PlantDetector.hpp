#ifndef PLANT_DETECTOR_HPP
#define PLANT_DETECTOR_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include "Plant.hpp"

class PlantDetector {
protected:
    cv::Mat ElimColor(cv::Mat in, cv::Scalar min, cv::Scalar max, int morph_size, int inpaint_size);
public:
    std::vector<Plant> detectPlants(const cv::Mat& img); // Détection des plantes
};

#endif // PLANT_DETECTOR_HPP
