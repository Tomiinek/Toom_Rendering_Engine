#ifndef Thing_h
#define Thing_h

/**
	Thing.h
	Description:
		Class representing single static sprite in the game map.
		Method create acts as factory.
*/

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>
#include <exception>

#include "Geometry.h"
#include "MapTypes.h"
#include "ThingTextures.h"

class Thing {
public:
		
	Thing(Vector3 position, float height, float radius, Sector* sector, std::string name) : 
		pos_({position.X, position.Y}), 
		height_(height), 
		Z(position.Z), 
		radius_(radius), 
		sector_(sector), 
		name_(name) {}

	/**
	@param name		 Name of a thing to be created.
	@param position	 Position of the new thing, position.Z should be between sector->fh and sector->ch.
	@param sector    Sector where specified position is in.
	*/
	static std::unique_ptr<Thing> create(const std::string& name, Vector3 position, Sector* sector) {
		if (name == "barrel") return std::make_unique<Thing>(position, 100, 37, sector, "barrel");
		else std::exception("Thing of the given name is not supported!");
		return {};
	}

	const Vector& get_position() const { return pos_; }
	float get_radius() const { return radius_; }

	/**
	@param from	 Point with respect to which the distance is measured.
	@return		 Distance.
	*/
	float get_distance(Vector from) const { return (pos_ - from).length(); }

	virtual float get_head_level() const { return height_ + Z; }
	virtual float get_direction() const { return 0.0f; }
	virtual float get_foot_level() const { return Z; }
	virtual float get_eye_level() const { return get_head_level(); }
	virtual bool in_collision(const Thing& pos, const Vector& new_pos) const;

	/**
	@param reference Point with respect to which the texture is selected (differenct view angle).
	@return		     Suitable texture with respect to reference param.
	*/
	virtual sf::Texture* get_texture(const Vector& reference) const;
	virtual Sector* get_sec() const { return sector_; }
	 
protected:
	Vector pos_;
	float height_;  // height of the thing
	float Z;	    // level of foots
	float radius_;	// it is cylinder
	Sector* sector_;
	std::string name_;

}; 

#endif // !Thing_h
 