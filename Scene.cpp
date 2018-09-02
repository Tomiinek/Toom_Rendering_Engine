#include "Scene.h"
#include "Game.h"

using namespace std;

#define Textured

inline int clamp(int n, int lower, int upper) {
	return std::max(lower, std::min(n, upper));
}

void Scene::vertical_line_c(int x, int y1, int y2, sf::Color c, sf::RenderTarget & target, sf::RenderStates states) const {

	sf::Vertex l[2] =
	{
		sf::Vertex(sf::Vector2f((float)x, (float)y1), c),
		sf::Vertex(sf::Vector2f((float)x, (float)y2), c)
	};

	target.draw(l, 2, sf::Lines, states);	
}

void Scene::vertical_line(int x, int y1, int y2, int t_x, int t_y1 ,int t_y2, sf::RenderTarget & target, sf::RenderStates states) const {

	sf::Vertex l[2] =
	{
		sf::Vertex(sf::Vector2f((float)x, (float)y1), sf::Vector2f((float)t_x,    (float)t_y1  * texture_quality_)),
		sf::Vertex(sf::Vector2f((float)x, (float)y2), sf::Vector2f((float)t_x + 1.0f, (float)t_y2  * texture_quality_))
	};

	target.draw(l, 2, sf::Lines, states);
}

bool Scene::is_usable(float between, float b1, float b2) const {

	float left = min(b1, b2);
	float right = max(b1, b2);

	return (between >= left && between <= right);
}

Vector Scene::intersection(const Vector& v1, const Vector& v2, const Vector& v3, const Vector& v4) const {

	float d = (v1 - v2) % (v3 - v4);
	float det1 = v1 % v2;
	float det2 = v3 % v4;

	return Vector {
				    Vector{ det1, v1.X - v2.X } % Vector{ det2, v3.X - v4.X } / d,
				    Vector{ det1, v1.Y - v2.Y } % Vector{ det2, v3.Y - v4.Y } / d
	              };
}

