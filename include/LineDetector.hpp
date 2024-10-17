//------------------------------------------------------------------------------
//
// File:        LineDetector.hpp
// Description: Definition of LineDetector (OpenCV hough transformation)
//
//------------------------------------------------------------------------------
//
// File generated on Oct 2024 by Rin Baudelet
//------------------------------------------------------------------------------
#ifndef LINE_DETECTOR_HPP
#define LINE_DETECTOR_HPP

#include <vector>
#include <opencv2/opencv.hpp>

namespace idl // Images Development Library 
{
    // Forward declaration
    class ProcessingFactory;

    class LineDetector 
    {
    public:
        // Disallow copy
        LineDetector(const LineDetector&) = delete;
        LineDetector& operator =(const LineDetector&) = delete;

        // Disallow move
        LineDetector(LineDetector&&) noexcept = delete;
        LineDetector& operator =(LineDetector&&) noexcept = delete;

        ~LineDetector() noexcept = default;
    //protected:
        // Disable creation from external classes except factory
        /**
         * Create a line detector from an image to analyze. 
         * @param iImgSrc the image to detect lines from. 
         */
        LineDetector(const cv::Mat& iImgSrc);
    public:
        /** 
         * Get detected lines using hough transformation.
         * @return a list of vec4i (x0, y0, x1, y1) 
         */
        std::vector<cv::Vec4i> getCurLines() const;  

        /**
         * Computes intersection between lines
         */
        std::vector<cv::Point> getIntersections() const;   

        /**
         * Get the intersection by computing the mean points between every detect intersections
         * @see LineDetector::getIntersections()
         */
        cv::Point getIntersection() const;

        /**
         * For debug purpose. 
         * Display the line onto the origin image. 
         */
        void showResults() const;           
    private:
        // Internal functions
        static cv::Vec3f toLinearEquation(const cv::Vec4i&);
        static float computeAngle(const cv::Vec4i& iLine1, const cv::Vec4i& iLine2);
        cv::Mat filterLinesColor(const cv::Mat& iImg); // Filtrer les couleurs de ligne

        // Attributes
        const cv::Mat& _img; //< reference image to analyze

        // Factory
        friend class idl::ProcessingFactory;
    };
} // namespace idl

#endif // LINE_DETECTOR_HPP
