#include "../include/PlantDetector.hpp"
#include "../include/Plant.hpp"
#include "../include/Species.hpp"

cv::Mat PlantDetector::ElimColor(cv::Mat in, cv::Scalar min, cv::Scalar max, int morph_size = 5, int inpaint_size = 5) {
    cv::Mat result;

    //to hsv
    cv::Mat hsv;
    cv::cvtColor(in, hsv, cv::COLOR_BGR2HSV);

    //remove cyan from the image
    cv::inRange(hsv, min, max, result);

    //apply a mask to the image
    cv::Mat masked;
    //thicken the mask
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morph_size, morph_size));
    cv::dilate(result, result, kernel);
    //invert mask
    cv::Mat resn;
    cv::bitwise_not(result, resn);
    //apply mask
    cv::bitwise_and(in, in, masked, resn);
    //repair the masked image (fill in the black holes left by the mask with nearby pixels)
    cv::inpaint(masked, result, masked, inpaint_size, cv::INPAINT_TELEA);

    return masked;
}

std::vector<Plant> PlantDetector::detectPlants(const cv::Mat& img){
    cv::Mat image = img.clone();

    cv::Mat ranged;

    cv::Mat masked = ElimColor(image, cv::Scalar(80, 100, 100), cv::Scalar(100, 255, 255), 5, 5);
    cv::inRange(masked, cv::Scalar(103,73,69), cv::Scalar(136,119,118), ranged);

    cv::Mat binary;
    cv::threshold(ranged, binary, 0, 255, cv::THRESH_BINARY);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

    //grow the blobs
    cv::dilate(binary, binary, kernel);
    cv::dilate(binary, binary, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for(int i = 0; i < contours.size(); i++){
        cv::Rect rect = cv::boundingRect(contours[i]);
        for(int j = 0; j < contours.size(); j++){
            if(i != j){
                cv::Rect rect2 = cv::boundingRect(contours[j]);
                if(rect.x < rect2.x + rect2.width && rect.x + rect.width > rect2.x && rect.y < rect2.y + rect2.height && rect.y + rect.height > rect2.y){
                    if(rect.width * rect.height > rect2.width * rect2.height){
                        contours.erase(contours.begin() + j);
                    }else{
                        contours.erase(contours.begin() + i);
                    }
                }
            }
        }
    }
    
    std::vector<Plant> plants;

    for(int i = 0; i < contours.size(); i++){
        Plant plant;
        plant.boundingBox = cv::boundingRect(contours[i]);
        plant.plantImg = image(plant.boundingBox);
        plant.mask = binary(plant.boundingBox);
        plant.position = cv::Vec2d(plant.boundingBox.x, plant.boundingBox.y);
        plant.center = cv::Vec2d(plant.boundingBox.x + plant.boundingBox.width / 2, plant.boundingBox.y + plant.boundingBox.height / 2);

        plant.plantSpecies = cv::contourArea(contours[i]) < 500 ? species::advantis : species::wheat;
    }
    return plants;
}