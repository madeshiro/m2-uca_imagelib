#include "../include/ImagePreProcessor.hpp"
#include <opencv2/opencv.hpp>

cv::Mat convertToColor(const cv::Mat& img) {
    cv::Mat imgColor;
    if (img.channels() == 1) {
        cv::cvtColor(img, imgColor, cv::COLOR_GRAY2BGR);
    } else {
        imgColor = img;
    }
    return imgColor;
}

void displayImages(const std::vector<cv::Mat>& images, int imagesPerRow) {
    if (images.empty()) {
        std::cerr << "Erreur : La liste d'images est vide." << std::endl;
        return;
    }

    // calcul dil
    int maxRows = (images.size() + imagesPerRow - 1) / imagesPerRow;
    int imgWidth = images[0].cols;
    int imgHeight = images[0].rows;
    cv::Mat compositeImage(maxRows * imgHeight, imagesPerRow * imgWidth, images[0].type(), cv::Scalar(255, 255, 255));

    for (size_t i = 0; i < images.size(); ++i) {
        int row = i / imagesPerRow; 
        int col = i % imagesPerRow; 
        cv::Rect roi(col * imgWidth, row * imgHeight, imgWidth, imgHeight);

        cv::Mat imgToCopy = convertToColor(images[i]);
        imgToCopy.copyTo(compositeImage(roi)); 
    }

    cv::imshow("Comparaison des Images", compositeImage);
    cv::waitKey(0);
}


int main() {
    cv::Mat img = cv::imread("../data/im006.png");
    if (img.empty()) {
        std::cerr << "Erreur : Impossible de charger l'image." << std::endl;
        return -1;
    }

    // Application des traitements 
    cv::Mat imgGrayscale = ImagePreProcessor::applyGrayscale(img);
    cv::Mat imgHistEqualized = ImagePreProcessor::applyHistogramEqualization(imgGrayscale);
    cv::Mat imgGaussianBlur = ImagePreProcessor::applyGaussianBlur(imgHistEqualized, 7, 1);
    cv::Mat imgMedianBlur = ImagePreProcessor::applyMedianBlur(imgHistEqualized, 3);
    cv::Mat imgDenoised = ImagePreProcessor::applyNoiseCorrection(img, 10);

    std::vector<cv::Mat> images;
    images.push_back(img);
    images.push_back(imgGaussianBlur);
    images.push_back(imgMedianBlur);
    images.push_back(imgDenoised);

    displayImages(images, 2); 

    // Application des traitements 
    cv::Mat imgGrayscaleV2 = ImagePreProcessor::applyGrayscale(img);
    cv::Mat imgHistEqualizedV2 = ImagePreProcessor::applyHistogramEqualization(imgGrayscaleV2);
    cv::Mat imgGaussianBlurE = ImagePreProcessor::applyGaussianBlur(imgHistEqualizedV2, 7, 15);
    cv::Mat imgMedianBlur1 = ImagePreProcessor::applyMedianBlur(imgHistEqualizedV2, 5);
    cv::Mat imgDenoised2 = ImagePreProcessor::applyNoiseCorrection(imgHistEqualizedV2, 5);

    cv::Mat imgMedianBlur12 = ImagePreProcessor::applyMedianBlur(imgGrayscaleV2, 5);
    cv::Mat imgMedianBlurEQUA = ImagePreProcessor::applyHistogramEqualization(imgMedianBlur12);



    std::vector<cv::Mat> imagesV2;
    imagesV2.push_back(img);
    imagesV2.push_back(imgMedianBlur1);
    imagesV2.push_back(imgMedianBlurEQUA);


    displayImages(imagesV2, 2); 

    return 0;
}
