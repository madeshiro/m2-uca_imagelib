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


#include <filesystem>  // Utilisation correcte du header filesystem

#ifdef __APPLE__
    namespace fs = std::__fs::filesystem;  // Sur macOS, utilisez __fs::filesystem
#else
    namespace fs = std::filesystem;  // Sur les autres systèmes, utilisez std::filesystem
#endif

     /**Get the current date and time as a formatted string
     * The date and time format is: YYYY-MM-DD_HH-MM-SS
     * @return string representing date and time
     */
    std::string getCurrentDateTime() 
    {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct); //YYYY-MM-DD_HH-MM-SS
        return std::string(buf);
    }

    /**
     * Generates and writes data to a CSV file with the name format
     * "YYYY-MM-DD_HH-MM-SS_plantCheck.csv". It records the image name, laser intersection point,
     * laser state, plant positions, and advantis positions
     * @param factory containing data for each image and path for save file
     */

    void generateCSV(const idl::ProcessingFactory& factory, const fs::path pathFolder) {

        //check if the folder exists
        if (!fs::exists(pathFolder)) 
        {
            std::cerr << "Error: The directory '" << pathFolder << "' does not exist." << std::endl;
            return;
        }

        //The name of file
        std::string filename = getCurrentDateTime() + "_plantCheck.csv";
        fs::path fullFilePath = pathFolder / filename;

        //create folder
        std::ofstream csvFile;
        csvFile.open(fullFilePath, std::ios::app);
        if (!csvFile.is_open()) {
            std::cerr << "Error: Unable to open or create the CSV file." << std::endl;
            return;
        }

        // Write the header
        csvFile << "Image Name, Laser Intersection (X; Y), LaserOn, Advantis Positions (X; Y), Weed Positions\n";

        //write all data
        for(const auto& ImageProc : factory.listProcessing())
        {
            ImageProc.write(csvFile);
        }

        csvFile.close();
        std::cout << "CSV file successfully created and updated!" << std::endl;
        
    }


int main()
{
    idl::ProcessingFactory factory("../data");
    std::cout << "Found " << factory.listProcessing().size() << " image(s)!" << std::endl;

    cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);

    //create a new folder
    std::string directoryPath = "./" + getCurrentDateTime() + "WeedProj_Results";

    if (!fs::exists(directoryPath)) {
        //if doesn't exist create the folder
        if (!fs::create_directory(directoryPath)) {
            std::cerr << "Error : for create folder" << std::endl;
            return 1;
        }
    }
    fs::path savedPath = fs::absolute(directoryPath);

    for(int i = 0; i < factory.listProcessing().size(); ++i)
    {   
         
        cv::Mat imgs[2] = {
            factory[i].getImageWithDetails(),
            factory[i].getImageWithMasks()
        };
        std::string baseImageName = factory[i].getImageName(); //name of img

        // path file img
        fs::path imgDetailsPath = savedPath / (baseImageName + "_details.png");
        fs::path imgMaskPath = savedPath / (baseImageName + "_mask.png");

        // save img
        if (!cv::imwrite(imgDetailsPath.string(), imgs[0])) 
        {
            std::cerr << "Error: Failed to save image for '" << baseImageName << "'" << std::endl;
        }
        if (!cv::imwrite(imgMaskPath.string(), imgs[1])) 
        {
            std::cerr << "Error: Failed to save image for '" << baseImageName << "'" << std::endl;
        }

        // display picture
        for (auto img : imgs)
        {
            char windowTitle[60];
            std::snprintf(windowTitle, 60, "Image #%d", i);
            cv::imshow("Image", img);
            if(cv::waitKey(200) == 27){
                return 0;
            }
        }
    }

    //generate CSV
    generateCSV(factory, savedPath);

    return 0;
}