void Scene::draw(sf::RenderTarget & target, sf::RenderStates states) const {

	const GameMap& map = game_->get_map();

	// cached drawing (transparent walls, sprites)
	std::stack<sprite_drawing> drawings;

	// hold already resolved walls, needed to prevent drawing both sides of transprent walls
	// index is pointer to the wall and wall side is determined by bool value
	std::unordered_map<const Line*, bool> resolved_walls;

	// remembers which things were already resolved
	std::set<Thing*> things_resolved;

	const Player& player = Player::instance();

	int screen_width = target.getSize().x;
	int screen_height = target.getSize().y;

	Planes planes(screen_width, screen_height);

	// structures for representing clear windows on screen
	static vector<short int> y_top(screen_width, 0), y_bottom(screen_width, screen_height - 1);
	static vector<bool> x_vis(screen_width, false);

	std::fill(y_top.begin(), y_top.end(), 0);
	std::fill(y_bottom.begin(), y_bottom.end(), screen_height - 1);
	std::fill(x_vis.begin(), x_vis.end(), false);

	// BSP tree traversal:
	const BSPTree& tree = map.get_bsp_tree();
	for (auto i = tree.begin(player.get_position()); i != tree.end(player.get_position()); ++i) {
		
		// TODO: Bounding box check here:

		// only and a little bit ugly way to get sector which contains the subsector
		const auto& sector_line = (*i).front();
		Side* side = (sector_line.direction ? sector_line.line->right : sector_line.line->left);
		Sector* curr_sector = side->sector;

		//
		// Things drawing computation
		//

		// we need to sort things in certain sector by distance from player
		auto cmp = [](std::pair<float, sprite_drawing> t1, std::pair<float, sprite_drawing> t2) { return t1.first > t2.first; };
		std::priority_queue<std::pair<float, sprite_drawing>, std::vector<std::pair<float, sprite_drawing>>, decltype(cmp)> sorted_things(cmp);

		for (auto&& thing : curr_sector->thgs) {

			if (things_resolved.find(thing) != things_resolved.end()) continue;

			// check whether the thing is in current subsector
			if(tree.get_subsector(thing->get_position()) != &(*i)) continue;

			Vector pos = thing->get_position();
			float radius = thing->get_radius();

			Vector change = pos - player.get_position();
			Vector perp = { change.Y, -change.X };

			Vector start = pos - radius / perp.length() * perp;
			Vector end	 = pos + radius / perp.length() * perp;

			auto h_res = project_plane_horizontally(start, end, 0, screen_width, screen_height);
			auto v_res = project_plane_vertically(thing->get_head_level(), thing->get_foot_level(), std::get<0>(h_res), std::get<1>(h_res), screen_height);

			if (!std::get<10>(h_res)) continue;

			int x_s = std::get<4>(h_res);
			int x_e = std::get<5>(h_res);
			int txt_s = std::get<6>(h_res);
			int txt_e = std::get<7>(h_res);
			Vector rot_s = std::get<8>(h_res);
			Vector rot_e = std::get<9>(h_res);
			int y_top_e = std::get<2>(v_res);
			int y_top_s = std::get<0>(v_res);
			int y_bot_e = std::get<3>(v_res);
			int y_bot_s = std::get<1>(v_res);

			// store drawing for later use
			sprite_drawing drawing;
			drawing.texture = thing->get_texture(player.get_position());

			// from begin_x to end_x coordinates
			for (int x = std::get<2>(h_res); x != std::get<3>(h_res); ++x) {
			
				// next stripe if already drawed, thing is behind a wall
				if (x_vis[x]) continue;

				// head level
				int ya = interpolate(x, x_s, x_e, y_top_s, y_top_e);
				int cya = clamp(ya, y_top[x], y_bottom[x]);

				// foot level
				int yb = interpolate(x, x_s, x_e, y_bot_s, y_bot_e);
				int cyb = clamp(yb, y_top[x], y_bottom[x]);

				// perspective texture mapping
				int texture_x = static_cast<int>(txt_s * ((x_e - x) * rot_e.Y) + txt_e * ((x - x_s) * rot_s.Y)) / ((x_e - x)*rot_e.Y + (x - x_s)*rot_s.Y);

				int t_y1 = interpolate_txt_u(thing->get_head_level() - thing->get_foot_level(), 0, ya, yb, cya);
				int t_y2 = interpolate_txt_l(thing->get_head_level() - thing->get_foot_level(), 0, ya, yb, cyb);

				drawing.data.push_back({ x, cya, cyb, texture_x, t_y1, t_y2 });
			}

			// add visible sprite on stack
			if (!drawing.data.empty()) {
				sorted_things.push(make_pair(thing->get_distance(player.get_position()), drawing));
				things_resolved.insert(thing);
			}
		}

		// copy sorted thing drawings to common queue for walls and things
		while (!sorted_things.empty()) {
			drawings.push(sorted_things.top().second);
			sorted_things.pop();
		}

		//
		// Subsector drawing
		//

		for (auto&& segment : *i) {

			Side* curr_line_side = (segment.direction ? segment.line->right : segment.line->left);
 			Side* ngbr_line_side = (segment.direction ? segment.line->left  : segment.line->right);

			auto res = resolved_walls.find(segment.line);
			if (res != resolved_walls.end() && res->second == segment.direction) continue;

			auto h_res = project_plane_horizontally(segment.start, segment.end, curr_line_side->tx, screen_width, screen_height);
			auto v_res = project_plane_vertically(curr_sector->ch, curr_sector->fh, std::get<0>(h_res), std::get<1>(h_res), screen_height);

			if (!std::get<10>(h_res)) continue;

			int x_s = std::get<4>(h_res);
			int x_e = std::get<5>(h_res);
			int txt_s = std::get<6>(h_res);
			int txt_e = std::get<7>(h_res);
			Vector rot_s = std::get<8>(h_res);
			Vector rot_e = std::get<9>(h_res);
			int y_top_e = std::get<2>(v_res);
			int y_top_s = std::get<0>(v_res);
			int y_bot_e = std::get<3>(v_res);
			int y_bot_s = std::get<1>(v_res);
			int begin_x = std::get<2>(h_res);
			int end_x = std::get<3>(h_res);

			#ifdef Textured
			// clip begin_x and end_x to already undrawed space
			std::vector<int> intervals;
			bool in_interval = false;

			for (int x = begin_x; x != end_x; ++x) {
				if (x_vis[x]) {
					if (in_interval) { intervals.push_back(x); in_interval = false; }
				}
				else {
					if (!in_interval) { intervals.push_back(x); in_interval = true; }
				}
			}
			if (in_interval) intervals.push_back(end_x);

			if (intervals.empty()) continue;
			#endif

			bool mark_floor = true;
			bool mark_ceiling = true;
			// above view plane
			if (curr_sector->ch - player.get_eye_level() <= 0) mark_ceiling = false;
			// below view plane
			if (curr_sector->fh - player.get_eye_level() >= 0) mark_floor = false;

			int ny_top_e, ny_top_s, ny_bot_e, ny_bot_s;

			if (ngbr_line_side != nullptr) {

				auto n_v_res = project_plane_vertically(ngbr_line_side->sector->ch, ngbr_line_side->sector->fh, std::get<0>(h_res), std::get<1>(h_res), screen_height);
	
				ny_top_e = std::get<2>(n_v_res);
				ny_top_s = std::get<0>(n_v_res);
				ny_bot_e = std::get<3>(n_v_res);
				ny_bot_s = std::get<1>(n_v_res);
			}
					
			#ifdef Textured
			// search for plane with same floor or ceiling height and same textures
			plane_t* floor_plane = planes.find_plane(curr_sector->fh, curr_sector->ft);
			plane_t* ceiling_plane = planes.find_plane(curr_sector->ch, curr_sector->ct);

			// check if found plane (candidate for merging) does not overlap new plane vertically 
			// if overlaps, then create new plane, add it to planes and use it as current plane (c,f)
			for (auto i = intervals.begin(); i != intervals.end(); i+=2) {
				if (mark_ceiling) ceiling_plane = planes.check_plane(ceiling_plane, *i, *(i + 1) - 1, intervals.front(), intervals.back() -1);
				if (mark_floor)   floor_plane = planes.check_plane(floor_plane, *i, *(i + 1) - 1, intervals.front(), intervals.back() -1);
			}
			#endif

			// for possible transparent wall
			sprite_drawing drawing;
			drawing.texture = curr_line_side->middle;

			// from begin_x to end_x coordinates
			for (int x = begin_x; x != end_x; ++x) {

				if (x_vis[x]) { continue; }

				// ceiling level
				int ya = interpolate(x, x_s, x_e, y_top_s, y_top_e);
				int cya = clamp(ya, y_top[x], y_bottom[x]);

				// floor level
				int yb = interpolate(x, x_s, x_e, y_bot_s, y_bot_e);
				int cyb = clamp(yb, y_top[x], y_bottom[x]);

				#ifdef Textured

				if (mark_ceiling) {
					ceiling_plane->top[x] = y_top[x];
					ceiling_plane->bottom[x] = cya;
				}
				if (mark_floor) {
					floor_plane->top[x] = cyb;
					floor_plane->bottom[x] = y_bottom[x];
				}

				// perspective texture mapping
				int texture_x = segment.offset + (txt_s*((x_e - x)*rot_e.Y) + txt_e*((x - x_s)*rot_s.Y)) / ((x_e - x)*rot_e.Y + (x - x_s)*rot_s.Y);

				#else

				// draw ceiling
				vertical_line_c(x, y_top[x], cya, sf::Color(50, 50, 50), target, states);
				// draw floor
				vertical_line_c(x, cyb, y_bottom[x], sf::Color(200, 200, 200), target, states);

				#endif

				if (ngbr_line_side != nullptr) {

					int nya = interpolate(x, x_s, x_e, ny_top_s, ny_top_e);
					int cnya = clamp(nya, y_top[x], y_bottom[x]);

					int nyb = interpolate(x, x_s, x_e, ny_bot_s, ny_bot_e);
					int cnyb = clamp(nyb, y_top[x], y_bottom[x]);

					#ifdef Textured

					// upper wall
					int t_y1 = interpolate_txt_u(curr_line_side->sector->ch - ngbr_line_side->sector->ch, curr_line_side->ty, ya, nya, cya);
					int t_y2 = interpolate_txt_l(curr_line_side->sector->ch - ngbr_line_side->sector->ch, curr_line_side->ty, ya, nya, cnya);
					
					states.texture = curr_line_side->upper;
					if (cya < cnya) vertical_line(x, cya, cnya, texture_x, t_y1, t_y2, target, states);

					// bottom wall
					t_y1 = interpolate_txt_u(ngbr_line_side->sector->fh - curr_line_side->sector->fh, curr_line_side->ty, nyb, yb, cnyb);
					t_y2 = interpolate_txt_l(ngbr_line_side->sector->fh - curr_line_side->sector->fh, curr_line_side->ty, nyb, yb, cyb);
					
					states.texture = curr_line_side->lower;
					if (cyb > cnyb) vertical_line(x, cnyb, cyb, texture_x, t_y1, t_y2, target, states);

					#else

					// upper wall
					if (cya < cnya - 1) vertical_line_c(x, cya, cnya - 1, sf::Color(125, x == x_s || x == x_e ? 0 : 125, 125), target, states);
					// bottom wall
					if (cyb > cnyb - 1) vertical_line_c(x, cnyb + 1, cyb, sf::Color(125, x == x_s || x == x_e ? 0 : 125, 125), target, states);
					
					#endif

					// adjust free window for drawing
					y_top[x] =    clamp(max(cya, cnya), y_top[x], screen_height - 1);
					y_bottom[x] = clamp(min(cyb, cnyb), 0, y_bottom[x]);

					#ifdef Textured

					// this is used for simulating partly transparent walls
					if (curr_line_side->middle != nullptr) {

						int ceiling_diff = min(curr_line_side->sector->ch, ngbr_line_side->sector->ch);
						int floor_diff =   max(curr_line_side->sector->fh, ngbr_line_side->sector->fh);

						t_y1 = interpolate_txt_u(ceiling_diff - floor_diff, curr_line_side->ty, max(ya, nya), min(yb, nyb), max(cya, cnya));
						t_y2 = interpolate_txt_l(ceiling_diff - floor_diff, curr_line_side->ty, max(ya, nya), min(yb, nyb), min(cyb, cnyb));

						drawing.data.push_back({ x, max(cya, cnya), min(cyb, cnyb), texture_x, t_y1, t_y2 });
					}

					#endif
				}
				else {

					// mark stripe as already completed
					x_vis[x] = true;

					#ifdef Textured

					int t_y1 = interpolate_txt_u(curr_line_side->sector->ch - curr_line_side->sector->fh, curr_line_side->ty, ya, yb, cya);
					int t_y2 = interpolate_txt_l(curr_line_side->sector->ch - curr_line_side->sector->fh, curr_line_side->ty, ya, yb, cyb);

					// draw solid wall from ceiling to floor
					states.texture = curr_line_side->middle;
					vertical_line(x, cya, cyb, texture_x, t_y1, t_y2, target, states);

					#else
					vertical_line_c(x, cya, cyb, sf::Color(125, x == x_s || x == x_e ? 0 : 125, 125), target, states);
					#endif
				}
			}

			// add new transparent wall on stack
			if (!drawing.data.empty()) drawings.push(drawing);
			if (ngbr_line_side != nullptr) resolved_walls.insert({ segment.line, !segment.direction });	
		}

	} // all subsectors were resolved

	#ifdef Textured

	//
	// Render ceiling and floor textures
	//

	planes.draw(target, states, fov_, texture_quality_);

	#endif

	#ifdef Textured

	//
	// Drawing precomputed transparent walls and things (including enemies, etc)
	//

	while (!drawings.empty()) {

		states.texture = drawings.top().texture;
		// holds, x, top, bottom, texture x, texture top, texture bottom
		for (auto&& stripe : drawings.top().data) {
			vertical_line(stripe[0], stripe[1], stripe[2], stripe[3], stripe[4], stripe[5], target, states);
		}
		drawings.pop();
	}

	#endif

}

