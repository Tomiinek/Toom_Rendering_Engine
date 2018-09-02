#include "Planes.h"

void Planes::draw_horizontal_line(int y, int x1, int x2, int t_x1, int t_y1, int t_x2, int t_y2, float quality, sf::RenderTarget & target, sf::RenderStates states) const {

	sf::Vertex l[2] =
	{
		sf::Vertex(sf::Vector2f((float)x1, (float)y), sf::Vector2f(t_x1 * quality, t_y1 * quality)),
		sf::Vertex(sf::Vector2f((float)x2, (float)y), sf::Vector2f(t_x2 * quality, t_y2 * quality))
	};

	target.draw(l, 2, sf::Lines, states);
}

void Planes::draw(sf::RenderTarget & target, sf::RenderStates states, float fov, float quality) const {

	float psin = std::sinf(Player::instance().get_direction() * PI_180);
	float pcos = std::cosf(Player::instance().get_direction() * PI_180);

	// player's fov
	const static float fov_tmp = std::tanf(fov * PI_180 / 2) / 2.0f;
	const static float h_fov = fov_tmp * max_width_;			 // = 1/tg(fov/2) * width /2
	const static float v_fov = max_width_ / max_height_ * h_fov; // = aspect_ratio * 1/tg(fov/2) * width /2

	for (auto const& pl : planes) {

		// invalid plane
		if (pl->minx > pl->maxx) continue;

		// span storage
		std::vector<int> span_s(max_height_, 0);

		states.texture = pl->texture;

		for (int x = pl->minx; x <= pl->maxx + 1; x++) {

			int t1 = (x - 1 < 0 ? max_height_ : pl->top[x - 1]);
			int t2 = (x >= max_width_ ? max_height_ : pl->top[x]);
			int b1 = (x - 1 < 0 ? 0 : pl->bottom[x - 1]);
			int b2 = (x >= max_width_ ? 0 : pl->bottom[x]);

			while (t1 < t2 && t1 <= b1) {

				// Reverse projection
				float y_s = v_fov * (pl->height - Player::instance().get_eye_level()) / (max_height_ / 2 - t1);
				float x_s = y_s * (span_s[t1] - max_width_ / 2) / h_fov;
				float real_x_s = Player::instance().get_position().X + x_s * pcos    + y_s * psin;
				float real_y_s = Player::instance().get_position().Y + x_s * (-psin) + y_s * pcos;

				float y_e = v_fov * (pl->height - Player::instance().get_eye_level()) / (max_height_ / 2 - t1);
				float x_e = y_e * (x - 1 - max_width_ / 2) / h_fov;
				float real_x_e = Player::instance().get_position().X + x_e * pcos + y_e * psin;
				float real_y_e = Player::instance().get_position().Y + x_e * (-psin) + y_e * pcos;

				draw_horizontal_line(t1, span_s[t1], x - 1, (int)real_x_s, (int)real_y_s, (int)real_x_e, (int)real_y_e, quality, target, states);
				++t1;
			}

			while (b1 > b2 && b1 >= t1) {

				// Reverse projection
				float y_s = v_fov * (pl->height - Player::instance().get_eye_level()) / (max_height_ / 2 - b1);
				float x_s = y_s * (span_s[b1] - max_width_ / 2) / h_fov;
				float real_x_s = Player::instance().get_position().X + x_s * pcos + y_s * psin;
				float real_y_s = Player::instance().get_position().Y + x_s * (-psin) + y_s * pcos;

				float y_e = v_fov * (pl->height - Player::instance().get_eye_level()) / (max_height_ / 2 - b1);
				float x_e = y_e * (x - 1 - max_width_ / 2) / h_fov;
				float real_x_e = Player::instance().get_position().X + x_e * pcos + y_e * psin;
				float real_y_e = Player::instance().get_position().Y + x_e * (-psin) + y_e * pcos;

				draw_horizontal_line(b1, span_s[b1], x - 1, (int)real_x_s, (int)real_y_s, (int)real_x_e, (int)real_y_e, quality, target, states);
				b1--;
			}

			while (t2 < t1 && t2 <= b2) {
				span_s[t2] = x - 1;
				t2++;
			} 

			while (b2 > b1 && b2 >= t2) {
				span_s[b2] = x - 1;
				b2--;
			}
		}
	}
}

plane_t* Planes::find_plane(size_t height, sf::Texture* txt) {

	for (auto it = planes.begin(); it != planes.end(); ++it) {
		if (height == (*it)->height	&& txt == (*it)->texture) return (*it).get();
	}

	plane_p new_plane = std::make_unique<plane_t>(max_width_, std::numeric_limits<short>::max());

	new_plane->height = height;
	new_plane->texture = txt;
	new_plane->minx = max_width_ +1;
	new_plane->maxx = -1;

	planes.push_back(move(new_plane));
	return planes.back().get();
}

plane_t* Planes::check_plane(plane_t* pl, int start, int stop, int global_min, int global_max) {

	int	overlap_start, overlap_stop;
	int	new_start, new_stop;

	if (start < pl->minx) {
		overlap_start = pl->minx +1;
		new_start = start;
	}
	else {
		new_start = pl->minx;
		overlap_start = start;
	}

	if (stop > pl->maxx) {
		overlap_stop = pl->maxx +1;
		new_stop = stop;
	}
	else {
		new_stop = pl->maxx;
		overlap_stop = stop;
	}

	// Simple intersection test of two intervals is not enough, multiple intervals
	// can be stored in a single plane, i.e. XXXXX__NNN__XXXXXX, where X denotes 
	// existing plane and N denotes checking range from start to stop
	int x;
	for (x = overlap_start; x <= overlap_stop; x++) {
		if (pl->top[x] != std::numeric_limits<short>::max()) break;
	}

	// there is not an overlap, use the same plane
	if (x > overlap_stop) {
		pl->minx = new_start;
		pl->maxx = new_stop;
		return pl;
	}

	// make a new plane
	plane_p new_plane = std::make_unique<plane_t>(max_width_, std::numeric_limits<short>::max());
	new_plane->height = pl->height;
	new_plane->texture = pl->texture;
	new_plane->minx = global_min;
	new_plane->maxx = global_max;

	planes.push_back(move(new_plane));
	return planes.back().get();

}