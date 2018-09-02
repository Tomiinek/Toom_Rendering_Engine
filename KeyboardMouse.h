#ifndef KeyboardMouse_h
#define KeyboardMouse_h

/**
	KeyboardMouse.h
	Description:
		Specific controller type. Singleton structure.
*/

#include <SFML/Window.hpp>

#include "Controller.h"

class KeyboardMouse : public Controller {
private:

	KeyboardMouse(): window_(nullptr) {}

	int fire_ = sf::Mouse::Left;
	int forward_ = sf::Keyboard::W;
	int back_ = sf::Keyboard::S;
	int left_ = sf::Keyboard::A;
	int right_ = sf::Keyboard::D;
	int action_ = sf::Keyboard::E;
	int jump_ = sf::Keyboard::Space;
	int sneak_ = sf::Keyboard::LControl;

	float mouse_sensitivity = 25;

	const sf::Window* window_;

public:

	static KeyboardMouse& instance() {
		static KeyboardMouse instance;
		return instance;
	}

	void set(const sf::Window& w) override {
		window_ = &w;
	}

	KeyboardMouse(KeyboardMouse const&) = delete;
	void operator=(KeyboardMouse const&) = delete;

	virtual bool fire() override { return sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(fire_)); }
	virtual bool forward() override { return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(forward_)); }
	virtual bool back() override { return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(back_)); }
	virtual bool left() override { return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(left_)); }
	virtual bool right() override { return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(right_)); }
	virtual bool action() override { return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(action_)); }
	virtual bool jump() override { return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(jump_)); }
	virtual bool sneak() override { return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(sneak_)); }

	virtual float rotation() override;

	virtual Direction direction() override {	
		// for understanding shifts, see Controller::Direction enum
		return static_cast<Direction>(forward() | back() << 1 | left() << 2 | right() << 3);
	}

};


#endif // !KeyboardMouse_h
