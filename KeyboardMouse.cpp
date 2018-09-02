#include "KeyboardMouse.h"

float KeyboardMouse::rotation() {

	int half_width = window_->getSize().x / 2;
	int change = sf::Mouse::getPosition(*window_).x - half_width;

	sf::Mouse::setPosition({ half_width, (int)window_->getSize().y / 2 }, *window_);

	return mouse_sensitivity * (float)change / (float)half_width;
}
