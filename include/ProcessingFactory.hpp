//------------------------------------------------------------------------------
//
// File:        LineDetector.hpp
// Description: Definition of LineDetector (OpenCV hough transformation)
//
//------------------------------------------------------------------------------
//
// File generated on Oct 2024 by Rin Baudelet
//------------------------------------------------------------------------------
#ifndef PROCESSING_FACTORY_HPP
#define PROCESSING_FACTORY_HPP
#include "LineDetector.hpp"
#include "PlantDetector.hpp"
#include "ImagePreProcessor.hpp"
#include "JetPositionChecker.hpp"
// OpenCV
#include <opencv2/opencv.hpp>
// STL
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

namespace idl
{
    class ProcessingFactory 
    {  
    public:
        class ImageProcessing
        {
            friend class ProcessingFactory;
        protected:
            ImageProcessing(cv::Mat&& iImage, std::string&& nImage);
        public:
            ImageProcessing() = default;

            ImageProcessing(const ImageProcessing&);
            ImageProcessing& operator =(const ImageProcessing&);

            ~ImageProcessing();

            /**
             * Write results to CSV in an output stream. 
             */
            void write(std::ofstream&) const;

            /**
             * @return the original image
             */
            cv::Mat getImage() const;

            /**
             * @return the original image with the detected laser and 
             * plants gravity center. 
             */
            cv::Mat getImageWithDetails() const;
            /**
             * @return the original image with plants mask draw above.
             */
            cv::Mat getImageWithMasks() const;
        private:
            std::string _nameImg;
            cv::Mat _img;
            std::vector<Plant> _plants;
            LineDetector* _lineDetector     = nullptr;    
            JetPositionChecker* _jetChecker = nullptr;
        };
    private:
        ProcessingFactory(const ProcessingFactory&) = delete;
        ProcessingFactory(ProcessingFactory&&) noexcept = delete;

        ProcessingFactory& operator =(const ProcessingFactory&) = delete;
        ProcessingFactory& operator =(ProcessingFactory&&) noexcept = delete;        
    public:
        /**
         * Create a new image processing pipeline. 
         * 
         * @param iImgDirectory a directory containing png file label as img###.png 
         *                      with ### the number of the file from 000 to 100 (in order)
         */
        ProcessingFactory(const std::string& iImgDirectory);

        /**
         * List each process create for every image
         */
        constexpr const std::vector<ImageProcessing>& listProcessing() const
        { return _listOfProcess; }

        const ImageProcessing& operator[](size_t index) const;
    private:
        std::vector<ImageProcessing> _listOfProcess;
    };
}

#endif // PROCESSING_FACTORY_HPP
