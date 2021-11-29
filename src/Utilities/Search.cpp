#include "Search.h"

namespace VulkanProject
{
    Path Search::path = Path();
    // G-values - shortest distance found so far from start node to current
    std::unordered_map<Tile, double> Search::gValues = std::unordered_map<Tile, double>();
    std::unordered_map<Tile, double> Search::nodeFValues = std::unordered_map<Tile, double>();
    std::vector<Tile> Search::openNodes = std::vector<Tile>();
    std::vector<Tile> Search::closedNodes = std::vector<Tile>();

    Path Search::generatePath(int start[2], int goal[2])
    {
        path = Path(start, goal);

        Tile startNode = path.getStartNode();

        gValues.insert(std::pair<Tile, double>(startNode, 0));
        nodeFValues.insert(std::pair<Tile, double>(startNode, totalCostToReachNode(startNode) + estimatedDistanceFromCurrentToGoal(startNode)));
        openNodes.push_back(startNode);

        while (!openNodes.empty())
        {
            Tile current = openNodes.back();
            for (auto& entry : nodeFValues)
            {
                // Get the tile from the open list with the lowest 'f' value
                if (std::find(openNodes.begin(), openNodes.end(), entry.first) == openNodes.end())
                {
                    continue;
                }
                double currentFCost = totalCostToReachNode(current) + estimatedDistanceFromCurrentToGoal(current);
                if (entry.second < currentFCost)
                {
                    current = entry.first;
                }
            }

            if (current.equals(path.getGoalNode()))
            {
                std::cout << "Path Found." << std::endl;

                std::vector<Tile> sequence = std::vector<Tile>();
                Tile* parent = new Tile(&current);
                while (!parent->equals(path.getStartNode()))
                {
                    sequence.push_back(parent);
                    parent = parent->getParent();
                }
                std::reverse(sequence.begin(), sequence.end());
                path.setSequence(sequence);
                return path;
            }

            std::vector<Tile> neighbors = getAdjacentNodes(current);

            Tile previousNeighbor = Tile(-1, -1, true);

            for (Tile neighbor : neighbors)
            {
                if (std::find(closedNodes.begin(), closedNodes.end(), neighbor) == closedNodes.end())
                {
                    if (std::find(openNodes.begin(), openNodes.end(), neighbor) == openNodes.end())
                    {
                        neighbor.setParent(new Tile(&current));
                        openNodes.push_back(neighbor);

                        double neighborG = totalCostToReachNode(neighbor);
                        double neighborF = neighborG + estimatedDistanceFromCurrentToGoal(neighbor);

                        nodeFValues.insert(std::pair<Tile, double> { neighbor, neighborF });
                        gValues.insert(std::pair<Tile, double> { neighbor, neighborG });
                    }
                }
                else
                {
                    if (previousNeighbor.getX() != -1 && previousNeighbor.getY() != -1)
                    {
                        if (nodeFValues.at(previousNeighbor) < totalCostToReachNode(neighbor))
                        {
                            neighbor.setParent(new Tile(&current));

                            double neighborG = totalCostToReachNode(current) + distanceBetweenNodes(current, neighbor);
                            double neighborF = neighborG + estimatedDistanceFromCurrentToGoal(neighbor);

                            nodeFValues.insert(std::pair<Tile, double> { neighbor, neighborF });
                            gValues.insert(std::pair<Tile, double> { neighbor, neighborG });

                            if (std::find(closedNodes.begin(), closedNodes.end(), neighbor) != closedNodes.end())
                            {
                                openNodes.push_back(neighbor);
                            }
                        }
                    }
                }
                previousNeighbor = neighbor;
            }

            for (std::vector<Tile>::iterator iter = openNodes.begin(); iter != openNodes.end(); iter++) {
                if (current.equals(*iter))
                {
                    openNodes.erase(iter);
                    break;
                }
            }
            closedNodes.push_back(current);
        }

        std::cout << "No Path Found." << std::endl;

        return path;
    }

    bool Search::lineOfSight(Tile current, Tile neighbor)
    {
        return false;
    }

    double Search::totalCostToReachNode(Tile current)
    {
        for (const std::pair<Tile, double>& entry : gValues)
        {
            if (entry.first.getX() == current.getX() &&
                entry.first.getY() == current.getY() &&
                entry.first.isBarrier() == current.isBarrier())
            {
                return entry.second;
            }
        }
        return gValues.at(path.getCurrentEndNode()) + 1;
    }

    double Search::estimatedDistanceFromCurrentToGoal(Tile current)
    {
        // This assumes a direct diagonal line of sight
        double horizontalDistance = pow(path.getGoalNode().getX() - current.getX(), 2);
        double verticalDistance = pow(path.getGoalNode().getY() - current.getY(), 2);
        double diagDistance = sqrt(horizontalDistance + verticalDistance);

        return diagDistance;
    }

    std::vector<Tile> Search::getAdjacentNodes(Tile current)
    {
        std::vector<Tile> neighbors = std::vector<Tile>();
        std::vector<Tile> grid = Grid::getGrid();
        for (std::vector<Tile>::iterator iter = grid.begin(); iter != grid.end(); iter++) {
            if (abs((*iter).getX() - current.getX()) <= 1 &&
                abs((*iter).getY() - current.getY()) <= 1 &&
                !(*iter).isBarrier() &&
                !current.equals(*iter))
            {
                neighbors.push_back(*iter);
            }
        }

        return neighbors;
    }

    double Search::distanceBetweenNodes(Tile current, Tile neighbor)
    {
        // The abs method below may need to be removed
        return abs(totalCostToReachNode(neighbor) - totalCostToReachNode(current));
    }
}
