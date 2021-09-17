#pragma once
#include "Tile.h"
#include <unordered_map>

class Path {
public:
	Path();
	Path(int start[2], int goal[2]);
	~Path();
	void addTile(Tile tile);
	bool removeTile(Tile tile);
	bool contains(Tile tile);
	void printPath() const;
	void setSequence(std::vector<Tile> tileSequence);
	std::vector<Tile> getSequence() const;
	Tile getStartNode();
	Tile getGoalNode();
	Tile getCurrentEndNode();

private:
	std::vector<Tile> tileSequence = std::vector<Tile>();
	Tile startNode;
	Tile goalNode;
};
