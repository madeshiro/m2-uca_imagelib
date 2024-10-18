#include <iostream>
#include <opencv2/opencv.hpp>
#include <LineDetector.hpp>
#include <Plant.hpp>
#include <PlantDetector.hpp>
#include <JetPositionChecker.hpp>
#include <ImagePreProcessor.hpp>
#include <LaserBehavior.hpp>
#include <ProcessingFactory.hpp>

#include <fstream>
#include <vector>
#include <string>
#include <ctime>  // To generate the current date and time
#include <sstream> // For formatting the filename
/**
 * @struct ImageData
 * Struct to store the data for each image tested
*/
struct ImageData {
    std::string imageName;
    cv::Point laserIntersection;
    std::vector<cv::Vec2d> plantPositions;
    std::vector<cv::Vec2d> advantisPositions;
    laserBehavior laserOnObject; 
};

/**Get the current date and time as a formatted string
 * The date and time format is: YYYY-MM-DD_HH-MM-SS
 * @return string representing date and time
 */
std::string getCurrentDateTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct); //YYYY-MM-DD_HH-MM-SS
    return std::string(buf);
}

/**
 * Format a vector of 2D positions into a string
 * like "(x1; y1)/ (x2; y2)/"
 * @param vector of 2D points (cv::Vec2d)
 * @return string representing 2D positions
 */
std::string formatPositions(const std::vector<cv::Vec2d>& positions) {
    std::stringstream ss;
    for (const auto& pos : positions) {
        ss << "(" << pos[0] << "; " << pos[1] << ")/ ";
    }
    std::string result = ss.str();
    if (!result.empty()) {
        result.pop_back();  // Remove last space
        result.pop_back();  // Remove last comma
    }
    return result;
}

/**
 * Generates and writes data to a CSV file with the name format
 * "YYYY-MM-DD_HH-MM-SS_plantCheck.csv". It records the image name, laser intersection point,
 * laser state, plant positions, and advantis positions.
 * @param imageDataList A vector of ImageData structs containing data for each image.
 */
void generateCSV(const std::vector<ImageData>& imageDataList) {

    //The name of file
    std::string filename = getCurrentDateTime() + "_plantCheck.csv";

    std::ofstream csvFile;
    csvFile.open(filename, std::ios::app);
    if (!csvFile.is_open()) {
        std::cerr << "Error: Unable to open or create the CSV file." << std::endl;
        return;
    }

    // Write the header
    csvFile << "Image Name, Laser Intersection (X; Y), LaserOn, Advantis Positions (X; Y), Weed Positions\n";

    // Write the data for each image
    for (const auto& data : imageDataList) {
        std::string plantPosStr = formatPositions(data.plantPositions);
        std::string weedPosStr = formatPositions(data.advantisPositions);

        //type of beahvior the laser 
        std::string laserOnStr = "Unknown";
        switch (data.laserOnObject) {
            case laserBehavior::onNothing:
                laserOnStr = "onNothing";
                break;
            case laserBehavior::onAdventis:
                laserOnStr = "onWheat";
                break;
            case laserBehavior::onWheat:
                laserOnStr = "onAdventis";
                break;
            default:
                laserOnStr = "Unknown";
                break;
        }

        csvFile << data.imageName << ", ("
                << data.laserIntersection.x << "; " << data.laserIntersection.y << "), "
                << laserOnStr << ", "
                << "\"" << plantPosStr << "\"" << ", "
                << "\"" << weedPosStr << "\"\n";
    }

    csvFile.close();
    std::cout << "CSV file '" << filename << "' successfully created and updated!" << std::endl;
}

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
    	//image = idl::ImagePreProcessor::process(image);
        images.push_back(image);
    }

    cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);

    cv::Mat ranged;
    std::vector<ImageData> imageDataList; 
    int i = 0;
    while(1)
    while(1)
    {    
        std::vector<idl::Plant> plants = detector.detectPlants(images[i]);
        cv::Mat image2 = images[i].clone();

        //generateCSV with this varaiable
        std::vector<cv::Vec2d> plantPositions;
        std::vector<cv::Vec2d> advantisPositions;

        for(int j = 0; j < plants.size(); j++){
            cv::rectangle(image2, plants[j].boundingBox, plants[j].plantSpecies == idl::Species::wheat ? cv::Scalar(255,0,0) : cv::Scalar(0,255,0), 2);
            //generateCSV fill variable
            if (plants[j].plantSpecies == idl::Species::wheat) {
                plantPositions.push_back(plants[j].position);
            } else {
                advantisPositions.push_back(plants[j].position);
            }
        }

        // Detect the laser intersection
        //idl::LineDetector lineDetector(image2);
        cv::Point laserIntersection(204, 208);
       	// Get image name
        std::string imageName = dataFileNames[i].substr(dataFileNames[i].find_last_of("/\\") + 1);
        laserBehavior imagelaserOn = laserBehavior::onWheat;

        // Store the data for CSV
        ImageData imageData;
        imageData.imageName = imageName;
        imageData.laserIntersection = laserIntersection;
        imageData.plantPositions = plantPositions;
        imageData.advantisPositions = advantisPositions;
        imageData.laserOnObject = imagelaserOn; 

        imageDataList.push_back(imageData);


        cv::imshow("Image", image2);
        if(cv::waitKey(100) == 27){
            break;
        }

        i++;
        if(i >= images.size()){
        	break;
            //i = 0;
        }
        
    }

    generateCSV(imageDataList);

    return 0;
}
