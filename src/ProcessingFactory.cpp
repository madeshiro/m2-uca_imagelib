#include "ProcessingFactory.hpp"

namespace idl
{
    ProcessingFactory::ImageProcessing::ImageProcessing(cv::Mat&& iImage)
        :   _img(std::move(iImage)), _plants(PlantDetector::detectPlants(_img)), 
            _lineDetector(new LineDetector(_img)), 
            _jetChecker(new JetPositionChecker(_plants, *_lineDetector))
    {
    }

    ProcessingFactory::ImageProcessing::ImageProcessing(const ImageProcessing& iOther)
        : _img(iOther._img.clone()), _plants(iOther._plants), 
          _lineDetector(new LineDetector(_img)),
          _jetChecker(new JetPositionChecker(_plants, *_lineDetector))
    {
    }

    ProcessingFactory::ImageProcessing::~ImageProcessing()
    {
        delete _lineDetector;
        delete _jetChecker;
    }

    void ProcessingFactory::ImageProcessing::write(std::ostream&) const 
    {
        // print to csv 
    }

    cv::Mat ProcessingFactory::ImageProcessing::getImage() const 
    {
        return _img.clone();
    }

    cv::Mat ProcessingFactory::ImageProcessing::getImageWithDetails() const 
    {
        cv::Mat detailedImage;
        if (_lineDetector)
        {
            detailedImage = _lineDetector->drawResults();

            for (const auto& plant : _plants)
            {
                cv::Point pt = {static_cast<int>(plant.center[0]), static_cast<int>(plant.center[1])};
                cv::circle(detailedImage, pt, 5, cv::Scalar(255,0,0), -1);
            }

            const char* laserState = nullptr;
            switch (getLaserBehavior())
            {
            case LaserBehavior::onAdventis:
                laserState = "on adventis";
                break;
            case LaserBehavior::onWheat:
                laserState = "on wheat";
                break;
            case LaserBehavior::onNothing:
                laserState = "on ground";
                break;
            default:
                laserState = "not detected";
                break;
            }

            cv::putText(detailedImage, laserState, {10,30}, cv::FONT_HERSHEY_SIMPLEX,
            1, cv::Scalar(255,255,255), 2, cv::LINE_AA);
        }

        return detailedImage;
    }

    cv::Mat ProcessingFactory::ImageProcessing::getImageWithMasks() const 
    {
        cv::Mat imageWithMasks = _img.clone();

        for (const auto& plant : _plants)
        {
            cv::Mat mask;

            cv::rectangle(imageWithMasks, plant.boundingBox, 
                plant.plantSpecies == idl::Species::wheat ? 
                    cv::Scalar(255,0,0) : cv::Scalar(0,255,0), 2);

            int xMin = plant.position[0], xMax = xMin + plant.mask.cols;
            int yMin = plant.position[1], yMax = yMin + plant.mask.rows;

            cv::cvtColor(plant.mask, mask, cv::COLOR_GRAY2BGR);
            mask.copyTo(imageWithMasks.rowRange(yMin, yMax).colRange(xMin, xMax));
        }

        return imageWithMasks;
    }

    LaserBehavior ProcessingFactory::ImageProcessing::getLaserBehavior() const 
    {
        return _jetChecker->computeState();
    }

    ProcessingFactory::ProcessingFactory(const std::string& iImgDirectory)
    {
        cv::String path = iImgDirectory + "/*.png";
        std::vector<cv::String> dataFileNames;
        cv::glob(path, dataFileNames, false);
        for (const auto& fileName : dataFileNames)
        {
            auto img = cv::imread(fileName, cv::IMREAD_COLOR);
            // img = ImagePreProcessor::process(img);
            _listOfProcess.emplace_back(ImageProcessing {std::move(img)});
        }
    }

    const ProcessingFactory::ImageProcessing& ProcessingFactory::operator[](size_t iIndex) const 
    { 
        return _listOfProcess[iIndex]; 
    }
}