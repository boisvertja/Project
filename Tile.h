#pragma once
#include "stdafx.h"
#include <memory>

class Tile
{
public:
	Tile();
	Tile(Tile* tile);
	Tile(int xy[2], bool barrier);
	Tile(int x, int y, bool barrier);
	~Tile();
	int getX() const;
	int getY() const;
	bool isBarrier() const;
	Tile* getParent();
	void setX(int x);
	void setY(int y);
	void setBarrier(bool barrier);
	void setParent(Tile* parent);
	bool equals(Tile other);

public:
	std::size_t operator()(Tile const& tile) const
	{
		return tile.getX() * 31 + tile.getY() * 31;
	}
	std::size_t operator==(Tile const& tile) const
	{
		return x == tile.getX() &&
			y == tile.getY() &&
			barrier == tile.isBarrier();
	}

private:
	int x;
	int y;
	bool barrier;

	Tile* parent = nullptr;
};

namespace std {
	template <>
	struct hash<Tile>
	{
		std::size_t operator()(const Tile& tile) const
		{
			using std::size_t;
			using std::hash;

			return ((hash<int>()(tile.getX())
				^ (hash<int>()(tile.getY()) << 1)) >> 1)
				^ (hash<int>()((int)tile.isBarrier()) << 1);
		}
	};
}
