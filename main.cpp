#include "Search.h"

int main()
{
	Grid grid = Grid();
	grid.generateGrid();

	int startArr[2] = { 1, 1 };
	int goalArr[2] = { 1, 7 };
	Tile start = Tile(startArr, false);
	Tile goal = Tile(goalArr, false);
	Path path = Search::generatePath(startArr, goalArr);
	path.printPath();

	std::cin.get();

	return 0;
}
