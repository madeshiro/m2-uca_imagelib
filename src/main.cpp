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
     * @param factory containing data for each image
     */

    void generateCSV(const idl::ProcessingFactory& factory) {
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

        //write all data
        for(const auto& ImageProc : factory.listProcessing())
        {
            ImageProc.write(csvFile);
        }

        csvFile.close();
        std::cout << "CSV file '" << filename << "' successfully created and updated!" << std::endl;
        
    }


int main()
{
    idl::ProcessingFactory factory("../data");
    std::cout << "Found " << factory.listProcessing().size() << " image(s)!" << std::endl;

    cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);

    int i = 0;
    while(1)
    {   
         
        cv::Mat imgs[2] = {
            factory[i].getImageWithDetails(),
            factory[i].getImageWithMasks()
        };

        for (auto img : imgs)
        {
            char windowTitle[60];
            std::snprintf(windowTitle, 60, "Image #%d", i);
            cv::imshow("Image", img);
            if(cv::waitKey(100) == 27){
                return 0;
            }
        }
    
        i = (++i%factory.listProcessing().size());
        if(0 == i)
        {
            generateCSV(factory);
            return 0;
        }

    }

    return 0;
}


