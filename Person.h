#ifndef Person_h
#define Person_h

/**
	Person.h
	Description:
		Class representing single static sprite in the game map.
		Method create acts as factory.
*/

#include "Geometry.h"
#include "Thing.h"
#include "BlockMap.h"
#include "Weapon.h"

class GameMap;
class Person : public Thing {
public:

	Person(Vector3 position, float height, float radius, Sector* sector, Weapon weapon, GameMap* map, std::string name) : 
		Thing(position, height, radius, sector, name), 
		weapon_(weapon), 
		map_(map) {}

	virtual void damage(float hp) { health_ -= hp; }
	bool is_alive() const { return health_ > 0; }
	float get_health() const { return health_; }

	GameMap* get_map() const { return map_; }

	/**
	@return		 Returns angle, 0 goes to north (positive y), 90 south ->, 270 west <-.
	*/
	float get_direction() const override { return angle_; }

	virtual float get_max_speed() const { return max_speed_; }
	virtual float get_step_height() const { return step_height_; }

	Vector3 get_velocity() const { return velocity_; }
	void change_velocity(Vector3 const& change) { velocity_ += change; }

protected:

	/**
	@param seconds_elapsed Seconds elapsed since last update.
	@param direction	   Direction in which the object is being pushed.
	@param pushing		   True if there is active force (needed for inertia).
	@return                Returns true if no collision was detected, false otherwise.
	*/
	bool move_to(float seconds_elapsed, const Vector& direction, bool pushing);
	
	Vector3 velocity_ = { 0,0,0 };	// centimeters per second
	float max_speed_ = 250;		    // centimeters per second 
	float angle_ = 0;				// degrees	
	float health_ = 100;
	bool falling_ = false;
	Weapon weapon_;

	const float acceleration_ = 3000;  // centimeters per socond^2
	const float gravity_a = 1100; 
	const float step_height_ = 50;
	
	GameMap* map_;

private: 

	/**
	@param block  Map block whose segments and things are checked against collision with this object.
	@param new_pos      New precomputed position of this object, it can be updated, reference needed!
	*/
	void collision_detection(const lines_p_t * block, Vector& new_pos);
};


#endif // !Person_h
