#ifndef Game_h
#define Game_h

/**
	Game.h
	Description:
		Root class of whole game. Controlls drawing, updates and map.
*/

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Player.h"
#include "Scene.h"
#include "GameMap.h"
#include "Controller.h"
#include "KeyboardMouse.h"
#include "Sounds.h"
#include "ThingTextures.h"

#include <exception>
#include <string>
#include <sstream>

class Game : public sf::Drawable, public sf::Transformable {
public:

	/**
	Run game including its loop.
	*/
	void run();

	const GameMap& get_map() const { return map_; }
	const Player& get_player() const { return player_; }

private:

	/**
	Update whole game.
	*/
	void update(int millis_elapsed);

	/**
	Draw whole game into specified target.
	*/
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	/**
	Load game map.
	*/
	void load(const std::string& file_name) {
		if (!map_.load(file_name)) 
			throw std::exception("Error while loading map!");
	}

	Scene scene_{ this };
	GameMap map_;
	Controller& controller_ = KeyboardMouse::instance();
	const Player& player_ = Player::instance();

};

#endif // !Game_h
