#ifndef Scene_h
#define Scene_h

/**
	Game.h
	Description:
		Here happens drawing. BSP tree is traversed and subsectors are drawn.
*/

#include <SFML/Graphics.hpp>
#include <queue>
#include <stack>
#include <unordered_map>
#include <set>
#include <tuple>
#include <array>
#include <vector>

#include "Player.h"
#include "GameMap.h"
#include "MapTypes.h"
#include "Planes.h"

class Game;

class Scene : public sf::Drawable, public sf::Transformable {
public:
	
	explicit Scene(Game* owner) : game_(owner) {}
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	int get_fov() const { return fov_; }
	
private:
	
	/**
	@param b1	    First bound.
	@param b2		Second bound.
	@param between  Value to be checked.
	@return			Returns true if between is in the interval defined by b1 and b2	  
	*/
	bool is_usable(float between, float b1, float b2) const;

	/**
	@return  Returns intersection point of (v1,v2) and (v3,v4).
	*/
	Vector intersection(const Vector& v1, const Vector& v2, const Vector& v3, const Vector& v4) const;

	using projection_h_res = std::tuple<Vector, Vector, int, int, int, int, int, int, Vector, Vector, bool>;
	using projection_v_res = std::tuple<int, int, int, int>;

	projection_h_res project_plane_horizontally(const Vector& start, const Vector& end, int texture_shift, int width, int height) const;
	projection_v_res project_plane_vertically(int top, int bottom, const Vector& scale_s, const Vector& scale_e, int height) const;

	int interpolate(int pos, int start, int end, int min, int max) const {
		return static_cast<int>((int)((float)(pos - start) / (float)(end - start) * (float)(max - min) + min));
	}
	int interpolate_txt_u(int range, int shift, int max, int min, int clamped) const {
		return static_cast<int>(range * (float)(max - clamped) / (float)(max - min) + shift);
	}
	int interpolate_txt_l(int range, int shift, int max, int min, int clamped) const {
		return static_cast<int>(range * (1 - (float)(clamped - min) / (float)(max - min)) + shift);
	}

	void vertical_line_c(int x, int y1, int y2, sf::Color c, sf::RenderTarget & target, sf::RenderStates states) const;
	void vertical_line(int x, int y1, int y2, int t_x, int t_y1, int t_y2, sf::RenderTarget & target, sf::RenderStates states) const;

	/**
	Object for rendering, act as buffer
	*/
	struct sprite_drawing {	
		sf::Texture* texture;
		// holds, x, top, bottom, texture x, texture top, texture bottom
		std::vector<std::array<int, 6>> data;
	};

	Game* game_;
	const int fov_ = 70;  // degrees
	const float texture_quality_ = 3.15f; // 512 pixels per 140 cm
};

#endif // !Scene_h
