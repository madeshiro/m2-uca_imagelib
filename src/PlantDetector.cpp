#include "PlantDetector.hpp"
#include "Plant.hpp"
#include "Species.hpp"
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace cv;
using namespace std;

namespace idl
{
    /**
     * @brief This method remove a specific color from an image by using mask detection + grow, + removal and then inpainting
     * 
     * @param in image to be processed
     * @param min color min 
     * @param max color max
     * @param morph_size morph kernel size
     * @param inpaint_size inpainting kernel size
     * @return cv::Mat processed image with remvoved color
     */
    cv::Mat ElimColor(const cv::Mat& in, Scalar min, Scalar max, int morph_size = 5, int inpaint_size = 5)
    {
        Mat result;

        // Convert to HSV color space
        Mat hsv;
        cvtColor(in, hsv, COLOR_BGR2HSV);

        // Remove cyan (laser line) from the image
        inRange(hsv, min, max, result);

        // Thicken the mask
        Mat kernel = getStructuringElement(MORPH_RECT, Size(morph_size, morph_size));
        dilate(result, result, kernel);

        // Invert the mask
        Mat resn;
        bitwise_not(result, resn);

        // Apply the inverted mask to the image
        Mat masked;
        bitwise_and(in, in, masked, resn);

        // Repair the masked image by inpainting
        inpaint(masked, result, masked, inpaint_size, INPAINT_TELEA);

        return masked;
    }

    /**
     * @brief This method groups together a set of contours.
     * 
     * @param contours contour to be grouped
     * @param groupedContours grouped contours
     * @param maxDistance maximum distance between two contours
     */
    void groupContours(const vector<vector<Point>>& contours, vector<vector<Point>>& groupedContours, double maxDistance)
    {
        vector<bool> visited(contours.size(), false);

        for (size_t i = 0; i < contours.size(); ++i)
        {
            if (visited[i])
                continue;

            vector<Point> group = contours[i];
            visited[i] = true;

            Rect rect_i = boundingRect(contours[i]);

            for (size_t j = i + 1; j < contours.size(); ++j)
            {
                if (visited[j])
                    continue;

                Rect rect_j = boundingRect(contours[j]);

                // Check if contours are close or overlapping
                double distance = norm((rect_i.tl() + rect_i.br()) * 0.5 - (rect_j.tl() + rect_j.br()) * 0.5);

                if ((rect_i & rect_j).area() > 0 || distance < maxDistance)
                {
                    group.insert(group.end(), contours[j].begin(), contours[j].end());
                    visited[j] = true;
                }
            }
            groupedContours.push_back(group);
        }
    }

   /// -------------------------------These are just the structures to hold the filter settings--------------------------------
    struct AdvantisParams
    {
        int inRangeMinH;
        int inRangeMinS;
        int inRangeMinV;
        int inRangeMaxH;
        int inRangeMaxS;
        int inRangeMaxV;
        int morphOpenSize;
        int dilateIterations;
        double areaThreshold;
        double groupMaxDistance;
    };

    struct WheatParams
    {
        int min_L;
        int min_a;
        int min_b;
        int max_L;
        int max_a;
        int max_b;
        int morphKernelSize;
        int morphIterations;
        double areaThreshold;
        double aspectRatioMin;
        double aspectRatioMax;
        double groupMaxDistance;
    };
    //----------------------------------------------------------------------------------------------------

    /**
     * @brief it does what the name says
     * 
     * @param masked The input raw mask
     * @param params The detection parameters
     * @return cv::Mat detected advants mask
     */
    cv::Mat detectAdvantis(const cv::Mat& masked, const AdvantisParams& params)
    {
        cv::Mat ranged_advantis;
        cv::inRange(masked,
                    cv::Scalar(params.inRangeMinH, params.inRangeMinS, params.inRangeMinV),
                    cv::Scalar(params.inRangeMaxH, params.inRangeMaxS, params.inRangeMaxV),
                    ranged_advantis);

        cv::Mat binary_advantis;
        cv::threshold(ranged_advantis, binary_advantis, 0, 255, cv::THRESH_BINARY);

        cv::Mat kernel_advantis = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(params.morphOpenSize, params.morphOpenSize));
        cv::morphologyEx(binary_advantis, binary_advantis, cv::MORPH_OPEN, kernel_advantis);

