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
     * @brief This method removes a specific color from an image by using mask detection, growth, removal, and then inpainting.
     * 
     * @param in image to be processed
     * @param min color min 
     * @param max color max
     * @param morph_size morph kernel size
     * @param inpaint_size inpainting kernel size
     * @return cv::Mat processed image with removed color
     */
    cv::Mat ElimColor(const cv::Mat& in, Scalar min, Scalar max, int morph_size = 5, int inpaint_size = 5)
    {
        Mat result;

        // Convert to HSV color space
        Mat hsv;
        cvtColor(in, hsv, COLOR_BGR2HSV);

        // Remove specified color from the image
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
     * @brief Detect advantis plants in the image without grouping contours.
     * 
     * @param masked The input image with the laser line removed
     * @param params The detection parameters
     * @return cv::Mat Detected advantis mask
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

        return binary_advantis;
    }

    /**
     * @brief Detect wheat plants in the image without grouping contours.
     * 
     * @param masked The input image with the laser line removed
     * @param params The detection parameters
     * @return cv::Mat Detected wheat mask
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

        return plantMask_wheat;
    }

    /**
     * @brief Struct to hold contour information along with computed features and species classification.
     */
    struct ContourInfo
    {
        std::vector<cv::Point> contour;
        Species plantSpecies;
        double area;
        double score;
        cv::Rect boundingBox;
        cv::Point2f center;
    };

    /**
     * @brief Struct to hold circle information for debugging purposes.
     */
    struct Circle
    {
        cv::Point2f center;
        float radius;
    };

    /**
     * @brief Detects plants from a combined wheat+advantis mask using a smart scoring system and performs species-aware grouping.
     *        Additionally, removes advantis plants near the centers of wheat plants.
     * 
     * @param combinedMask The combined mask of wheat and advantis plants
     * @param image The original image
     * @param edgeMask The edge mask for filtering
     * @param wheatScoreThreshold The threshold for classifying wheat
     * @param wheatCircles Vector to store circles around wheat centers for debugging
     * @return std::vector<Plant> The detected and grouped plants
     */
    std::vector<Plant> processCombinedMask(const cv::Mat& combinedMask, const cv::Mat& image, const cv::Mat& edgeMask, double wheatScoreThreshold, std::vector<Circle>& wheatCircles)
    {
        std::vector<std::vector<cv::Point>> contours_combined;
        cv::findContours(combinedMask, contours_combined, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // **Prepare ContourInfo Objects**
        std::vector<ContourInfo> contourInfos;

        double centerLineX = image.cols / 2.0;
        double centerLineThreshold = image.cols * 0.3; // Adjust as needed

        // Image diagonal length
        double imageDiagonal = std::sqrt(image.cols * image.cols + image.rows * image.rows);
        double proximityThreshold = 0.04 * imageDiagonal; // 4% of image diagonal

        // First pass: classify plants and store advantis centers
        std::vector<cv::Point2f> advantisCenters;

        for (const auto& contour : contours_combined)
        {
            ContourInfo info;
            info.contour = contour;

            // Compute bounding box
            info.boundingBox = cv::boundingRect(contour);

            // Compute center using moments for the contour
            cv::Moments m = cv::moments(contour);
            cv::Point2f center;
            if (m.m00 != 0)
            {
                center = cv::Point2f(static_cast<float>(m.m10 / m.m00), static_cast<float>(m.m01 / m.m00));
            }
            else
            {
                center = cv::Point2f(info.boundingBox.x + info.boundingBox.width / 2.0f,
                                     info.boundingBox.y + info.boundingBox.height / 2.0f);
            }
            info.center = center;

            // **Compute Features for Intelligent Scoring**

            // Compute contour area
            double area = cv::contourArea(contour);
            info.area = area;

            // Compute convex hull and solidity
            std::vector<cv::Point> hull;
            cv::convexHull(contour, hull);
            double hullArea = cv::contourArea(hull);
            double solidity = area / hullArea;

            // Compute aspect ratio
            double aspectRatio = static_cast<double>(info.boundingBox.width) / info.boundingBox.height;

            // Compute extent (area / bounding box area)
            double boundingBoxArea = info.boundingBox.width * info.boundingBox.height;
            double extent = area / boundingBoxArea;

            // Compute distance from center line
            double distanceFromCenter = std::abs(center.x - centerLineX);

            // Score calculation
            double score = 0.0;

            // Size Feature
            //if (area > 400.0)
                score += area / 400;

            // Solidity Feature (wheat leaves may have higher solidity due to heart shape)
            if (solidity > 0.8)
                score += solidity;

            // Extent Feature
            if (extent > 0.5)
                score += 1.0;

            // Position Feature (wheat along center line)
            if (distanceFromCenter < centerLineThreshold && distanceFromCenter > 0)
                score += 1.0f / distanceFromCenter;
            else if (distanceFromCenter == 0)
                score += 1.0f / (distanceFromCenter + 1); // Avoid division by zero

            info.score = score;

            if (score >= wheatScoreThreshold)
            {
                info.plantSpecies = Species::wheat;
            }
            else
            {
                info.plantSpecies = Species::advantis;
                // Store advantis center
                advantisCenters.push_back(center);
            }

            contourInfos.push_back(info);
        }

        // **Second pass: Reclassify small plants near advantis as advantis**
        for (auto& info : contourInfos)
        {
            if (info.plantSpecies == Species::wheat)
            {
                // Check if plant is small
                if (info.area < 3000.0)
                {
                    // Check if score is near the advantis threshold
                    if (info.score < wheatScoreThreshold + 1.0)
                    {
                        // Check proximity to any advantis plant
                        for (const auto& advCenter : advantisCenters)
                        {
                            double distance = cv::norm(info.center - advCenter);
                            if (distance < proximityThreshold)
                            {
                                // Reclassify as advantis
                                info.plantSpecies = Species::advantis;
                                break;
                            }
                        }
                    }
                }
            }
        }

        // **Group Contours by Species**
        std::vector<std::vector<cv::Point>> wheatContours;
        std::vector<std::vector<cv::Point>> advantisContours;

        for (const auto& info : contourInfos)
        {
            if (info.plantSpecies == Species::wheat)
            {
                wheatContours.push_back(info.contour);
            }
            else if (info.plantSpecies == Species::advantis)
            {
                advantisContours.push_back(info.contour);
            }
        }

        // **Perform Species-Aware Grouping**
        std::vector<std::vector<cv::Point>> groupedWheatContours;
        std::vector<std::vector<cv::Point>> groupedAdvantisContours;

        double wheatGroupMaxDistance = 50.0;    // Adjust as needed
        double advantisGroupMaxDistance = 30.0; // Adjust as needed

        groupContours(wheatContours, groupedWheatContours, wheatGroupMaxDistance);
        groupContours(advantisContours, groupedAdvantisContours, advantisGroupMaxDistance);

        // **Create Plant Objects from Grouped Contours**
        std::vector<Plant> plants;

        // Process grouped wheat contours
        for (const auto& contourGroup : groupedWheatContours)
        {
            Plant plant;

            // Compute bounding box
            cv::Rect boundingBox = cv::boundingRect(contourGroup);

            // Ensure bounding box is within image boundaries
            boundingBox &= cv::Rect(0, 0, image.cols, image.rows);

            plant.boundingBox = boundingBox;

            // Extract plant image and mask
            plant.plantImg = image(boundingBox);

            // Create mask
            cv::Mat plantMask = cv::Mat::zeros(boundingBox.size(), CV_8UC1);

            // Shift contour points to ROI coordinates
            std::vector<std::vector<cv::Point>> shiftedContours(1);
            for (const auto& pt : contourGroup)
            {
                shiftedContours[0].push_back(cv::Point(pt.x - boundingBox.x, pt.y - boundingBox.y));
            }

            // Draw contour onto mask
            cv::drawContours(plantMask, shiftedContours, -1, cv::Scalar(255), cv::FILLED);

            plant.mask = plantMask;

            // Compute center
            cv::Moments m = cv::moments(contourGroup);
            cv::Point2f center;
            if (m.m00 != 0)
            {
                center = cv::Point2f(static_cast<float>(m.m10 / m.m00), static_cast<float>(m.m01 / m.m00));
                plant.center = cv::Vec2d(center.x, center.y);
            }
            else
            {
                center = cv::Point2f(boundingBox.x + boundingBox.width / 2.0f,
                                     boundingBox.y + boundingBox.height / 2.0f);
                plant.center = cv::Vec2d(center.x, center.y);
            }

            plant.position = cv::Vec2d(static_cast<double>(boundingBox.x),
                                       static_cast<double>(boundingBox.y));

            plant.plantSpecies = Species::wheat;

            // Compute area (optional)
            plant.area = cv::contourArea(contourGroup);

            plants.push_back(plant);
        }

        // Process grouped advantis contours
        for (const auto& contourGroup : groupedAdvantisContours)
        {
            Plant plant;

            // Compute bounding box
            cv::Rect boundingBox = cv::boundingRect(contourGroup);

            // Ensure bounding box is within image boundaries
            boundingBox &= cv::Rect(0, 0, image.cols, image.rows);

            plant.boundingBox = boundingBox;

            // Extract plant image and mask
            plant.plantImg = image(boundingBox);

            // Create mask
            cv::Mat plantMask = cv::Mat::zeros(boundingBox.size(), CV_8UC1);

            // Shift contour points to ROI coordinates
            std::vector<std::vector<cv::Point>> shiftedContours(1);
            for (const auto& pt : contourGroup)
            {
                shiftedContours[0].push_back(cv::Point(pt.x - boundingBox.x, pt.y - boundingBox.y));
            }

            // Draw contour onto mask
            cv::drawContours(plantMask, shiftedContours, -1, cv::Scalar(255), cv::FILLED);

            plant.mask = plantMask;

            // Compute center
            cv::Moments m = cv::moments(contourGroup);
            cv::Point2f center;
            if (m.m00 != 0)
            {
                center = cv::Point2f(static_cast<float>(m.m10 / m.m00), static_cast<float>(m.m01 / m.m00));
                plant.center = cv::Vec2d(center.x, center.y);
            }
            else
            {
                center = cv::Point2f(boundingBox.x + boundingBox.width / 2.0f,
                                     boundingBox.y + boundingBox.height / 2.0f);
                plant.center = cv::Vec2d(center.x, center.y);
            }

            plant.position = cv::Vec2d(static_cast<double>(boundingBox.x),
                                       static_cast<double>(boundingBox.y));

            plant.plantSpecies = Species::advantis;

            // Compute area (optional)
            plant.area = cv::contourArea(contourGroup);

            plants.push_back(plant);
        }

        // **Separate plants into wheat and advantis plants**
        std::vector<Plant> wheatPlants;
        std::vector<Plant> advantisPlants;

        for (const auto& plant : plants)
        {
            if (plant.plantSpecies == Species::wheat)
            {
                wheatPlants.push_back(plant);
            }
            else if (plant.plantSpecies == Species::advantis)
            {
                advantisPlants.push_back(plant);
            }
        }

        // **Third pass: Remove advantis plants near the centers of wheat plants**
        std::vector<bool> advantisPlantToRemove(advantisPlants.size(), false);

        for (const auto& wheatPlant : wheatPlants)
        {
            cv::Point2f wheatCenter(wheatPlant.center[0], wheatPlant.center[1]);

            // Diameter is a third of the wheat plant's bounding box width
            float diameter = wheatPlant.boundingBox.width / 3.0f;
            float radius = diameter / 2.0f;

            // Store the circle for debugging
            wheatCircles.push_back({wheatCenter, radius});

            for (size_t i = 0; i < advantisPlants.size(); ++i)
            {
                const Plant& advantisPlant = advantisPlants[i];

                cv::Point2f advantisCenter(advantisPlant.center[0], advantisPlant.center[1]);

                // Compute distance between centers
                float distance = cv::norm(wheatCenter - advantisCenter);

                if (distance <= radius)
                {
                    // Mark advantisPlant for removal
                    advantisPlantToRemove[i] = true;
                }
            }
        }

        // Remove marked advantis plants
        std::vector<Plant> filteredAdvantisPlants;
        for (size_t i = 0; i < advantisPlants.size(); ++i)
        {
            if (!advantisPlantToRemove[i])
            {
                filteredAdvantisPlants.push_back(advantisPlants[i]);
            }
        }

        // Combine wheat and filtered advantis plants
        plants.clear();
        plants.insert(plants.end(), wheatPlants.begin(), wheatPlants.end());
        plants.insert(plants.end(), filteredAdvantisPlants.begin(), filteredAdvantisPlants.end());

        // **Final pass: Remove plants whose bounding boxes do not overlap with the edge mask**
        std::vector<Plant> finalPlants;
        for (const auto& plant : plants)
        {
            // Extract the edge mask region corresponding to the plant's bounding box
            cv::Rect bbox = plant.boundingBox & cv::Rect(0, 0, edgeMask.cols, edgeMask.rows);
            cv::Mat edgeRegion = edgeMask(bbox);

            // Check if there are any positive pixels in the edge region, also discard plants that are too big
            int nonZeroCount = cv::countNonZero(edgeRegion);
            if (nonZeroCount > 0 && plant.area < 100000)
            {
                // Keep the plant
                finalPlants.push_back(plant);
            }
            else
            {
                // Discard the plant (it does not overlap with any edges)
            }
        }

        return finalPlants;
    }

    /**
     * @brief This is the main class function to detect the various plants in the image and classify their species (advantis/wheat)
     * 
     * @param img the input image
     * @param enableSliders whether or not you want the debug filtering sliders to appear, useful to fiddle with the values in real time
     * @return std::vector<Plant> The detected plants
     */
    std::vector<Plant> PlantDetector::detectPlants(const cv::Mat& img, bool enableSliders)
    {
        const double wheatScoreThreshold = 4.0;
        cv::Mat image = img.clone();

        // Remove the laser line
        cv::Mat masked = ElimColor(image, cv::Scalar(80, 80, 80), cv::Scalar(100, 255, 255), 5, 5);

        // Initialize default parameters
        AdvantisParams advantisParams = {96, 0, 0, 179, 253, 109, 2, 2, 50.0, 0.0};
        WheatParams wheatParams = {0, 82, 123, 240, 131, 134, 3, 2, 500.0, 0.2, 5.0, 50.0};

        // Edge detection parameters
        int lowThreshold = 22;
        int highThreshold = 64;
        int edgeDilateSize = 13;
        int edgeErodeSize = 16; // Erosion size parameter

        if (enableSliders)
        {
            // Create windows
            cv::namedWindow("Advantis Mask", cv::WINDOW_NORMAL);
            cv::namedWindow("Wheat Mask", cv::WINDOW_NORMAL);
            cv::namedWindow("Combined Mask", cv::WINDOW_NORMAL);
            cv::namedWindow("Result", cv::WINDOW_NORMAL);
            cv::namedWindow("Edge Mask", cv::WINDOW_NORMAL);

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

            // Create trackbars for edge detection parameters
            cv::createTrackbar("Edge Low Threshold", "Edge Mask", &lowThreshold, 255);
            cv::createTrackbar("Edge High Threshold", "Edge Mask", &highThreshold, 255);
            cv::createTrackbar("Edge Dilate Size", "Edge Mask", &edgeDilateSize, 20);
            cv::createTrackbar("Edge Erode Size", "Edge Mask", &edgeErodeSize, 20); // Erosion slider

            while (true)
            {
                // Update wheat aspect ratio parameters from trackbars
                wheatParams.aspectRatioMin = cv::getTrackbarPos("Wheat Aspect Ratio Min x100", "Wheat Mask") / 100.0;
                wheatParams.aspectRatioMax = cv::getTrackbarPos("Wheat Aspect Ratio Max x100", "Wheat Mask") / 100.0;

                // Update edge detection parameters from trackbars
                lowThreshold = cv::getTrackbarPos("Edge Low Threshold", "Edge Mask");
                highThreshold = cv::getTrackbarPos("Edge High Threshold", "Edge Mask");
                edgeDilateSize = cv::getTrackbarPos("Edge Dilate Size", "Edge Mask");
                if (edgeDilateSize < 1) edgeDilateSize = 1; // Ensure it's at least 1

                edgeErodeSize = cv::getTrackbarPos("Edge Erode Size", "Edge Mask");
                if (edgeErodeSize < 1) edgeErodeSize = 1; // Ensure it's at least 1

                // Perform edge detection
                cv::Mat grayMasked;
                cv::cvtColor(masked, grayMasked, cv::COLOR_BGR2GRAY);
                cv::Mat edges;
                cv::Canny(grayMasked, edges, lowThreshold, highThreshold);

                // Dilate the edges
                cv::Mat edgeKernelDilate = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(edgeDilateSize, edgeDilateSize));
                cv::dilate(edges, edges, edgeKernelDilate);

                // Erode the edges
                cv::Mat edgeKernelErode = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(edgeErodeSize, edgeErodeSize));
                cv::erode(edges, edges, edgeKernelErode);

                cv::imshow("Edge Mask", edges);

                // Detect advantis and wheat plants
                cv::Mat cleanedMask_advantis = detectAdvantis(masked, advantisParams);
                cv::Mat cleanedMask_wheat = detectWheat(masked, wheatParams);

                cv::Mat combinedMask;
                cv::bitwise_or(cleanedMask_advantis, cleanedMask_wheat, combinedMask);

                cv::imshow("Advantis Mask", cleanedMask_advantis);
                cv::imshow("Wheat Mask", cleanedMask_wheat);
                cv::imshow("Combined Mask", combinedMask);

                // Prepare vector to hold wheat circles for debugging
                std::vector<Circle> wheatCircles;

                std::vector<Plant> plants = processCombinedMask(combinedMask, image, edges, wheatScoreThreshold, wheatCircles);

                cv::Mat resultImage = image.clone();
                for (const auto& plant : plants)
                {
                    cv::rectangle(resultImage, plant.boundingBox, (plant.plantSpecies == Species::wheat) ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255), 2);
                }

                // Draw the wheat center circles for debugging
                for (const auto& circle : wheatCircles)
                {
                    cv::circle(resultImage, circle.center, static_cast<int>(circle.radius), cv::Scalar(255, 0, 0), 2);
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
            // Perform edge detection without sliders
            cv::Mat grayMasked;
            cv::cvtColor(masked, grayMasked, cv::COLOR_BGR2GRAY);
            cv::Mat edges;
            cv::Canny(grayMasked, edges, lowThreshold, highThreshold);

            // Dilate the edges
            cv::Mat edgeKernelDilate = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(edgeDilateSize, edgeDilateSize));
            cv::dilate(edges, edges, edgeKernelDilate);

            // Erode the edges
            cv::Mat edgeKernelErode = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(edgeErodeSize, edgeErodeSize));
            cv::erode(edges, edges, edgeKernelErode);

            // Detect advantis and wheat plants
            cv::Mat cleanedMask_advantis = detectAdvantis(masked, advantisParams);
            cv::Mat cleanedMask_wheat = detectWheat(masked, wheatParams);

            cv::Mat combinedMask;
            cv::bitwise_or(cleanedMask_advantis, cleanedMask_wheat, combinedMask);

            // Prepare vector to hold wheat circles (not used in non-interactive mode)
            std::vector<Circle> wheatCircles;

            std::vector<Plant> plants = processCombinedMask(combinedMask, image, edges, wheatScoreThreshold, wheatCircles);

            return plants;
        }

        // Return an empty vector if sliders were enabled (as processing is interactive)
        return std::vector<Plant>();
    }
}