#ifndef Enemy_h
#define Enemy_h

/**
	Enemy.h
	Description:
		Stores state of enemy sprite. Method create acts as factory.
		Derived from Person class.
*/

#include "Weapon.h"
#include "Person.h"

#include <random>
#include <memory>

class Enemy : public Person {
public:

	Enemy(Vector3 position, float height, float radius, Sector* sector, Weapon weapon, GameMap* map, const std::string& name) :
		Person(position, height, radius, sector, weapon, map, name), target_(nullptr) {}

	/**
	@param name		 Name of a enemy sprite to be created.
	@param position	 Position of the new enemy, position.Z should be between sector->fh and sector->ch.
	@param sector    Sector where specified position is in.
	@param map       Pointer to the map, where enemy live.
	*/
	static std::unique_ptr<Enemy> create(const std::string& name, const Vector3& position, Sector* sector, GameMap* map) {
		if (name == "enemy1") return std::unique_ptr<Enemy>(new Enemy({ position.X, position.Y, position.Z }, 160, 53, sector, Weapon(800, 10, 3000), map, "enemy1"));
		else std::exception("Thing of the given name is not supported!");
		return {};
	}

	bool is_activated() const { return activated_; }
	void activate(Person* target) { activated_ = true; target_ = target; }

	/**
	Performs simple AI.
	@param seconds_elapsed  Seconds ellapsed since last Update.
	*/
	void make_action(float seconds_ellapsed);

	const static int activation_range = 500; // centimeters
	const static int min_move_time = 1;      // seconds
	const static int state_change_time = 3;  // seconds

private:

	/**
	Just translation function, calls Person::move_to, affects block_
	*/
	void move_to(float seconds_elapsed, Vector dir, bool pushing);

	/**
	Just translation function, calls Person::move_to, affects block_
	*/
	void move_to(float seconds_elapsed, float dir, bool pushing);
	
	Person* target_;
	bool activated_ = false;
	bool blocked_ = false;
	bool shooting_ = false;
	float move_time_ = 0;
	float change_time_ = 0;	

};


#endif // !Enemy_h
