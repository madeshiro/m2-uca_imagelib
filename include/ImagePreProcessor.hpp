#ifndef IMAGE_PRE_PROCESSOR_HPP
#define IMAGE_PRE_PROCESSOR_HPP
// OpenCV
#include <opencv2/opencv.hpp>

namespace idl 
{  
    class ImagePreProcessor 
    {
    public:
        cv::Mat process(const cv::Mat& in); // Méthode de pré-traitement des images
    };
}

#endif // IMAGE_PRE_PROCESSOR_HPP
