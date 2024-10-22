#include <ImagePreProcessor.hpp>

namespace idl
{
    cv::Mat ImagePreProcessor::applyGrayscale(const cv::Mat& img) {
        cv::Mat gray;
        try {
            //check if it's already grayscale
            if (img.channels() == 3) {
                cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            } else {
                return img;
            }
        } catch (const cv::Exception& e) {
            std::cerr << "Error : convertion in Grayscale : " << e.what() << std::endl;
            return img;
        }
        return gray;
    }

    cv::Mat ImagePreProcessor::applyGaussianBlur(const cv::Mat& img, int kernelSize, double sigma) {
        //check if is odd
        if (kernelSize % 2 == 0 || kernelSize < 1) {
            return img;
        }

        cv::Mat output;
        cv::GaussianBlur(img, output, cv::Size(kernelSize, kernelSize), sigma);
        return output;
    }

    cv::Mat ImagePreProcessor::applyHistogramEqualization(const cv::Mat& img) {
        cv::Mat result;

        //check type of chanel gray or RGB
        if (img.channels() == 1) {
            cv::equalizeHist(img, result);
        } else if (img.channels() == 3) {
            std::vector<cv::Mat> channels;
            cv::split(img, channels);
            for (int i = 0; i < 3; i++) {
                cv::equalizeHist(channels[i], channels[i]);
            }
            cv::merge(channels, result);
            return result;
        } else {
            std::cerr << "Error : applyHistogramEqualization" << std::endl;
            return img;
        }

        return result;
    }

    cv::Mat ImagePreProcessor::applyMedianBlur(const cv::Mat& img, int kernelSize) {
        //check if is odd
        if (kernelSize % 2 == 0 || kernelSize < 1) {
            return img;
        }
        cv::Mat output;
        cv::medianBlur(img, output, kernelSize);
        return output;
    }

    cv::Mat ImagePreProcessor::applyThresholding(const cv::Mat& img, double thresh, double maxVal) {
        cv::Mat gray, output;

        if (img.channels() == 3) {
            gray = ImagePreProcessor::applyGrayscale(img);
        } else {
            gray = img;
        }
        cv::threshold(gray, output, thresh, maxVal, cv::THRESH_BINARY);

        return output;
    }

    cv::Mat ImagePreProcessor::applyNoiseCorrection(const cv::Mat& img, float h) {
        cv::Mat output;

        if (img.channels() == 1) {
            cv::Mat imgColor;
            cv::cvtColor(img, imgColor, cv::COLOR_GRAY2BGR);
            cv::fastNlMeansDenoisingColored(imgColor, output, h, h, 7, 21);
            output = ImagePreProcessor::applyGrayscale(imgColor);
        } else if (img.channels() == 3 || img.channels() == 4) {
            cv::fastNlMeansDenoisingColored(img, output, h, h, 7, 21);
        } else {
            std::cerr << "Erreur : Le format de l'image n'est pas pris en charge." << std::endl;
            return output;
        }

        return output;
    }

    //mux for choose PreprocessingType
    cv::Mat ImagePreProcessor::process(const cv::Mat& img, PreprocessingType type) {
        switch(type) {
            case PreprocessingType::GaussianBlur:
                return ImagePreProcessor::applyGaussianBlur(img, 5, 1.5);
            case PreprocessingType::MedianBlur:
                return ImagePreProcessor::applyMedianBlur(img, 5);
            case PreprocessingType::Thresholding:
                return ImagePreProcessor::applyThresholding(img, 128, 255);
            case PreprocessingType::HistogramEqualization:
                return ImagePreProcessor::applyHistogramEqualization(img);
            case PreprocessingType::NoiseCorrection:
                return ImagePreProcessor::applyNoiseCorrection(img);
            default:
                return img;
        }
    }

    // PreprocessingType choose specialy for the subject
    cv::Mat ImagePreProcessor::process(const cv::Mat& img) {
        if (img.empty()) {
            std::cerr << "Error : img void." << std::endl;
            return img;
        }
        cv::Mat imgNew = img;
        /*
        imgNew = ImagePreProcessor::applyGrayscale(imgNew);
        imgNew = ImagePreProcessor::applyMedianBlur(imgNew, 5);
        imgNew = ImagePreProcessor::applyHistogramEqualization(imgNew);
        */

        //imgNew = ImagePreProcessor::applyNoiseCorrection(imgNew, 7);

        return imgNew;                                                
    }
}
