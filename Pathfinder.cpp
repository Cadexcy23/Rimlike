#include <vector>
#include "Pathfinder.h"
#include "Artist.h"


std::vector<Artist::pos> getNeighbors(Artist::pos start, int diag, std::vector<std::vector<bool>> nodeMap)
{
	std::vector<Artist::pos> returnNeighbors;

	//up
	returnNeighbors.push_back({ start.x, start.y - 1 });
	//right
	returnNeighbors.push_back({ start.x + 1, start.y });
	//down
	returnNeighbors.push_back({ start.x, start.y + 1 });
	////left
	returnNeighbors.push_back({ start.x - 1, start.y });

	if (diag == 1)
	{
		//up right
		returnNeighbors.push_back({ start.x + 1, start.y - 1 });
		//down right
		returnNeighbors.push_back({ start.x + 1, start.y + 1 });
		//down left
		returnNeighbors.push_back({ start.x - 1, start.y + 1 });
		//up left
		returnNeighbors.push_back({ start.x - 1, start.y - 1 });
	}
	else if (diag == 2)
	{
		//up right
		if (start.x + 1 >= 0 && start.x + 1 < nodeMap.size() && start.y - 1 >= 0 && start.y - 1 < nodeMap[0].size())//check if the spot is on the node map
		{
			if (nodeMap[returnNeighbors[0].x][returnNeighbors[0].y] && nodeMap[returnNeighbors[1].x][returnNeighbors[1].y])//check if then path diagonaly is open
			{
				returnNeighbors.push_back({ start.x + 1, start.y - 1 });
			}
		}
		//down right
		if (start.x + 1 >= 0 && start.x + 1 < nodeMap.size() && start.y + 1 >= 0 && start.y + 1 < nodeMap[0].size())//check if the spot is on the node map
		{
			if (nodeMap[returnNeighbors[2].x][returnNeighbors[2].y] && nodeMap[returnNeighbors[1].x][returnNeighbors[1].y])
			{
				returnNeighbors.push_back({ start.x + 1, start.y + 1 });
			}
		}
		//down left
		if (start.x - 1 >= 0 && start.x - 1 < nodeMap.size() && start.y + 1 >= 0 && start.y + 1 < nodeMap[0].size())//check if the spot is on the node map
		{
			if (nodeMap[returnNeighbors[2].x][returnNeighbors[2].y] && nodeMap[returnNeighbors[3].x][returnNeighbors[3].y])
			{
				returnNeighbors.push_back({ start.x - 1, start.y + 1 });
			}
		}
		//up left
		if (start.x - 1 >= 0 && start.x - 1 < nodeMap.size() && start.y - 1 >= 0 && start.y - 1 < nodeMap[0].size())//check if the spot is on the node map
		{
			if (nodeMap[returnNeighbors[0].x][returnNeighbors[0].y] && nodeMap[returnNeighbors[3].x][returnNeighbors[3].y])
			{
				returnNeighbors.push_back({ start.x - 1, start.y - 1 });
			}
		}
	}

	return returnNeighbors;
}

std::vector<std::vector<bool>> Pathfinder::findPaths(std::vector<std::vector<bool>> nodeMap, Artist::pos start, std::vector<std::vector<Artist::pos>>* paths)
{
	//declare all the variables
	//current frontier point
	Artist::pos current;
	//keeps track of moves
	std::vector<Artist::pos> frontier;
	//keep track of where we came from
	std::vector<std::vector<Artist::pos>> cameFrom;
	//keeptrack of where we can go
	std::vector<std::vector<bool>> visitable;
	//list of points next to the one we are on
	std::vector<Artist::pos> neighbors;
	//init all the variables
	//start frontier
	frontier.push_back(start);
	//set current 
	current = frontier.front();
	//resize tables
	cameFrom.resize(nodeMap.size());
	for (int i = 0; i < nodeMap.size(); i++)
		cameFrom[i].resize(nodeMap[0].size());
	visitable.resize(nodeMap.size());
	for (int i = 0; i < nodeMap.size(); i++)
		visitable[i].resize(nodeMap[0].size());
	//clear tables and set start point
	Artist::pos invalidPos = { -1, -1 };
	for (int x = 0; x < cameFrom.size(); x++)
	{
		for (int y = 0; y < cameFrom[0].size(); y++)
		{
			cameFrom[x][y] = invalidPos;
		}
	}
	Artist::pos initStart = { start.x, start.y }; //seems useless?
	cameFrom[start.x][start.y] = initStart;
	visitable[start.x][start.y] = true;
	//init done


	//run until frontier is empty
	while (!frontier.empty())
	{
		//take the spot are about to use off the frontier
		frontier.erase(frontier.begin());
		//get starting points neighbors
		neighbors = getNeighbors(current, 2, nodeMap);
		//go through all neighbors
		for (int i = 0; i < neighbors.size(); i++)
		{
			//make sure it is within the bounds of the node map
			if (neighbors[i].x >= 0 && neighbors[i].x < nodeMap.size() && neighbors[i].y >= 0 && neighbors[i].y < nodeMap[0].size())
			{
				//make sure we havnt found a path to this neighbor yet
				if (cameFrom[neighbors[i].x][neighbors[i].y].x == -1 && cameFrom[neighbors[i].x][neighbors[i].y].y == -1)
				{
					//add it to the frontier if we can walk on it and add it to the visitable table
					if (nodeMap[neighbors[i].x][neighbors[i].y])
					{
						frontier.push_back(neighbors[i]);
						visitable[neighbors[i].x][neighbors[i].y] = true;
						//add the position we found this neighbor from as the point we got to this one from
						cameFrom[neighbors[i].x][neighbors[i].y] = current;
					}
				}
			}
		}
		//update current and move on to next spot
		if (frontier.size() > 0)
		{
			current = frontier.front();
		}
	}






	//return final values
	*paths = cameFrom;
	return visitable;
}


