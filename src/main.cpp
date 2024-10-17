#include "../include/Main.hpp"
#include "../include/LineDetector.hpp"

int main(){
	LineDetector ld = LineDetector();
	cv::Mat img = cv::imread("../data/im001.png");
	if (img.empty()) {
		std::cerr << "Image not found!" << std::endl;
		return 1;
	}
	cv::Mat imgExtracted = ld.filterLinesColor(img);
	cv::imshow("Original", img);
	cv::imshow("Extracted",imgExtracted);
	cv::waitKey();
	return 0;
}
