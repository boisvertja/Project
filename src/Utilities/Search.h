#pragma once
#include "Grid.h"
#include "Path.h"
#include <unordered_map>

namespace VulkanProject
{
	class Search
	{
	public:
		static Path generatePath(int start[2], int goal[2]);
	private:
		static std::vector<Tile> getAdjacentNodes(Tile current);
		static bool lineOfSight(Tile current, Tile neighbor);
		static double totalCostToReachNode(Tile current);
		static double inline distanceBetweenNodes(Tile current, Tile neighbor);
		static double inline estimatedDistanceFromCurrentToGoal(Tile current);

		static Path path;
		static std::unordered_map<Tile, double> gValues;
		static std::unordered_map<Tile, double> nodeFValues;
		static std::vector<Tile> openNodes;
		static std::vector<Tile> closedNodes;
	};
}
