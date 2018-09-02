#ifndef Plane_h
#define Plane_h

/**
	Planes.h
	Description:
		A Plane represents a stripe of ceiling or floor. It is only used for drawing.
*/


#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "Player.h"

struct plane_t {

	plane_t(size_t w, short value) : 
		minx(0), maxx(0),
	    top(w, value), bottom(w, 0), 
		height(0), texture(nullptr) {}

	int	minx;
	int	maxx;
	std::vector<short> top;
	std::vector<short> bottom;

	size_t height;
	sf::Texture* texture;

};

using plane_p = std::unique_ptr<plane_t>;

class Planes {
public:

	Planes(int w, int h) : 
		max_width_(w), 
		max_height_(h) {}

	/**
	Draws planes onto target argument, fov and quality can be specified.
	*/
	void draw(sf::RenderTarget & target, sf::RenderStates states, float fov, float quality) const;

	/**
	@param height  Level of the plane we search for.
	@param txt	   Texture of the plane we search for.
	@return		   Found plane.
	*/
	plane_t* find_plane(size_t height, sf::Texture* txt);

	/**
	@param pl		   Plane to be checked.
	@param start	   Starting screen x-coordinate of an interval  being occupied by this plane.
	@param start	   Ending screen x-coordinate of an interval being occupied by this plane.
	@param global_min  Starting screen x-coordinate of all intervals being occupied by this plane.
	@param global_max  Ending screen x-coordinate of all intervals being occupied by this plane.
	@return			   If pl is not suitable, new plain is returned, otherwise pl is returned.
	*/
	plane_t* check_plane(plane_t* pl, int start, int stop, int global_min, int global_max);

	void draw_horizontal_line(int y, int x1, int x2, int t_x1, int t_y1, int t_x2, int t_y2, float quality, sf::RenderTarget & target, sf::RenderStates states) const;

	std::vector<plane_p> planes;

private:	
	int max_width_;
	int max_height_;
};


#endif // !Plane_h
