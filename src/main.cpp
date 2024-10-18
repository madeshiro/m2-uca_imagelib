#include <iostream>
#include <opencv2/opencv.hpp>
#include <LineDetector.hpp>
#include <Plant.hpp>
#include <PlantDetector.hpp>
#include <JetPositionChecker.hpp>
#include <ImagePreProcessor.hpp>
#include <ProcessingFactory.hpp>

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
            if(cv::waitKey(3000) == 27){
                return 0;
            }
        }

        i = (++i%factory.listProcessing().size());
    }

    return 0;
}
