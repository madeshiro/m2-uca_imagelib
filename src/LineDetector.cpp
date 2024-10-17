#include "../include/LineDetector.hpp"
#include <iostream>

LineDetector::LineDetector(){}

cv::Mat LineDetector::filterLinesColor(const cv::Mat& in){
    //DEF STATIC VAR
    cv::Vec3i targetColor = {163, 101, 139};
    int threshold = 30;
    cv::Mat imgLab;
    cv::cvtColor(in,imgLab,cv::COLOR_BGR2Lab);
    cv::Mat out(imgLab.rows, imgLab.cols, CV_8UC1);
    for (int j = 0; j < imgLab.rows; j++)
    {
        for (int i = 0; i < imgLab.cols; i++)
        {
            cv::Vec3b pixel = imgLab.at<cv::Vec3b>(cv::Point{i,j});
            double distance = abs(targetColor[0]-pixel[0]) + abs(targetColor[1]-pixel[1]) + abs(targetColor[2]-pixel[2]);

            if(distance<threshold){
                out.at<uchar>(cv::Point{i,j})=255;
            } else {
                out.at<uchar>(cv::Point{i,j})=0;
            }
        }
    }
    
    return out;
}
