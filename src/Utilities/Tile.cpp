#include "Tile.h"

namespace VulkanProject
{
	Tile::Tile()
	{
		x = 0;
		y = 0;
		barrier = false;
	}

	Tile::Tile(Tile* tile)
	{
		this->x = tile->getX();
		this->y = tile->getY();
		this->barrier = tile->isBarrier();

		this->parent = tile->getParent();
	}

	Tile::Tile(int xy[2], bool barrier)
	{
		this->x = xy[0];
		this->y = xy[1];
		this->barrier = barrier;
	}

	Tile::Tile(int x, int y, bool barrier)
	{
		this->x = x;
		this->y = y;
		this->barrier = barrier;
	}

	Tile::~Tile()
	{
	}

	int Tile::getX() const
	{
		return x;
	}

	int Tile::getY() const
	{
		return y;
	}

	bool Tile::isBarrier() const
	{
		return barrier;
	}

	Tile* Tile::getParent()
	{
		return parent;
	}

	void Tile::setParent(Tile* parent)
	{
		this->parent = parent;
	}

	void Tile::setX(int x)
	{
		this->x = x;
	}

	void Tile::setY(int y)
	{
		this->y = y;
	}

	void Tile::setBarrier(bool barrier)
	{
		this->barrier = barrier;
	}

	bool Tile::equals(Tile other)
	{
		if (x == other.getX() &&
			y == other.getY())
		{
			return true;
		}
		return false;
	}
}
