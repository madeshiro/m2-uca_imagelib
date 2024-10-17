#ifndef PROCESSING_FACTORY_HPP
#define PROCESSING_FACTORY_HPP

#include <opencv2/opencv.hpp>
#include "LineDetector.hpp"
#include "PlantDetector.hpp"
#include "ImagePreProcessor.hpp"
#include "JetPositionChecker.hpp"

namespace idl
{
    class ProcessingFactory {
    private:
        cv::Mat img;             
        cv::Point centerLaser;  
    public:
        void process(const cv::Mat& img);
    };
}

#endif
