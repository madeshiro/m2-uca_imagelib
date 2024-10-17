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
    
        LaserBehavior computeState() const;

        static LaserBehavior isOnPlant(const Plant& p, const cv::Point& jet); 
    private:
        const LineDetector& _lineDetector;
        std::vector<Plant> _plants;
    };
}

#endif // JET_POSITION_CHECKER_HPP
