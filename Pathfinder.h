#pragma once
#ifndef Artist
#include "Artist.h"
#endif
#ifndef vector
#include <vector>
#endif

class Pathfinder {
public:


	std::vector<std::vector<bool>> findPaths(std::vector<std::vector<bool>> nodeMap, Artist::pos start, std::vector<std::vector<Artist::pos>>* paths);

};