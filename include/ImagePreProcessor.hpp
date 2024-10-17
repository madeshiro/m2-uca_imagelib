//------------------------------------------------------------------------------
//
// File:        ImagePreProcessor.hpp
// Description: Definition of ImagePreProcessor (OpenCV)
//
//------------------------------------------------------------------------------
//
// File generated on Oct 2024 by Rin Baudelet
//------------------------------------------------------------------------------
#ifndef IMAGE_PRE_PROCESSOR_HPP
#define IMAGE_PRE_PROCESSOR_HPP

#include <opencv2/opencv.hpp>

// Enum class defining various types of image preprocessing operations
enum class PreprocessingType {
    GaussianBlur,
    MedianBlur,
    Thresholding,
    HistogramEqualization,
    NoiseCorrection,
    Grayscale
};

class ImagePreProcessor {
private:
    /**
     * Apply Gaussian blur to an image
     * @param img the input image to be blurred
     * @param kernelSize the size of the Gaussian kernel (odd value)
     * @param sigma the standard deviation of the Gaussian kernel
     * @return the image after
     */
    static cv::Mat applyGaussianBlur(const cv::Mat& img, int kernelSize, double sigma);

    /**
     * Apply median blur to an image
     * @param img the input image to be blurred
     * @param kernelSize the size of the kernel (must be odd)
     * @return the image after
     */
    static cv::Mat applyMedianBlur(const cv::Mat& img, int kernelSize);

    /**
     * Apply binary thresholding to an image
     * @param img the input image for thresholding
     * @param thresh the threshold value
     * @param maxVal the maximum value assigned to pixels exceeding the threshold
     * @return the image after
     */
    static cv::Mat applyThresholding(const cv::Mat& img, double thresh, double maxVal);

    /**
     * Apply histogram equalization to an image
     * @param img the input image
     * @return the image after
     */
    static cv::Mat applyHistogramEqualization(const cv::Mat& img);

    /**
     * Apply noise correction to an image
     * @param img the input image
     * @param h the filter strength for noise reduction (default 30.0f)
     * @return the image after
     */
    static cv::Mat applyNoiseCorrection(const cv::Mat& img, float h = 30.0f);

    /**
     * Convert an image to grayscale
     * @param img the input image
     * @return the image in gray
     */
    static cv::Mat applyGrayscale(const cv::Mat& img);

public:
    /**
     * Apply a specified preprocessing operation to an image
     * @param img the input
     * @param type the preprocessing operation to apply (GaussianBlur, Thresholding)
     * @return the image
     */
    static cv::Mat process(const cv::Mat& img, PreprocessingType type);

    /**
     * Apply a predefined series of preprocessing steps to an image
     * @param img the input image to process
     * @return the image after
     */
    static cv::Mat process(const cv::Mat& img);

};

#endif

