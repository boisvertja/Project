#include "Path.h"

Path::Path()
{
}

Path::Path(int start[2], int goal[2])
{
    startNode = Tile(start[0], start[1], false);
    goalNode = Tile(goal[0], goal[1], false);

    startNode.setParent(new Tile(startNode));

    addTile(startNode);
}

Path::~Path()
{
}

void Path::addTile(Tile tile)
{
    tileSequence.push_back(tile);
}

bool Path::removeTile(Tile tile)
{
    for (std::vector<Tile>::iterator iter = tileSequence.begin(); iter != tileSequence.end(); iter++) {
        if (tile.equals(*iter))
        {
            tileSequence.erase(iter);
            return true;
        }
    }
    return false;
}

bool Path::contains(Tile tile)
{
    for (std::vector<Tile>::iterator iter = tileSequence.begin(); iter != tileSequence.end(); iter++)
    {
        if (tile.equals(*iter))
        {
            return true;
        }
    }
    return false;
}

void Path::printPath() const
{
    for (auto const& tile : tileSequence)
    {
        std::cout << "[" << tile.getX() << "," << tile.getY() << "]" << std::endl;
    }
}

void Path::setSequence(std::vector<Tile> tileSequence)
{
    this->tileSequence = tileSequence;
}

std::vector<Tile> Path::getSequence() const
{
    return tileSequence;
}

Tile Path::getStartNode()
{
    return startNode;
}

Tile Path::getGoalNode()
{
    return goalNode;
}

Tile Path::getCurrentEndNode()
{
    return tileSequence.at(tileSequence.size() - 1);
}
