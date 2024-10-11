#include "LineDetector.hpp"
#include <iostream>

namespace idl 
{
    cv::Mat LineDetector::filterLinesColor(const cv::Mat& in)
    {
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

    LineDetector::LineDetector(const cv::Mat& iImgSrc):
        _img(iImgSrc)
    {
    }

    std::vector<cv::Vec4i> LineDetector::getCurLines() const 
    {
        // output 
        std::vector<cv::Vec4i> oResults; 
        
        // intermediary variables
        cv::Mat dst, cdst;  


        // Use canny for edge detection
        cv::Canny(_img, dst, 50, 200, 3);
        cvtColor(dst, cdst, cv::COLOR_GRAY2BGR);
        
        // Apply hough transformation
        std::vector<cv::Vec2f> lines;
        cv::HoughLines(dst, lines, 1, CV_PI/180, 150, 0, 0);

        // Fill results vector
        for (const auto& line : lines)
        {
            // Line parameters
            float rho = line[0];
            float theta = line[1];
            
            double a = cos(theta), b = sin(theta);
            double x0 = a*rho, y0 = b*rho;
            
            // Define points
            cv::Vec4i resultLine;
            resultLine[0] = cvRound(x0 + 1000*(-b));
            resultLine[1] = cvRound(y0 + 1000*(a));
            resultLine[2] = cvRound(x0 - 1000*(-b));
            resultLine[3] = cvRound(y0 - 1000*(a));

            oResults.emplace_back(resultLine);
        }

        return oResults;
    }

    void LineDetector::showResults() const 
    {
        cv::Mat dst = _img.clone();
        auto lines = getCurLines();
        for (const auto line : lines)
        {
            cv::Point pt1, pt2;
            pt1.x = line[0];
            pt1.y = line[1];
            pt2.x = line[2];
            pt2.y = line[3];
            cv::line(dst, pt1, pt2, cv::Scalar(0,0,255), 3, cv::LINE_AA);
        }

        cv::imshow("Detected Lines (in red)", dst);
        cv::waitKey();
    }
} // namespace idl
