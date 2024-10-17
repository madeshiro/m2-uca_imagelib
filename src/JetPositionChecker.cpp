#include "JetPositionChecker.hpp"

namespace idl
{
    LaserBehavior JetPositionChecker::isOnPlant(const Plant& iPlant, const cv::Point& iJet)
    {
        // Check first if the jet is in the boundary box
        if (iPlant.boundingBox.contains(iJet))
        {
            // Reposition mask using plant position
            int xPos = iPlant.position[0];
            int yPos = iPlant.position[1];

            // Reposition jet relatively to the mask
            cv::Point relativeJet = {iJet.x - xPos, iJet.y - yPos};

            if (iPlant.mask.at<bool>(relativeJet))
            {
                switch (iPlant.plantSpecies)
                {
                    case Species::wheat:
                        return LaserBehavior::onWheat;
                    case Species::advantis:
                        return LaserBehavior::onAdventis;
                    default:
                        return LaserBehavior::notDetected;
                }
            }
        }
        
        // The laser isn't on the plant
        return LaserBehavior::onNothing;
    }

    JetPositionChecker::JetPositionChecker(
        const std::vector<Plant>& iPlants,
        const LineDetector& iLineDetector
    ): _lineDetector(iLineDetector), _plants(iPlants)
    {
    }

    LaserBehavior JetPositionChecker::computeState() const 
    {
        if (!_lineDetector.hasIntersection())
        {
            return LaserBehavior::notDetected;
        }

        auto intersection = _lineDetector.getIntersection(); 

        for (const auto& plant : _plants)
        {
            auto state = isOnPlant(plant, intersection);
            if (LaserBehavior::onAdventis == state || LaserBehavior::onWheat == state)
            {
                return state;
            }
        }

        return LaserBehavior::onNothing;
    }
}