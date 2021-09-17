#pragma once
#include "Tile.h"
#include <string>
#include <fstream>

class Grid
{
public:
	Grid();
	~Grid();
	static std::vector<Tile>& getGrid();
	static void setGrid(std::vector<Tile> grid);
	static void generateGrid();
	static Tile getTileAtPosition(int x, int y);

private:
	static void readBarrierFile();

	static std::vector<Tile> grid;
};
