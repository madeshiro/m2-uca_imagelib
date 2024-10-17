#include <iostream>
#include <opencv2/opencv.hpp>
#include <LineDetector.hpp>
#include <Plant.hpp>
#include <PlantDetector.hpp>
#include <JetPositionChecker.hpp>
#include <ImagePreProcessor.hpp>

using namespace cv;

int main()
{
    idl::PlantDetector detector;

    cv::Mat image = cv::imread("../data/im001.png", cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Error: Image not found" << std::endl;
        return 1;
    }

    std::vector<cv::Mat> images;

    cv::String path = "../data/*.png";
    std::vector<cv::String> dataFileNames;
    cv::glob(path, dataFileNames, false);
    for(size_t k = 0; k < dataFileNames.size(); k++){
    	cv::Mat image = cv::imread(dataFileNames[k], cv::IMREAD_COLOR);
    	image = idl::ImagePreProcessor::process(image);
        images.push_back(image);
    }

    cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);

    cv::Mat ranged;
    int i = 0;
    while(i <= images.size())
    {    
        std::vector<idl::Plant> plants = detector.detectPlants(images[i]);
        cv::Mat image2 = images[i].clone();
        idl::LineDetector ldetector(images[i]);
        idl::JetPositionChecker jetChecker(plants, ldetector);

        for (const auto& plant : plants)
        {
            Mat mask;
            cv::rectangle(image2, plant.boundingBox, plant.plantSpecies == idl::Species::wheat ? cv::Scalar(255,0,0) : cv::Scalar(0,255,0), 2);

            int xMin = plant.position[0], xMax = xMin+plant.mask.cols;
            int yMin = plant.position[1], yMax = yMin+plant.mask.rows;
            
            cv::cvtColor(plant.mask, mask, cv::COLOR_GRAY2BGR);
            mask.copyTo(image2.rowRange(yMin, yMax).colRange(xMin, xMax));
        }

        if (ldetector.hasIntersection())
        {
            cv::Scalar jetPointColor = cv::Scalar(0,0,0);
            switch (jetChecker.computeState())
            {
                case LaserBehavior::onAdventis:
                    jetPointColor = cv::Scalar(0,255,0);
                    break;
                case LaserBehavior::onWheat:
                    jetPointColor = cv::Scalar(0,0,255);
                    break;
                case LaserBehavior::onNothing:
                    jetPointColor = cv::Scalar(0,255,255);
                    break;
            }
            cv::circle(image2, ldetector.getIntersection(), 4, jetPointColor, -1);
        }

        cv::imshow("Image", image2);
        if(cv::waitKey(3000) == 27){
            break;
        }

        i++;
        if(i >= images.size()){
            i = 0;
        }
    }

    return 0;
}
