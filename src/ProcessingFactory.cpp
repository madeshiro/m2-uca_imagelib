#include "ProcessingFactory.hpp"

namespace idl
{
    ProcessingFactory::ImageProcessing::ImageProcessing(cv::Mat&& iImage, std::string&& nImage)
        :   _nameImg(std::move(nImage)), _img(std::move(iImage)), _plants(PlantDetector::detectPlants(_img)), 
            _lineDetector(new LineDetector(_img)), 
            _jetChecker(new JetPositionChecker(_plants, *_lineDetector))
    {
    }

    ProcessingFactory::ImageProcessing::ImageProcessing(const ImageProcessing& iOther)
        : _nameImg(iOther._nameImg), _img(iOther._img.clone()), _plants(iOther._plants), 
          _lineDetector(new LineDetector(_img)),
          _jetChecker(new JetPositionChecker(_plants, *_lineDetector))
    {
    }

    ProcessingFactory::ImageProcessing::~ImageProcessing()
    {
        delete _lineDetector;
        delete _jetChecker;
    }

    /**
     * Format a vector of 2D positions into a string
     * like "(x1; y1)/ (x2; y2)/"
     * @param vector of plant
     * @return string representing 2D positions
     */
    std::string formatPositions(const std::vector<Plant>& plants) 
    {
        std::stringstream ssAdventis;
        std::stringstream ssWheat;
        for (const auto& p : plants) {
            std::stringstream ss;
            ss << "(" << p.center[0] << "; " << p.center[1] << ")/ ";

            if(p.plantSpecies == Species::advantis)
            {
                ssAdventis << ss.str();
            }
            else if (p.plantSpecies == Species::wheat)
            {
                ssWheat << ss.str();
            }
        }
        std::string result = ssAdventis.str() + ", " + ssWheat.str();

        return result;
    }

    void ProcessingFactory::ImageProcessing::write(std::ofstream& csvFile) const 
    {
        //check if is open
        if (!csvFile.is_open()) {
            std::cerr << "Failed to open file" << std::endl;
            return;
        }

        std::string plantPosStr = formatPositions(_plants);

        //type of beahvior the laser 
        std::string laserOnStr = "Unknown";
        switch (_jetChecker->computeState()) {
            case LaserBehavior::onNothing:
                laserOnStr = "onNothing";
                break;
            case LaserBehavior::onAdventis:
                laserOnStr = "onWheat";
                break;
            case LaserBehavior::onWheat:
                laserOnStr = "onAdventis";
                break;
            default:
                laserOnStr = "No laser";
                break;
        }
        cv::Point intersectionLaser = _lineDetector->getIntersection();
        csvFile << _nameImg << ", ("
                << intersectionLaser.x << "; " << intersectionLaser.y << "), "
                << laserOnStr << ", "
                << "\"" << plantPosStr << "\"\n";
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

    ProcessingFactory::ProcessingFactory(const std::string& iImgDirectory)
    {
        cv::String path = iImgDirectory + "/*.png";
        std::vector<cv::String> dataFileNames;
        cv::glob(path, dataFileNames, false);
        for (const auto& fileName : dataFileNames)
        {
            auto img = cv::imread(fileName, cv::IMREAD_COLOR);
            if (img.empty()) {
            std::cerr << "Error: Could not load image " << fileName << std::endl;
            continue; // Skip to the next file
}
            //img = ImagePreProcessor::process(img);
            std::string fileNameStr = fileName.substr(fileName.find_last_of("/") + 1);;
            ImageProcessing imgProce = ImageProcessing {std::move(img), std::move(fileNameStr)};
            _listOfProcess.emplace_back(imgProce);
        }
    }

    const ProcessingFactory::ImageProcessing& ProcessingFactory::operator[](size_t iIndex) const 
    { 
        return _listOfProcess[iIndex]; 
    }
}