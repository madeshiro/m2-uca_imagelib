#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


cv::Mat ElimColor(cv::Mat in, cv::Scalar min, cv::Scalar max, int morph_size = 5, int inpaint_size = 5) {
    cv::Mat result;

    //to hsv
    cv::Mat hsv;
    cv::cvtColor(in, hsv, cv::COLOR_BGR2HSV);

    //remove cyan from the image
    cv::inRange(hsv, min, max, result);

    //apply a mask to the image
    cv::Mat masked;
    //fatten the mask
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morph_size, morph_size));
    cv::dilate(result, result, kernel);
    //invert mask
    cv::Mat resn;
    cv::bitwise_not(result, resn);
    //apply mask
    cv::bitwise_and(in, in, masked, resn);
    //repair the masked image (fill in the black holes left by the mask with nearby pixels)
    cv::inpaint(masked, result, masked, inpaint_size, cv::INPAINT_TELEA);

    return masked;
}


int main() {
    cv::Mat image = cv::imread("../data/im001.png", cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Error: Image not found" << std::endl;
        return 1;
    }

    //remove color RGB 93, 101, 110 from the image
        //cv::inRange(masked, cv::Scalar(80, 90, 90), cv::Scalar(100, 110, 120), masked);
        //ideal: (103,73,69) (136,119,118)

    std::vector<cv::Mat> images;

    cv::String path = "../data/*.png";
    std::vector<cv::String> fn;
    cv::glob(path, fn, false);
    for(size_t k = 0; k < fn.size(); k++){
        images.push_back(cv::imread(fn[k], cv::IMREAD_COLOR));
    }

    cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);

    cv::Mat ranged;
    int i = 0;
    while(1){
        i ++;
        if(i >= images.size()){
            i = 0;
        }

        cv::Mat image = images[i];

        cv::Mat masked = ElimColor(image, cv::Scalar(80, 100, 100), cv::Scalar(100, 255, 255), 5, 5);
        cv::inRange(masked, cv::Scalar(103,73,69), cv::Scalar(136,119,118), ranged);

        cv::Mat binary;
        cv::threshold(ranged, binary, 0, 255, cv::THRESH_BINARY);

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for(int i = 0; i < contours.size(); i++){
            cv::Rect rect = cv::boundingRect(contours[i]);
            for(int j = 0; j < contours.size(); j++){
                if(i != j){
                    cv::Rect rect2 = cv::boundingRect(contours[j]);
                    if(rect.x < rect2.x + rect2.width && rect.x + rect.width > rect2.x && rect.y < rect2.y + rect2.height && rect.y + rect.height > rect2.y){
                        if(rect.width * rect.height > rect2.width * rect2.height){
                            contours.erase(contours.begin() + j);
                        }else{
                            contours.erase(contours.begin() + i);
                        }
                    }
                }
            }
        }
        cv::Mat image2 = image.clone();

        for(int i = 0; i < contours.size(); i++){
            cv::Scalar color = cv::Scalar(0, 255, 0);
            if(cv::contourArea(contours[i]) < 300){
                color = cv::Scalar(0, 0, 255);
            }

            cv::Rect rect = cv::boundingRect(contours[i]);
            cv::rectangle(image2, rect, color, 2);
        }

        cv::imshow("Image", image2);
        if(cv::waitKey(3000) == 27){
            break;
        }
    }

    return 0;
}