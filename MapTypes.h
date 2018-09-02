#ifndef MapTypes_h
#define MapTypes_h

/**
	MapTypes.h
	Description:
		Definition of all map types!
*/

#include <SFML/Graphics.hpp>
#include "Geometry.h"

#include <vector>
#include <memory>

class Thing;
class Enemy;
using things_up_t = std::vector<std::unique_ptr<Thing>>;
using things_p_t = std::vector<Thing*>;


/**
Sectors are used to define areas with different floor and ceiling height and textures.
Sectors do not have to be convex.
*/
struct Sector {

	virtual ~Sector() = default;
	Sector(int floor_h, int ceiling_h, sf::Texture* floor_t, sf::Texture* ceiling_t) : 
		fh(floor_h), 
		ch(ceiling_h), 
		ft(floor_t), 
		ct(ceiling_t){}

	int fh;
	int ch;
	sf::Texture* ft;
	sf::Texture* ct;
	things_p_t thgs;

	virtual bool is_door() { return false; }

};
using sectors_t = std::vector<std::unique_ptr<Sector>>;


/**
Line has one or two sides. These sides are defined using this structure. It specifies textures
and shift of the textures.
*/
struct Side {

	sf::Texture* lower;
	sf::Texture* middle;
	sf::Texture* upper;
	size_t tx;
	size_t ty;
	Sector* sector;

};
using sides_p_t = std::vector<Side*>;
using sides_t = std::vector<Side>;


/**
Door is derived from Sector. If you want to have a pretty door, avoid the difference between
door and neighbour sectors floor, door cannot have middle texture! Door should have own side definition!
*/
struct Door : public Sector {

	enum State { open, closed, moving_up, moving_down };

	Door(int floor_h, int ceiling_h, sf::Texture* floor_t, sf::Texture* ceiling_t)
		: Sector(floor_h, floor_h, floor_t, ceiling_t), 
		seconds_opened(0),
		open_ch(ceiling_h), 
		s(Door::closed) {}

	float seconds_opened;
	int open_ch;
	State s;
	sides_p_t sides;
	 
	static const int door_speed = 200;
	static const int door_open_time = 3;
	// must be in harm with blocksize, should be in (200,0)
	static const int door_open_distance = 150;

	bool is_door() override { return true; }
};
using doors_p_t = std::vector<Door*>;


/**
Line defines a wall. One sided Line is solid wall. Two sided line is not wall, but empty space
between two sectors. By specifing two sided line with transparent textures, special transparent 
wall (player can walk through this wall) is defined.
*/
struct Line {

	Vector* start;
	Vector* end;
	Side* left;
	// nullptr if onesided	
	Side* right; 

	/**
	@param test_s  Start of the segment being checked.
	@param test_e  End of the segment being checked.
	@return        Returns true if the segment collides with the Line
	*/
	bool in_collision(const Vector& test_s, const Vector& test_e) const;

private:
	/**
	@return   Returns 0 if p, q and r are colinear, 1 if clockwise, 2 if counterclockwise
	*/
	static int orientation(Vector p, Vector q, Vector r);
	bool on_segment(Vector p, Vector q, Vector r) const;

};


/**
Segment is a part of Line. It is used by BSP tree to define sub-sectors.
*/
struct Segment {

	Vector start;
	Vector end;
	const Line* line;
	bool direction;
	// 0 left  side
	// 1 right side
	float offset;

	bool operator==(const Segment& s) const { return (start == s.start) && (end == s.end) && (direction == s.direction); }

};
using segments_t = std::vector<Segment>;



using textures_up_t = std::vector<std::unique_ptr<sf::Texture>>;
using vertices_t = std::vector<Vector>;
using lines_t	 = std::vector<Line>;
using lines_p_t  = std::vector<const Line*>;

using enemies_p_t = std::vector<std::unique_ptr<Enemy>>;

#endif // MapTypes_h
