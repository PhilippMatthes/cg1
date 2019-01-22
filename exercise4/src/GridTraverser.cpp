// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "GridTraverser.h"
#include "GridUtils.h"


GridTraverser::GridTraverser()
{ }

GridTraverser::GridTraverser(const Eigen::Vector3f& o, const Eigen::Vector3f&d, const Eigen::Vector3f cell_extents)
	: orig(o), dir(d), cellExtents(cell_extents)
{
	dir.normalize();
	Init();
}

Eigen::Vector3f& GridTraverser::Origin()
{
	return orig;
}
const Eigen::Vector3f& GridTraverser::Origin() const
{
	return orig;
}

Eigen::Vector3f& GridTraverser::Direction()
{
	return dir;
}

const Eigen::Vector3f& GridTraverser::Direction() const
{
	return dir;
}

void GridTraverser::SetCellExtents(const Eigen::Vector3f& cellExtent)
{
	this->cellExtents = cellExtent;
	Init();
}

void GridTraverser::Init()
{
	current = PositionToCellIndex(orig, cellExtents);
	/* Task 3.2.2 */
	//you can add some precalculation code here
	// https://tu-dresden.de/ing/informatik/smt/cgv/ressourcen/dateien/lehre/ws-18-19/cg1/CGI_11_Grids-and-Hierarchies.pdf?lang=de
	// https://stackoverflow.com/questions/49188302/how-can-you-iterate-linearly-through-a-3d-grid
	// https://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/grid
	// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.42.3443&rep=rep1&type=pdf
	// https://github.com/francisengelmann/fast_voxel_traversal/blob/master/main.cpp


	stepX = dir(0) < 0 ? -cellExtents(0) : cellExtents(0);
	stepY = dir(1) < 0 ? -cellExtents(1) : cellExtents(1);
	stepZ = dir(2) < 0 ? -cellExtents(2) : cellExtents(2);

	float nextVoxelBoundaryX = (current(0) + stepX) * cellExtents(0);
	float nextVoxelBoundaryY = (current(1) + stepY) * cellExtents(1);
	float nextVoxelBoundaryZ = (current(2) + stepZ) * cellExtents(2);

	if (stepX < 0) nextVoxelBoundaryX += cellExtents(0);
    if (stepY < 0) nextVoxelBoundaryY += cellExtents(1);
    if (stepZ < 0) nextVoxelBoundaryZ += cellExtents(2);

	tMaxX = dir(0) != 0.0 ? (nextVoxelBoundaryX - orig(0)) / dir(0) : FLT_MAX;
	tMaxY = dir(1) != 0.0 ? (nextVoxelBoundaryY - orig(1)) / dir(1) : FLT_MAX;
	tMaxZ = dir(2) != 0.0 ? (nextVoxelBoundaryZ - orig(2)) / dir(2) : FLT_MAX;

	tDeltaX = cellExtents(0) / dir(0) * stepX;
	tDeltaY = cellExtents(1) / dir(1) * stepY;
	tDeltaZ = cellExtents(2) / dir(2) * stepZ;
}

void GridTraverser::operator++(int)
{
	/* Task 3.2.2 */
	//traverse one step along the ray
	//update the cell index stored in attribute "current"
	if (tMaxX < tMaxY) {
		if (tMaxX < tMaxZ) {
			orig(0) += stepX;
			tMaxX += tDeltaX;
		} else {
			orig(2) += stepZ;
			tMaxZ += tDeltaZ;
		}
	} else {
		if (tMaxY < tMaxZ) {
			orig(1) += stepY;
			tMaxY += tDeltaY;
		} else {
			orig(2) += stepZ;
			tMaxZ += tDeltaZ;
		}
	}
	current = PositionToCellIndex(orig, cellExtents);
}

Eigen::Vector3i GridTraverser::operator*()
{
	return current;
}

	
