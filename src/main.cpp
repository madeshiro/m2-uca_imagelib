#include <iostream>
#include <opencv2/opencv.hpp>
#include <LineDetector.hpp>
#include <Plant.hpp>
#include <PlantDetector.hpp>
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
    	image = ImagePreProcessor::process(image);
        images.push_back(image);
    }

    cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);

    cv::Mat ranged;
    int i = 0;
    while(i <= images.size())
    {    
        std::vector<idl::Plant> plants = detector.detectPlants(images[i]);
        cv::Mat image2 = images[i].clone();

        for(int j = 0; j < plants.size(); j++){
            cv::rectangle(image2, plants[j].boundingBox, plants[j].plantSpecies == idl::Species::wheat ? cv::Scalar(255,0,0) : cv::Scalar(0,255,0), 2);
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
