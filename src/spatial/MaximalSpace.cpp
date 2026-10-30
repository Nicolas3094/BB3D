#include "MaximalSpace.hpp"
#include "../util/ExecutionCheckpoint.hpp"
#include <algorithm>
#include <tuple>
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
#include "MaximalSpacesProfile.hpp"
#endif

bool hasPositiveDimensions(const MaximalSpace &space)
{
    return space.minimum.x < space.maximum.x && space.minimum.y < space.maximum.y &&
           space.minimum.z < space.maximum.z;
}

bool spacesIntersect(const MaximalSpace &first, const MaximalSpace &second)
{
    // Face or edge contact leaves no occupied volume inside the other space.
    return first.minimum.x < second.maximum.x && second.minimum.x < first.maximum.x &&
           first.minimum.y < second.maximum.y && second.minimum.y < first.maximum.y &&
           first.minimum.z < second.maximum.z && second.minimum.z < first.maximum.z;
}

bool containsSpace(const MaximalSpace &outer, const MaximalSpace &inner)
{
    return outer.minimum.x <= inner.minimum.x && outer.maximum.x >= inner.maximum.x &&
           outer.minimum.y <= inner.minimum.y && outer.maximum.y >= inner.maximum.y &&
           outer.minimum.z <= inner.minimum.z && outer.maximum.z >= inner.maximum.z;
}

std::vector<MaximalSpace> splitFreeSpace(const MaximalSpace &space, const MaximalSpace &occupied)
{
    if (!spacesIntersect(space, occupied))
        return {space};

    std::vector<MaximalSpace> remainingSpaces;
    auto keepPositiveRegion = [&](const MaximalSpace &region)
    {
        if (hasPositiveDimensions(region))
            remainingSpaces.push_back(region);
    };

    // Each slab keeps the full extent on the other axes. Overlap is intentional:
    // a disjoint partition would lose candidate positions spanning two slabs.
    auto region = space;
    region.maximum.x = std::min(space.maximum.x, occupied.minimum.x);
    keepPositiveRegion(region);
    region = space;
    region.minimum.x = std::max(space.minimum.x, occupied.maximum.x);
    keepPositiveRegion(region);

    region = space;
    region.maximum.y = std::min(space.maximum.y, occupied.minimum.y);
    keepPositiveRegion(region);
    region = space;
    region.minimum.y = std::max(space.minimum.y, occupied.maximum.y);
    keepPositiveRegion(region);

    region = space;
    region.maximum.z = std::min(space.maximum.z, occupied.minimum.z);
    keepPositiveRegion(region);
    region = space;
    region.minimum.z = std::max(space.minimum.z, occupied.maximum.z);
    keepPositiveRegion(region);

    return remainingSpaces;
}

void removeRedundantSpaces(std::vector<MaximalSpace> &spaces)
{
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
    auto &profile = maximalSpacesProfile();
    profile.peakSpacesBeforePruning = std::max(profile.peakSpacesBeforePruning, spaces.size());
    MaximalSpacesPhaseTimer pruningTimer(profile.pruningMicroseconds);
#endif
    std::vector<MaximalSpace> retainedSpaces;

    for (size_t candidateIndex = 0; candidateIndex < spaces.size(); ++candidateIndex)
    {
        checkExecutionDeadline();
        const auto &candidate = spaces[candidateIndex];

        if (!hasPositiveDimensions(candidate))
            continue;

        bool redundant = false;

        for (size_t otherIndex = 0; otherIndex < spaces.size(); ++otherIndex)
        {
            checkExecutionDeadline();
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
            ++profile.pruningPairVisits;
#endif
            if (candidateIndex == otherIndex)
                continue;

            const auto &other = spaces[otherIndex];
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
            ++profile.containmentChecks;
#endif
            if (!containsSpace(other, candidate))
                continue;

            // Strict containment removes the smaller space. Equal spaces keep
            // their first occurrence, so duplicates cannot remove each other.
            const bool earlierOccurrence = otherIndex < candidateIndex;
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
            ++profile.containmentChecks;
#endif
            const bool strictlyContained = !containsSpace(candidate, other);

            if (earlierOccurrence || strictlyContained)
            {
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
                if (strictlyContained)
                    ++profile.containedSpacesRemoved;
                else
                    ++profile.duplicatesRemoved;
#endif
                redundant = true;
                break;
            }
        }
        if (!redundant)
            retainedSpaces.push_back(candidate);
    }

#ifdef BB3D_PROFILE_MAXIMAL_SPACES
    pruningTimer.finish();
    MaximalSpacesPhaseTimer sortingTimer(profile.sortingMicroseconds);
    profile.peakRetainedSpaces = std::max(profile.peakRetainedSpaces, retainedSpaces.size());
#endif
    // Maximum corners only break ties between spaces with the same DBL origin.
    std::sort(retainedSpaces.begin(), retainedSpaces.end(),
              [](const MaximalSpace &first, const MaximalSpace &second)
              {
                  return std::tie(first.minimum.x, first.minimum.z, first.minimum.y, first.maximum.x,
                                  first.maximum.z, first.maximum.y) <
                         std::tie(second.minimum.x, second.minimum.z, second.minimum.y, second.maximum.x,
                                  second.maximum.z, second.maximum.y);
              });
    spaces.swap(retainedSpaces);
}

void subtractOccupiedSpace(std::vector<MaximalSpace> &spaces, const MaximalSpace &occupied)
{
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
    auto &profile = maximalSpacesProfile();
    MaximalSpacesPhaseTimer splitTimer(profile.splitMicroseconds);
#endif
    std::vector<MaximalSpace> remainingSpaces;
    // A placement can intersect several overlapping spaces, not just its source.
    for (const auto &space : spaces)
    {
        checkExecutionDeadline();
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
        ++profile.spacesVisitedForSplit;

        if (spacesIntersect(space, occupied))
            ++profile.affectedSpaces;
#endif
        const auto pieces = splitFreeSpace(space, occupied);
        remainingSpaces.insert(remainingSpaces.end(), pieces.begin(), pieces.end());
    }
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
    splitTimer.finish();
#endif
    removeRedundantSpaces(remainingSpaces);
    spaces.swap(remainingSpaces);
}
