#ifndef Player_h
#define Player_h

/**
	Player.h
	Description:
		Stores state of Player. Singleton structure.
		Derived from Person class, adds sneaking, jumping and gun animation.
*/

#include <string>
#include <vector>
#include <cmath>
#include <memory>
#include <exception>

#include <SFML/Graphics.hpp>

#include "Person.h"
#include "Weapon.h"
#include "GameMap.h"
#include "MapTypes.h"
#include "Sounds.h"
#include "Controller.h"


class Player : public Person, public sf::Drawable, public sf::Transformable {
private:

	/**
	Other essential parameters like Secotr or Map are handled in set method
	*/
	Player() : Person({0,0,0}, 185, 30, nullptr, Weapon(2000, 50, 250), nullptr, ""), shooting_progress_(0) {};

public:

	Player(Player const&) = delete;
	void operator=(Player const&) = delete;

	static Player& instance() {	 
		static Player instance;
		return instance;
	}

	/**
	Method used to draw gun and its animation while shooting
	*/
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		target.draw(gun_drawing_);
	}

	float get_eye_level() const override { return get_head_level() - head_margin_; }
	float get_head_level() const override { return (sneaking ? sneak_height_ : height_) + Z; }
	float get_max_speed() const override { return (sneaking && !falling_ ? max_speed_ / 2.0f : max_speed_); }
	void damage(float hp) override { 
		health_ -= hp;
		if (health_ <= 0) Sounds::global().play(Sounds::death, 100);
	}

	/**
	@param seconds_elapsed  Seconds elapsed since last update.
	@param direction		Direction in which the object is being pushed (only for directions and their combinations).
	@param angle_change     Change in view angle (typical by mouse movement).
	*/
	void move_to(float seconds_elapsed, Controller::Direction  move_dir, float angle_change);

	void fire() { weapon_.fire(*this); }
	void jump() {
		if (falling_) return;
		velocity_.Z = jump_speed_;
		falling_ = true;
		Sounds::global().play(Sounds::jump, 100);
	}

	/**
	@param change  True in order to set Player sneaking, false otherwise.
	*/
	void set_sneaking(bool change) { 
		if (!change && sector_->ch < height_ + Z) return;
		if (falling_ && change != sneaking) {
			if (change) Z += sneak_height_;
			else   Z -= sneak_height_;
		}
		sneaking = change;
	}

	/**
	@param change  True in order to set Player sneaking, false otherwise.
	*/
	void set(Vector posisiton, float angle, Sector* sector, GameMap* map) {
		pos_ = posisiton;
		angle_ = angle;
		map_ = map;
		sector_ = sector;
		Z = static_cast<float>(sector->fh);
	}

	/**
	@sa set_gun
	@param files  Images to be loaded, each image represents one frame in animation.
	*/
	void set_gun_animation(const std::vector<std::string>& files);

	/**
	@param window Main application window 
	@param file   Images to be loaded, used when player is not shooting.
	*/
	void set_gun(const sf::RenderWindow & window, const std::string& file);

	void update_shooting(int millis_elapsed);

private:
	bool sneaking = false;
	const float sneak_height_ = 100;  // centimeters
	const float jump_height_ = 80;
	const float jump_speed_ = 350;
	const float head_margin_ = 15;

	float shooting_progress_;

	sf::Texture texture_;
	std::vector<std::unique_ptr<sf::Texture>> animation_;
	sf::Sprite gun_drawing_;
};


#endif // !Player_h