        // Grow the blobs
        cv::dilate(binary_advantis, binary_advantis, kernel_advantis, cv::Point(-1, -1), params.dilateIterations);

        std::vector<std::vector<cv::Point>> contours_advantis;
        cv::findContours(binary_advantis, contours_advantis, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // **Filter Advantis Contours**
        std::vector<std::vector<cv::Point>> filteredContours_advantis;
        for (const auto& contour : contours_advantis)
        {
            double area = cv::contourArea(contour);

            // Ignore tiny contours (noise)
            if (area < params.areaThreshold)
                continue;

            filteredContours_advantis.push_back(contour);
        }

        // **Group Advantis Contours to Form Individual Plants**
        std::vector<std::vector<cv::Point>> groupedContours_advantis;
        groupContours(filteredContours_advantis, groupedContours_advantis, params.groupMaxDistance);

        // Create a new mask to include only the grouped advantis contours
        cv::Mat cleanedMask_advantis = cv::Mat::zeros(binary_advantis.size(), CV_8UC1);

        // Draw the grouped advantis contours onto the new mask
        for (const auto& group : groupedContours_advantis)
        {
            cv::drawContours(cleanedMask_advantis, std::vector<std::vector<cv::Point>>{group}, -1, cv::Scalar(255), cv::FILLED);
        }

        return cleanedMask_advantis;
    }

    /**
     * @brief it's in the name come on
     * 
     * @param masked The input raw mask
     * @param params The detection parameters
     * @return cv::Mat detected wheat mask
     */
    cv::Mat detectWheat(const cv::Mat& masked, const WheatParams& params)
    {
        // Convert to Lab color space for better color segmentation
        cv::Mat lab;
        cv::cvtColor(masked, lab, cv::COLOR_BGR2Lab);

        // Split the Lab image into channels
        std::vector<cv::Mat> lab_channels;
        cv::split(lab, lab_channels);

        // Apply histogram equalization on the L channel
        cv::equalizeHist(lab_channels[0], lab_channels[0]);

        // Merge the channels back
        cv::merge(lab_channels, lab);

        // Threshold to get wheat plants using specified values
        cv::Mat plantMask_wheat;
        cv::inRange(lab,
                    cv::Scalar(params.min_L, params.min_a, params.min_b),
                    cv::Scalar(params.max_L, params.max_a, params.max_b),
                    plantMask_wheat);

        // Invert the mask since leaves are black regions
        cv::bitwise_not(plantMask_wheat, plantMask_wheat);

        // Adjust morphological operations to remove noise and fill holes
        int morph_kernel_size = params.morphKernelSize;
        int morph_iterations = params.morphIterations;

        // Ensure kernel size is odd and at least 1
        if (morph_kernel_size % 2 == 0) morph_kernel_size += 1;
        if (morph_kernel_size < 1) morph_kernel_size = 1;

        cv::Mat kernel_wheat = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(morph_kernel_size, morph_kernel_size));

        // Apply morphological opening to remove small noise
        cv::morphologyEx(plantMask_wheat, plantMask_wheat, cv::MORPH_OPEN, kernel_wheat, cv::Point(-1, -1), morph_iterations);

        // Apply morphological closing to fill small holes in the leaves
        cv::morphologyEx(plantMask_wheat, plantMask_wheat, cv::MORPH_CLOSE, kernel_wheat, cv::Point(-1, -1), morph_iterations);

