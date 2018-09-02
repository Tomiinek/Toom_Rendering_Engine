#ifndef Weapon_h
#define Weapon_h

/**
	Thing.h
	Description:
		Class stores state of weapon and its properties.
*/

#include <vector>
#include <algorithm>
#include <cmath>
#include <utility>

#include "Geometry.h"
#include "Thing.h"

class Player;
class Person;
class GameMap;
class Weapon {
public:

	Weapon(float range, int damage, float cooldown) : 
		range_(range),
		damage_(damage),
		max_cooldown_(cooldown) {}

	/**
	@return		Returns float 0 ... 1, 0 means "ready", 1 means "just fired".
	*/
	float get_cooldown_progress() const { return (float)cooldown_ / (float)max_cooldown_; }

	/**
	@param from  Shooter's position.
	@param to    Target position. 
	@return	     True if param to is reachable from param from by this weapon.
	*/
	bool in_range(const Vector& from, const Vector& to) const { return (to - from).length() <= range_; }

	void update(int milliseconds) {
		if (cooldown_ < 0) cooldown_ = 0;
		if (cooldown_ == 0) return;
		cooldown_ -= milliseconds;
	}

	void fire(const Enemy& shooter);
	void fire(const Player& shooter);

private:

	/**
	@param thing   Target.
	@param shooter Owner of this weapon.
	@return	       Returns validity flag and parameter expressing distance from thing center.
	*/
	std::tuple<float, bool> shoot_thing(const Thing& thing, const Person& shooter);

	float range_;		// centimeters
	int damage_;		// number of hp if hitted ideally
	int cooldown_ = 0;  // milliseconds
	float max_cooldown_;


};


#endif // !Weapon_h_h