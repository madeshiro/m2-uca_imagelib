#ifndef LINE_DETECTOR_HPP
#define LINE_DETECTOR_HPP

#include <vector>
#include <opencv2/opencv.hpp>

class LineDetector {
public:
    cv::Mat filterLinesColor(const cv::Mat& in); // Filtrer les couleurs de ligne

    LineDetector();				 // Constructeurs
    std::vector<cv::Vec2d> getCurLines();        // Retourner les lignes détectées
    cv::Vec2d getCenter();                       // Retourner le centre des lignes détectées
};

#endif // LINE_DETECTOR_HPP