        // Find contours on the cleaned mask
        std::vector<std::vector<cv::Point>> contours_wheat;
        cv::findContours(plantMask_wheat, contours_wheat, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Filter contours based on properties
        std::vector<std::vector<cv::Point>> filteredContours_wheat;

        for (const auto& contour : contours_wheat)
        {
            // Compute contour area
            double area = cv::contourArea(contour);

            // Ignore tiny contours (noise)
            if (area < params.areaThreshold)
                continue;

            // Compute bounding rectangle
            cv::Rect rect = cv::boundingRect(contour);

            // Ignore contours that are too elongated (unlikely to be leaves)
            double aspectRatio = static_cast<double>(rect.width) / rect.height;
            if (aspectRatio > params.aspectRatioMax || aspectRatio < params.aspectRatioMin)
                continue;

            // If all checks pass, add the contour to the filtered list
            filteredContours_wheat.push_back(contour);
        }

        // **Group Wheat Contours to Form Individual Plants**
        std::vector<std::vector<cv::Point>> groupedContours_wheat;
        groupContours(filteredContours_wheat, groupedContours_wheat, params.groupMaxDistance);

        // Create a new mask to include only the grouped wheat contours
        cv::Mat cleanedMask_wheat = cv::Mat::zeros(plantMask_wheat.size(), CV_8UC1);

        // Draw the grouped contours onto the new mask
        for (const auto& group : groupedContours_wheat)
        {
            cv::drawContours(cleanedMask_wheat, std::vector<std::vector<cv::Point>>{group}, -1, cv::Scalar(255), cv::FILLED);
        }

        return cleanedMask_wheat;
    }

