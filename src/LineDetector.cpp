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

        // Use Canny for edge detection
        cvtColor(_img, dst, cv::COLOR_BGR2GRAY);
        cv::Canny(dst, dst, 50, 300, 3, true);
        
        // Apply hough transformation
        std::vector<cv::Vec4i> lines;
        cv::HoughLinesP(dst, oResults, 1, .5*CV_PI/180, 150, 200, 500);

        return oResults;
    }

    cv::Vec3f LineDetector::toLinearEquation(const cv::Vec4i& iLine)
    {
        float a, b, c;
        float x1 = iLine[0];
        float y1 = iLine[1];
        float x2 = iLine[2];
        float y2 = iLine[3];

        a = y2-y1;
        b = x1-x2;
        c = y1 * (x2-x1) - (y2-y1) * x1;
        return cv::Vec3f {a,b,c};
    }

    float LineDetector::computeAngle(const cv::Vec4i& iLine1, const cv::Vec4i& iLine2)
    {
        cv::Vec2f vec1 = 
        {
            static_cast<float>(iLine1[2])-static_cast<float>(iLine1[0]),
            static_cast<float>(iLine1[3])-static_cast<float>(iLine1[1])
        };
        cv::Vec2f vec2 = 
        {
            static_cast<float>(iLine2[2])-static_cast<float>(iLine2[0]),
            static_cast<float>(iLine2[3])-static_cast<float>(iLine2[1])
        };

        return static_cast<float>(
            acos(vec1.dot(vec2)/(cv::norm(vec1)*cv::norm(vec2)))
        );
    }

    std::vector<cv::Point> LineDetector::getIntersections() const
    {
        std::vector<cv::Point> oResults;
        auto lines = getCurLines();

        for (size_t lineIndex = 0; lineIndex < lines.size()-1; lineIndex++)
        {
            auto line1     = lines[lineIndex];
            auto linearEq1 = toLinearEquation(line1); 

            for (size_t compareLineIndex = lineIndex+1; 
                compareLineIndex < lines.size(); 
                compareLineIndex++)
            {
                auto line2     = lines[compareLineIndex];
                auto linearEq2 = toLinearEquation(line2);
    
                if ((computeAngle(line1, line2)) > 0.1f /* threshold */)
                {
                    float data[2][2] = {
                        {linearEq1[0], linearEq1[1]},
                        {linearEq2[0], linearEq2[1]}
                    }; 
                    cv::Mat A (2, 2, CV_32F, data);
                    cv::Vec2f B = {-linearEq1[2], -linearEq2[2]};
                    cv::Vec2f pt;

                    bool isSolve = cv::solve(A, B, pt);
                    if (isSolve 
                        && pt[0] >= 0 && pt[0] < _img.cols
                        && pt[1] >= 0 && pt[1] < _img.rows)
                    {
                        oResults.emplace_back(cv::Point{
                            static_cast<int>(pt[0]), static_cast<int>(pt[1])
                        });
                    }
                }
            }
        }

        return oResults;
    }

    cv::Point LineDetector::getIntersection() const 
    {
        cv::Point oPoint;
        auto points = getIntersections();
        int xTotal = 0, yTotal = 0;
        int ptSize = static_cast<int>(points.size());
        

        for (const auto& pt : points)
        {
            xTotal += pt.x;
            yTotal += pt.y;
        }

        return cv::Point {xTotal / ptSize, yTotal / ptSize};
    }

    void LineDetector::showResults() const 
    {
        cv::Mat dst = _img.clone();
        auto lines = getCurLines();
        auto points = getIntersections();

        std::cout << "Detected " << lines.size() << " line(s)" << std::endl;

        for (const auto& line : lines)
        {
            cv::Point pt1, pt2;
            pt1.x = line[0];
            pt1.y = line[1];
            pt2.x = line[2];
            pt2.y = line[3];
            cv::line(dst, pt1, pt2, cv::Scalar(0,0,255), 2, cv::LINE_AA);
        }

        std::cout << "Detected " << points.size() << " intersection(s)" << std::endl;

        for (const auto& pt : points)
        {
            cv::circle(dst, pt, 3, cv::Scalar(0,255,255), -1);
        }

        cv::circle(dst, getIntersection(), 2, cv::Scalar(0,255,0), -1);

        cv::imshow("Detected Lines (in red) and intersections (in yellow + green)", dst);
        cv::waitKey();
    }
} // namespace idl
