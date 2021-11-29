#include "Grid.h"

namespace VulkanProject
{
	std::vector<Tile> Grid::grid = std::vector<Tile>();

	Grid::Grid()
	{
	}

	Grid::~Grid()
	{
	}

	std::vector<Tile>& Grid::getGrid()
	{
		if (grid.empty())
		{
			readBarrierFile();
		}
		return grid;
	}

	void Grid::setGrid(std::vector<Tile> grid)
	{
		Grid::grid = grid;
	}

	Tile Grid::getTileAtPosition(int x, int y)
	{
		for (Tile tile : grid)
		{
			if (tile.getX() == x && tile.getY() == y)
			{
				return tile;
			}
		}
		return Tile();
	}

	void Grid::generateGrid()
	{
		readBarrierFile();
	}

	void Grid::readBarrierFile()
	{
		std::ifstream barrierFile("./Barriers.txt");
		if (barrierFile.is_open())
		{
			int row = 0;
			std::string line;
			while (std::getline(barrierFile, line))
			{
				int col = 0;
				for (unsigned int i = 0; i < line.length(); i++)
				{
					char value = line.at(i);
					if (value == ',' || value == ' ')
					{
						continue;
					}
					bool barrier = (value == '1');
					Tile tile = Tile(col, row, barrier);
					grid.push_back(tile);
					col++;
				}
				row++;
			}
			barrierFile.close();
		}
		else
		{
			std::cerr << "ERROR::Unable to open file!" << std::endl;
		}
	}
}