    /**
     * @brief Detects plants from a combined wheat+advantis mask using a smart scoring system
     * 
     * @param combinedMask 
     * @param image 
     * @param wheatScoreThreshold 
     * @return std::vector<Plant> 
     */
    std::vector<Plant> processCombinedMask(const cv::Mat& combinedMask, const cv::Mat& image, double wheatScoreThreshold)
    {
        std::vector<std::vector<cv::Point>> contours_combined;
        cv::findContours(combinedMask, contours_combined, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // **Prepare Plant Objects**
        std::vector<Plant> plants;

        double centerLineX = image.cols / 2.0;
        double centerLineThreshold = image.cols * 0.3; // Adjust as needed

        // Image diagonal length
        double imageDiagonal = std::sqrt(image.cols * image.cols + image.rows * image.rows);
        double proximityThreshold = 0.04 * imageDiagonal; // 4% of image diagonal

        // Temporary vector to store advantis plant centers
        std::vector<cv::Point2f> advantisCenters;

        // First pass: classify plants and store advantis centers
        for (const auto& contour : contours_combined)
        {
            Plant plant;

            // Compute bounding box
            plant.boundingBox = cv::boundingRect(contour);

            // Ensure bounding box is within image boundaries
            plant.boundingBox &= cv::Rect(0, 0, image.cols, image.rows);

            // Extract plant image and mask
            plant.plantImg = image(plant.boundingBox);
            plant.mask = combinedMask(plant.boundingBox);

            // Compute center using moments for the contour
            cv::Moments m = cv::moments(contour);
            cv::Point2f center;
            if (m.m00 != 0)
            {
                center = cv::Point2f(static_cast<float>(m.m10 / m.m00), static_cast<float>(m.m01 / m.m00));
                plant.center = cv::Vec2d(center.x, center.y);
            }
            else
            {
                center = cv::Point2f(plant.boundingBox.x + plant.boundingBox.width / 2.0f,
                                     plant.boundingBox.y + plant.boundingBox.height / 2.0f);
                plant.center = cv::Vec2d(center.x, center.y);
            }

            plant.position = cv::Vec2d(static_cast<double>(plant.boundingBox.x),
                                       static_cast<double>(plant.boundingBox.y));

            // **Compute Features for Intelligent Scoring**

            // Compute contour area
            double area = cv::contourArea(contour);

            // Compute convex hull and solidity
            std::vector<cv::Point> hull;
            cv::convexHull(contour, hull);
            double hullArea = cv::contourArea(hull);
            double solidity = area / hullArea;

            // Compute aspect ratio
            double aspectRatio = static_cast<double>(plant.boundingBox.width) / plant.boundingBox.height;

            // Compute extent (area / bounding box area)
            double boundingBoxArea = plant.boundingBox.width * plant.boundingBox.height;
            double extent = area / boundingBoxArea;

            // Compute distance from center line
            double distanceFromCenter = std::abs(center.x - centerLineX);

            // score related stuff down there
            double score = 0.0;

            // Size Feature
            if (area > 500.0)
                score += area / 500;

            // Solidity Feature (wheat leaves may have higher solidity due to heart shape)
            if (solidity > 0.8)
                score += solidity;

            // Aspect Ratio Feature //removed because unreliable
            //if (aspectRatio > 0.5 && aspectRatio < 2.0)
            //    score += 1.0;

            // Extent Feature
            if (extent > 0.5)
                score += 1.0;

            // Position Feature (wheat along center line)
            if (distanceFromCenter < centerLineThreshold)
                score += 1.0f / distanceFromCenter;

            if (score >= wheatScoreThreshold)
            {
                plant.plantSpecies = Species::wheat;
            }
            else
            {
                plant.plantSpecies = Species::advantis;
                // Store advantis center
                advantisCenters.push_back(center);
            }

            // Store area and score for later use
            plant.area = area;
            plant.score = score;

            plants.push_back(plant);
        }

        // **Second pass: Reclassify small plants near advantis as advantis because they often are (de-clustering)**
        for (auto& plant : plants)
        {
            if (plant.plantSpecies == Species::wheat)
            {
                // Check if plant is small (obviously wheat is a lot larger)
                if (plant.area < 3000.0)
                {
                    // Check if score is near the advantis threshold
                    if (plant.score < wheatScoreThreshold + 1.0)
                    {
                        // Check proximity to any advantis plant
                        for (const auto& advCenter : advantisCenters)
                        {
                            double distance = cv::norm(cv::Point2f(plant.center[0], plant.center[1]) - advCenter);
                            if (distance < proximityThreshold)
                            {
                                // Reclassify as advantis
                                plant.plantSpecies = Species::advantis;
                                break;
                            }
                        }
                    }
                }
            }
        }

        return plants;
    }
    /**
     * @brief This is the main class function to detect the various plants in the image and classify their species (advantis/wheat)
     * 
     * @param img the input image, duh
     * @param enableSliders wether or not you want the debug filtering sliders to appear, useful to fiddle with the values in real time
     * @return std::vector<Plant> 
     */
    std::vector<Plant> PlantDetector::detectPlants(const cv::Mat& img, bool enableSliders)
    {
        const double wheatScoreThreshold = 4.0;
        cv::Mat image = img.clone();

        // Remove the laser line
        cv::Mat masked = ElimColor(image, cv::Scalar(80, 80, 80), cv::Scalar(100, 255, 255), 5, 5);

        // Initialize default parameters
        AdvantisParams advantisParams = {95, 0, 0, 179, 117, 105, 2, 2, 50.0, 30.0};
        WheatParams wheatParams = {0, 82, 123, 240, 131, 134, 3, 2, 500.0, 0.2, 5.0, 50.0};

        if (enableSliders)
        {
            // Create windows
            cv::namedWindow("Advantis Mask", cv::WINDOW_NORMAL);
            cv::namedWindow("Wheat Mask", cv::WINDOW_NORMAL);
            cv::namedWindow("Combined Mask", cv::WINDOW_NORMAL);
            cv::namedWindow("Result", cv::WINDOW_NORMAL);

            // Create trackbars for advantis parameters
            cv::createTrackbar("Adv Min H", "Advantis Mask", &advantisParams.inRangeMinH, 179);
            cv::createTrackbar("Adv Min S", "Advantis Mask", &advantisParams.inRangeMinS, 255);
            cv::createTrackbar("Adv Min V", "Advantis Mask", &advantisParams.inRangeMinV, 255);
            cv::createTrackbar("Adv Max H", "Advantis Mask", &advantisParams.inRangeMaxH, 179);
            cv::createTrackbar("Adv Max S", "Advantis Mask", &advantisParams.inRangeMaxS, 255);
            cv::createTrackbar("Adv Max V", "Advantis Mask", &advantisParams.inRangeMaxV, 255);
            cv::createTrackbar("Adv Morph Open Size", "Advantis Mask", &advantisParams.morphOpenSize, 20);
            cv::createTrackbar("Adv Dilate Iterations", "Advantis Mask", &advantisParams.dilateIterations, 10);
            cv::createTrackbar("Adv Area Threshold", "Advantis Mask", (int*)&advantisParams.areaThreshold, 1000);
            cv::createTrackbar("Adv Group Max Distance", "Advantis Mask", (int*)&advantisParams.groupMaxDistance, 100);

            // Create trackbars for wheat parameters
            cv::createTrackbar("Wheat Min L", "Wheat Mask", &wheatParams.min_L, 255);
            cv::createTrackbar("Wheat Min a", "Wheat Mask", &wheatParams.min_a, 255);
            cv::createTrackbar("Wheat Min b", "Wheat Mask", &wheatParams.min_b, 255);
            cv::createTrackbar("Wheat Max L", "Wheat Mask", &wheatParams.max_L, 255);
            cv::createTrackbar("Wheat Max a", "Wheat Mask", &wheatParams.max_a, 255);
            cv::createTrackbar("Wheat Max b", "Wheat Mask", &wheatParams.max_b, 255);
            cv::createTrackbar("Wheat Morph Kernel Size", "Wheat Mask", &wheatParams.morphKernelSize, 20);
            cv::createTrackbar("Wheat Morph Iterations", "Wheat Mask", &wheatParams.morphIterations, 10);
            cv::createTrackbar("Wheat Area Threshold", "Wheat Mask", (int*)&wheatParams.areaThreshold, 10000);
            cv::createTrackbar("Wheat Aspect Ratio Min x100", "Wheat Mask", (int*)&wheatParams.aspectRatioMin, 500);
            cv::createTrackbar("Wheat Aspect Ratio Max x100", "Wheat Mask", (int*)&wheatParams.aspectRatioMax, 500);
            cv::createTrackbar("Wheat Group Max Distance", "Wheat Mask", (int*)&wheatParams.groupMaxDistance, 100);

            while (true)
            {
                //most of this stuff is self explanatory
                wheatParams.aspectRatioMin = cv::getTrackbarPos("Wheat Aspect Ratio Min x100", "Wheat Mask") / 100.0;
                wheatParams.aspectRatioMax = cv::getTrackbarPos("Wheat Aspect Ratio Max x100", "Wheat Mask") / 100.0;

                cv::Mat cleanedMask_advantis = detectAdvantis(masked, advantisParams);
                cv::Mat cleanedMask_wheat = detectWheat(masked, wheatParams);

                cv::Mat combinedMask;
                cv::bitwise_or(cleanedMask_advantis, cleanedMask_wheat, combinedMask);

                cv::imshow("Advantis Mask", cleanedMask_advantis);
                cv::imshow("Wheat Mask", cleanedMask_wheat);
                cv::imshow("Combined Mask", combinedMask);

                std::vector<Plant> plants = processCombinedMask(combinedMask, image, wheatScoreThreshold);

                cv::Mat resultImage = image.clone();
                for (const auto& plant : plants)
                {
                    cv::rectangle(resultImage, plant.boundingBox, (plant.plantSpecies == Species::wheat) ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255), 2);
                }
                cv::imshow("Result", resultImage);

                // n/q to progress through the images
                char key = (char)cv::waitKey(1);
                if (key == 'n' || key == 'q' || key == 27)
                {
                    break;
                }
            }
        }
        else
        {
            cv::Mat cleanedMask_advantis = detectAdvantis(masked, advantisParams);
            cv::Mat cleanedMask_wheat = detectWheat(masked, wheatParams);

            cv::Mat combinedMask;
            cv::bitwise_or(cleanedMask_advantis, cleanedMask_wheat, combinedMask);

            std::vector<Plant> plants = processCombinedMask(combinedMask, image, wheatScoreThreshold);

            return plants;
        }

        // Return an empty vector if sliders were enabled (as processing is interactive)
        return std::vector<Plant>();
    }
}