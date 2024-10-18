//------------------------------------------------------------------------------
//
// File:        JetPositionChecker.hpp
// Description: Definition of JetPositionChecker 
//
//------------------------------------------------------------------------------
//
// File generated on Oct 2024 by Rin Baudelet
//------------------------------------------------------------------------------
#ifndef JET_POSITION_CHECKER_HPP
#define JET_POSITION_CHECKER_HPP

#include "Plant.hpp"
#include "LaserBehavior.hpp"
#include "LineDetector.hpp"
#include <opencv2/opencv.hpp>

namespace idl 
{
    class JetPositionChecker 
    {
    public:
        // Disallow copy
        JetPositionChecker(const JetPositionChecker&) = delete;
        JetPositionChecker& operator =(const JetPositionChecker&) = delete;

        // Disallow move
        JetPositionChecker(JetPositionChecker&&) noexcept = delete;
        JetPositionChecker& operator =(JetPositionChecker&&) noexcept = delete;

        ~JetPositionChecker() noexcept = default;

        /**
         * Create a JetPositionChecker from the line detector and plants.
         * @param iPlants a list of plants from the image
         * @param iLineDetector the image's line detector
         */
        JetPositionChecker(const std::vector<Plant>& iPlants,
            const LineDetector& iLineDetector);
    
        /**
         * Compute the laser behaviour's state. 
         * @return the current laser state according to its position. 
         */
        LaserBehavior computeState() const;

        /**
         * Retrieve the laser behavior according to a plant using its bounding box and its mask.
         * 
         * @param iPlant the plant to test with the laser
         * @param iJet the laser location point
         * 
         * @return the laser behavior relatively of the provided plant.
         */
        static LaserBehavior isOnPlant(const Plant& iPlant, const cv::Point& iJet); 
    private:
        const LineDetector& _lineDetector;
        const std::vector<Plant>& _plants;
    };
}

#endif // JET_POSITION_CHECKER_HPP
