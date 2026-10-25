#ifndef SCLP_CORE_HPP_
#define SCLP_CORE_HPP_

// IWYU pragma: begin_keep

#include "sclp/Types.hpp"

#include "sclp/Geometry/AllocatedObject.hpp"
#include "sclp/Geometry/AllocatedRectangularCuboid.hpp"
#include "sclp/Geometry/Anchor.hpp"
#include "sclp/Geometry/ContinuousVector.hpp"
#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Geometry/Orientation.hpp"
#include "sclp/Geometry/PlaneVector.hpp"
#include "sclp/Geometry/RectangularCuboid.hpp"

#include "sclp/Packing/AllocatedSmallItem.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/EmptySpace.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/LargeObject.hpp"
#include "sclp/Packing/Packing.hpp"
#include "sclp/Packing/SmallItem.hpp"

#include "sclp/EmptySpaceCollection/EmptySpaceCollection.hpp"
#include "sclp/EmptySpaceCollection/EmptySpaceCutter.hpp"

#include "sclp/IO/DiscreteVectorReader.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/IO/JsonKey.hpp"
#include "sclp/IO/LargeObjectReader.hpp"
#include "sclp/IO/SmallItemReader.hpp"

#include "sclp/Allocation/BsgSearch.hpp"
#include "sclp/Allocation/DoubleEffort.hpp"
#include "sclp/Allocation/FitnessComputer.hpp"
#include "sclp/Allocation/GreedySearch.hpp"
#include "sclp/Allocation/GuillotineBlockGenerator.hpp"
#include "sclp/Allocation/PseudoContact.hpp"
#include "sclp/Allocation/State.hpp"
#include "sclp/Allocation/ZhuVolumeLoss.hpp"

#include "sclp/Constraint/AllConstraints.hpp"

#include "sclp/Problem/CliParameters.hpp"
#include "sclp/Problem/Logs.hpp"
#include "sclp/Problem/Problem.hpp"

// IWYU pragma: end_keep

#endif
