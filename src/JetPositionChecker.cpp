#include "JetPositionChecker.hpp"

namespace idl
{
    LaserBehavior JetPositionChecker::isOnPlant(const Plant& iPlant, const cv::Point& iJet)
    {
        int tolerance = 0; // px
        auto boundingBox = iPlant.boundingBox;
        
        if (Species::advantis == iPlant.plantSpecies)
        {
            tolerance = 10; // px
            boundingBox.height += 10; // px
            boundingBox.width  += 10; // px

            boundingBox.x -= 5;
            boundingBox.y -= 5;
        }
        
        // Check first if the jet is in the boundary box
        if (iPlant.boundingBox.contains(iJet))
        {
            // Reposition mask using plant position
            int xPos = iPlant.position[0];
            int yPos = iPlant.position[1];

            // Reposition jet relatively to the mask
            cv::Point relativeJet = {iJet.x - xPos, iJet.y - yPos};

            bool isFound = iPlant.mask.at<bool>(relativeJet);
            
            for (int i = 0; i < tolerance && !isFound; i++)
            {
                for (int j = 0; j < tolerance && !isFound; j++)
                {
                    int dx = i ? 5 : tolerance/2 - i;
                    int dy = j ? 5 : tolerance/2 - j;
                    isFound = iPlant.mask.at<bool>(cv::Point{relativeJet.x+dx, relativeJet.y+dy});       
                }
            }

            if (isFound)
            {
                // Retrieve the laser behavior according to the targeted plant
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