// returns { scale_s, scale_e, begin_x, end_x, x_s, x_e, texture_s, texture_e }
Scene::projection_h_res Scene::project_plane_horizontally(const Vector& start, const Vector& end, int texture_shift, int width, int height) const {

	// projection center definition
	Player& player = Player::instance();
	Vector player_pos = player.get_position();
	float psin = std::sinf(player.get_direction() * PI_180);
	float pcos = std::cosf(player.get_direction() * PI_180);

	// view frustum
	const static float nearz = 0.001f, farz = 7500.0f;
	const static float nearside = 0.0001f, farside = 19000.0f;

	// player's fov
	const static float fov = std::tanf(fov_ * PI_180 / 2) / 2.0f;
	const static float h_fov = fov * width;			   // = 1/tg(fov/2) * width /2
	const static float v_fov = width / height * h_fov; // = aspect_ratio * 1/tg(fov/2) * width /2

	//
	// Projection computation
	//

	Vector p_e_v = end   - player_pos;
	Vector p_s_v = start - player_pos;

	Vector rot_e = { p_e_v.X * pcos - p_e_v.Y * psin,
		p_e_v.Y * pcos + p_e_v.X * psin };
	Vector rot_s = { p_s_v.X * pcos - p_s_v.Y * psin,
		p_s_v.Y * pcos + p_s_v.X * psin };

	// there is no part of the line in front of the player
	if (rot_e.Y <= 0 && rot_s.Y <= 0) return{ Vector{ 0,0 }, Vector{ 0,0 }, 0, 0, 0, 0, 0, 0, Vector{ 0,0 }, Vector{ 0,0 }, false };

	int plane_length = (int)((end - start).length() * texture_quality_);
	int texture_s = texture_shift;
	int texture_e = plane_length + texture_s;

	// the line is particulary visible, clipping needed
	if (rot_e.Y <= 0 || rot_s.Y <= 0) {

		// Find an intersection between the wall and the approximate edges of player's view
		Vector i1 = intersection(rot_e, rot_s, Vector{ -nearside, nearz }, Vector{ -farside, farz });
		Vector i2 = intersection(rot_e, rot_s, Vector{ nearside, nearz }, Vector{ farside, farz });

		if (rot_e.Y < nearz) {
			if ((rot_e.X < 0 || i2.Y < 0) && i1.Y > 0 && is_usable(i1.X, rot_e.X, rot_s.X)) {
				// update texture ending (or right) offset
				texture_e -= (float)plane_length * (1.0f - (i1.X - rot_s.X) / (rot_e.X - rot_s.X));
				// update end point
				rot_e.X = i1.X; rot_e.Y = i1.Y;
			}
			else
				if (i2.Y > 0 && is_usable(i2.X, rot_e.X, rot_s.X)) {
					// update texture ending (or right) offset
					texture_e -= (float)plane_length * (1.0f - (i2.X - rot_s.X) / (rot_e.X - rot_s.X));
					// update end point
					rot_e.X = i2.X; rot_e.Y = i2.Y;
				}
				else return{ Vector{ 0,0 }, Vector{ 0,0 }, 0, 0, 0, 0, 0, 0, Vector{ 0,0 }, Vector{ 0,0 }, false };
		}

		if (rot_s.Y < nearz) {
			if ((rot_s.X < 0 || i2.Y < 0) && i1.Y > 0 && is_usable(i1.X, rot_e.X, rot_s.X)) {
				// update texture starting (or left) offset
				texture_s += (float)plane_length * (1.0f - (rot_e.X - i1.X) / (rot_e.X - rot_s.X));
				// update start point
				rot_s.X = i1.X; rot_s.Y = i1.Y;
			}
			else
				if (i2.Y > 0 && is_usable(i2.X, rot_e.X, rot_s.X)) {
					// update texture starting (or left) offset
					texture_s += (float)plane_length * (1.0f - (rot_e.X - i1.X) / (rot_e.X - rot_s.X));
					// update start point
					rot_s.X = i2.X; rot_s.Y = i2.Y;
				}
				else return{ Vector{ 0,0 }, Vector{ 0,0 }, 0, 0, 0, 0, 0, 0, Vector{ 0,0 }, Vector{ 0,0 }, false };
		}
	}

	// projection of the wall onto camera plane
	Vector scale_e = { h_fov / rot_e.Y, v_fov / rot_e.Y };
	Vector scale_s = { h_fov / rot_s.Y, v_fov / rot_s.Y };

	int x_e = (int)(width / 2 + rot_e.X * scale_e.X);
	int x_s = (int)(width / 2 + rot_s.X * scale_s.X);

	// point on the right side is now endpoint
	if (x_e < x_s) {
		std::swap(x_e, x_s);
		std::swap(scale_e, scale_s);
		std::swap(texture_e, texture_s);
	}

	// projected wall is not in the view
	if (x_s > width || x_e < 0) return { Vector{0,0}, Vector{0,0}, 0, 0, 0, 0, 0, 0, Vector{ 0,0 }, Vector{ 0,0 }, false };

	int begin_x = std::max(x_s, 0);
	int end_x = std::min(x_e, width);

	return { scale_s, scale_e, begin_x, end_x, x_s, x_e, texture_s, texture_e, rot_s, rot_e, true };

}

// returns { y_top_s, y_bot_s,y_top_e,y_bot_e }
Scene::projection_v_res Scene::project_plane_vertically(int top, int bottom, const Vector& scale_s, const Vector& scale_e, int height) const {

	Player& player = Player::instance();

	float y_ceil = top - player.get_eye_level();
	float y_floor = bottom - player.get_eye_level();

	int y_top_e = (int)(height / 2 - y_ceil  * scale_e.Y),
		y_bot_e = (int)(height / 2 - y_floor * scale_e.Y);

	int y_top_s = (int)(height / 2 - y_ceil  * scale_s.Y),
		y_bot_s = (int)(height / 2 - y_floor * scale_s.Y);

	return { y_top_s, y_bot_s, y_top_e, y_bot_e };
}