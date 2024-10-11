#ifndef PROCESSING_FACTORY_HPP
#define PROCESSING_FACTORY_HPP

#include <opencv2/opencv.hpp>
#include "LineDetector.hpp"
#include "PlantDetector.hpp"
#include "ImagePreProcessor.hpp"
#include "JetPositionChecker.hpp"

class ProcessingFactory {
private:
    cv::Mat img;              // Image à traiter
    cv::Point centerLaser;    // Centre du laser
public:
    void process(const cv::Mat& img); // Méthode principale de traitement
};

#endif // PROCESSING_FACTORY_HPP
