#ifndef Controller_h
#define Controller_h

/**
	Controller.h
	Description:
		Interface for player's Controller, defines methods which should be
		supported by any controller.
*/

#include "SFML\Window.hpp"

class Controller {
public:

	enum Direction { fwd = 1, bck = 2, lft = 4, rgt = 8, };

	virtual ~Controller() {}
	virtual bool fire() = 0;
	virtual bool forward() = 0;
	virtual bool back() = 0;
	virtual bool left() = 0;
	virtual bool right() = 0;
	virtual bool action() = 0;
	virtual bool jump() = 0;
	virtual bool sneak() = 0;

	/**
	@return Value between -1 and 1, 0 means no change.
	*/
	virtual float rotation() = 0;	
	/**
	@return Direction, see enum Controller::Direction.
	*/
	virtual Direction direction() = 0;

	/**
	@return For setting up controller's defaults.
	*/
	virtual void set(const sf::Window& w) = 0;
};



#endif // !Controller_h
