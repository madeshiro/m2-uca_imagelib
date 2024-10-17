#ifndef IMAGE_PRE_PROCESSOR_HPP
#define IMAGE_PRE_PROCESSOR_HPP

#include <opencv2/opencv.hpp>

enum class PreprocessingType {
    GaussianBlur,
    MedianBlur,
    Thresholding,
    EdgeDetection,
    HistogramEqualization,
    NoiseCorrection,
    Grayscale
};

class ImagePreProcessor {
private:

public:
    static cv::Mat applyGaussianBlur(const cv::Mat& img, int kernelSize, double sigma);
    static cv::Mat applyMedianBlur(const cv::Mat& img, int kernelSize);
    static cv::Mat applyThresholding(const cv::Mat& img, double thresh, double maxVal);
    static cv::Mat applyEdgeDetection(const cv::Mat& img, double lowThreshold, double highThreshold);
    static cv::Mat applyHistogramEqualization(const cv::Mat& img);
    static cv::Mat applyNoiseCorrection(const cv::Mat& img, float h = 30.0f);
    static cv::Mat applyGrayscale(const cv::Mat& img);

    static cv::Mat process(const cv::Mat& img, PreprocessingType type);
    static cv::Mat process(const cv::Mat& img);
};

#endif
