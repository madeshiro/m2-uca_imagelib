#include <iostream>
#include <opencv2/opencv.hpp>
#include <LineDetector.hpp>
using namespace cv;

int main()
{
    for (size_t i=1; i < 9; i++)
    {
        char buf[255];
        sprintf(buf, "../data/im00%d.png", i);    
        Mat src = imread(buf);

        if (src.empty())
        {
            std::cout << "Image not found!" << std::endl;
            continue;
        }
        
        idl::LineDetector ld(src);
        if (ld.getCurLines().size()==0)
        {
            std::cout << "Lines not found!" << std::endl;
            continue;
        }

        ld.getIntersections();
        ld.showResults();
    }
    return 0;
}
