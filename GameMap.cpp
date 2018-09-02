#include "GameMap.h"

using namespace std;
		
// Parse file into map structures
// returns false in case of wrong format
bool GameMap::load(const std::string & filename) {

	std::ifstream input(filename);
	if (!input.good()) return false;

	string line;
	int count;

	// wall textures
	{

		input >> line; if (line != "#textures") return false;

		sf::Image image;
		string textures_file;
		size_t number_of_textures_x, number_of_textures_y, size_of_texture;

		input >> textures_file >> number_of_textures_x >> number_of_textures_y >> size_of_texture;		

		if (!image.loadFromFile(textures_file)) return false;

		for (size_t i = 0; i < number_of_textures_y; i++)
		for (size_t j = 0; j < number_of_textures_x; j++) {
			auto t = std::make_unique<sf::Texture>();
			t->loadFromImage(image, sf::IntRect(size_of_texture* j, size_of_texture * i, size_of_texture, size_of_texture));
			t->setRepeated(true);
			textures_.push_back(move(t));
		}

		if (!input.good()) return false;

	}

	// sectors
	{

		input >> line; if (line != "#sectors") return false;
		input >> count;

		bool doors;
		int floor_h, ceiling_h, floor_texture, ceiling_texture;

		while (count--) {

			if (!input.good()) return false;

			input >> floor_h >> ceiling_h >> floor_texture >> ceiling_texture >> doors;

			if (doors) {
				auto ptr = std::make_unique<Door>(floor_h,
				                                  ceiling_h,
				                                  floor_texture < 1 ? nullptr : textures_[floor_texture - 1].get(),
				                                  ceiling_texture < 1 ? nullptr : textures_[ceiling_texture - 1].get());

				doors_.push_back(ptr.get());
				sectors_.push_back(move(ptr));	
			}
			else {
				sectors_.push_back(std::make_unique<Sector>(floor_h,
				                                            ceiling_h,
				                                            floor_texture < 1 ? nullptr : textures_[floor_texture - 1].get(),
				                                            ceiling_texture < 1 ? nullptr : textures_[ceiling_texture - 1].get())
				);
			}		
		}

		if (!input.good()) return false;

	}

	// vertices
	Vector min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() },
		   max = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
	{
	
		input >> line; if (line != "#vertices") return false;
		input >> count;

		float x, y;

		while (count--) {

			if (!input.good()) return false;

			input >> x >> y;
			if (x < min.X) min.X = x;
			if (x > max.X) max.X = x;
			if (y < min.Y) min.Y = y;
			if (y > max.Y) max.Y = y;
			vertices_.push_back(Vector{ x, y });
		}

		if (!input.good()) return false;

	}

	// sides
	{

		input >> line; if (line != "#sides") return false;
		input >> count;

		size_t sector_id, texture_x, texture_y, middle_texture, lower_texture, upper_texture;

		while (count--) {

			if (!input.good()) return false;

			input >> sector_id >> texture_x >> texture_y >> middle_texture >> lower_texture >> upper_texture;

			if (sector_id - 1 >= sectors_.size()) return false;

			sides_.push_back(Side{ (lower_texture  < 1 ? nullptr : textures_[lower_texture  - 1].get()),
								   (middle_texture < 1 ? nullptr : textures_[middle_texture - 1].get()),
								   (upper_texture  < 1 ? nullptr : textures_[upper_texture  - 1].get()),
								    texture_x,
								    texture_y,
									sectors_[sector_id - 1].get() });
		}

		if (!input.good()) return false;

		// attach inner sides to doors
		for (auto&& side : sides_) {

			if (side.middle != nullptr && side.sector->is_door()) {
				Door* d = dynamic_cast<Door*>(side.sector);
				d->sides.push_back(&side);
			}
		}

	}
	
	// lines
	{

		input >> line; if (line != "#lines") return false;
		input >> count;

		size_t start, end, left, right;

		while (count--) {

			if (!input.good()) return false;

			input >> start >> end >> left >> right;

			if (start -1 >= vertices_.size() ||
				end   -1 >= vertices_.size() || 
				left  -1 >= sides_.size() ||
				(right != 0 && right -1 >= sides_.size()) ) 
				return false;

			lines_.push_back(Line{ &vertices_[start -1], &vertices_[end -1], &sides_[left -1], (right ? &sides_[right - 1] : nullptr) });
		
			// attach outer sides to doors
			if (right) {

				if (sides_[left - 1].sector->is_door()) {
					Door* d = dynamic_cast<Door*>(sides_[left - 1].sector);
					d->sides.push_back(&sides_[right - 1]);
				}

				if (sides_[right - 1].sector->is_door()) {
					Door* d = dynamic_cast<Door*>(sides_[right - 1].sector);
					d->sides.push_back(&sides_[left - 1]);
				}
			}
		}

		if (!input.good()) return false;
	}


	// player definition
	{

		input >> line; if (line != "#player") return false;

		float x_pos, y_pos, angle;
		int sector_id;

		if (!input.good()) return false;

		input >> x_pos >> y_pos >> angle >> sector_id;

		Player::instance().set({ x_pos, y_pos }, angle, sectors_[sector_id - 1].get(), this);
		sectors_[sector_id - 1]->thgs.push_back(&Player::instance());

		if (!input.good()) return false;
	}


	// things
	{

		input >> line; if (line != "#things") return false;
		input >> count;

		float x_pos, y_pos;
		int sector_id;
		std::string name;

		while (count--) {

			if (!input.good()) return false;

			input >> x_pos >> y_pos >> sector_id >> name;

			things_.push_back(Thing::create(name, { x_pos, y_pos, (float)sectors_[sector_id - 1]->fh }, sectors_[sector_id - 1].get()));
			sectors_[sector_id - 1]->thgs.push_back(things_.back().get());
		}

		if (!input.good()) return false;
	}


	// enemies
	{

		input >> line; if (line != "#enemies") return false;
		input >> count;

		float x_pos, y_pos;
		size_t sector_id;
		std::string name;

		while (count--) {

			if (!input.good()) return false;

			input >> x_pos >> y_pos >> sector_id >> name;

			things_.push_back(Enemy::create(name, { x_pos, y_pos, (float)sectors_[sector_id - 1]->fh }, sectors_[sector_id - 1].get(), this));
			sectors_[sector_id - 1]->thgs.push_back(things_.back().get());
		}

	}

	bsp_tree_  = BSPTree::build(lines_);
	block_map_ = BlockMap::build(lines_, min, max);

	return true;

}


void GameMap::check_block_doors(const lines_p_t* block, const Vector& pos) {

	for (auto&& line : *block) {

		if (line->right == nullptr ||
			(!line->right->sector->is_door() && 
			 !line->left->sector->is_door()))
			continue;

		float distance = (0.5f * (*line->end + *line->start) - pos).length();

		if (distance > Door::door_open_distance) continue;
		
		Door* d;

		if (line->right->sector->is_door()) {
			d = dynamic_cast<Door*>(line->right->sector);
		}

		if (line->left->sector->is_door()) {
			d = dynamic_cast<Door*>(line->left->sector);
		}

		if (d->s == Door::moving_down && d->ch == d->open_ch) d->s = Door::open;
		if (d->s == Door::moving_down || d->s == Door::closed) d->s = Door::moving_up;
	}

}

void GameMap::update(float seconds_elapsed){

	Vector player_pos = Player::instance().get_position();

	// check whether there are some doors to be opened
	for (int i = -1; i <= 1; i++)
	for (int j = -1; j <= 1; j++) {

		auto block = block_map_.get_block(player_pos +
			(float)i * Vector{ static_cast<float>(block_map_.block_size), 0.0f } +
			(float)j * Vector{ 0.0f, static_cast<float>(block_map_.block_size) });

		if (block == nullptr) continue;
		check_block_doors(block, player_pos);
	}

	// update doors state
	for (auto&& d : doors_) {
		
		int change;

		switch (d->s) {
		case Door::closed: continue;
		case Door::open: 
			d->seconds_opened += seconds_elapsed;
			if (d->seconds_opened > Door::door_open_time && d->thgs.empty()) {
				d->seconds_opened = 0;
				d->s = Door::moving_down;
			}
			break;
		case Door::moving_up: 
			Sounds::global().play(Sounds::door, 100);
			change = static_cast<int>(Door::door_speed * seconds_elapsed);
			d->ch += change;
			if (d->ch > d->open_ch) {
				change += d->open_ch - d->ch;
				d->ch = d->open_ch;
				d->s = Door::open;
				Sounds::global().stop(Sounds::door);
				Sounds::global().play(Sounds::door_end, 100);
			}
			for (auto&& side : d->sides) {
				if (side->middle == nullptr) side->ty += change;
				else side->ty -= change;
			}
			break;
		case Door::moving_down: 
			Sounds::global().play(Sounds::door, 100);
			if (!d->thgs.empty()) {
				d->s = Door::moving_up;
				continue;
			}
			change = static_cast<int>(Door::door_speed * seconds_elapsed);
			d->ch -= change;
			if (d->ch < d->fh) {
				change -= d->fh - d->ch;
				d->ch = d->fh;
				d->s = Door::closed;
				Sounds::global().stop(Sounds::door);
				Sounds::global().play(Sounds::door_end, 100);
			}
			for (auto&& side : d->sides) {
				if (side->middle == nullptr) side->ty -= change;
				else side->ty += change;
			}
			break;
		}

	}

	// remove dead enemies
	things_.erase(std::remove_if(things_.begin(), things_.end(),
		
		[](std::unique_ptr<Thing> const& p) {
			Person * person = dynamic_cast<Person*>(p.get());
			if (person == nullptr) return false;
			else {		
				if (person->is_alive()) return false;
				else {
					Sounds::global().play(Sounds::death_enemy, 100);
					auto& sec_things = person->get_sec()->thgs;
					sec_things.erase(std::remove(sec_things.begin(), sec_things.end(), person), sec_things.end());
					return true;
				}		
			};
		}

	), things_.end());

	// update enemies states (moving and shooting)
	for (auto it = things_.begin(); it != things_.end(); ++it) {

		Enemy * enemy = dynamic_cast<Enemy*>(it->get());
		if (enemy == nullptr) continue;
		if (enemy->is_activated()) enemy->make_action(seconds_elapsed);
		else {
			if (enemy->get_sec() == Player::instance().get_sec() || 
				enemy->get_distance(Player::instance().get_position()) <= Enemy::activation_range ||
				check_los(Player::instance(), *enemy)) {
				enemy->activate(&Player::instance());
			}
		}
	}

}

bool GameMap::check_los(float from_eyes, Vector from, Vector to, float to_head, float to_foot) const {

	float top_slope = to_head - from_eyes;
	float bottom_slope = to_foot - from_eyes;
	StraightLine ray(from, to);

	auto& bsp_root = bsp_tree_.get_root();
	return los_crosses_bsp_node(bsp_root, ray, from_eyes, bottom_slope, top_slope);

}

bool GameMap::los_crosses_bsp_node(const BSPTree::node& node, StraightLine ray, float start_z, float bottom_slope, float top_slope) const {

	if (node.leaf) {
		return los_crosses_subsector(node.lines, ray, start_z, bottom_slope, top_slope);
	}

	int side = node.split.determine_pnt_side(ray.point);

	// we would like to check either "touch"
	if (side == -1) side = 0;

	// go through the tree to the ray's start-point subsector
	if (!los_crosses_bsp_node((side == 0 ? *node.left.get() : *node.right.get()), ray, start_z, bottom_slope, top_slope)) return false;

	// the line does not touch the other node's child
	if (side == node.split.determine_pnt_side(ray.point + ray.diff)) return true;

	// go through the tree to the ray's end-point subsector
	return los_crosses_bsp_node((side == 0 ? *node.right.get() : *node.left.get()), ray, start_z, bottom_slope, top_slope);
}

bool GameMap::los_crosses_subsector(const segments_t& sub_sector, StraightLine ray, float start_z, float bottom_slope, float top_slope) const {

	for (auto&& seg : sub_sector) {

		auto l = seg.line;
		int side_s, side_e;

		// test ray against wall
		side_s = ray.determine_pnt_side(*l->start);
		side_e = ray.determine_pnt_side(*l->end);

		if (side_s == side_e) continue;

		// test wall against ray
		StraightLine div(*l->start, *l->end);
		side_s = div.determine_pnt_side(ray.point);
		side_e = div.determine_pnt_side(ray.point + ray.diff);

		if (side_s == side_e) continue;

		// one sided wall stops the ray
		if (l->right == nullptr) return false;

		auto front = (seg.direction ? l->right->sector : l->left->sector);
		auto back = (seg.direction ? l->left->sector : l->right->sector);

		// ray cannot be blocked neither by floor nor by ceiling
		if (front->fh == back->fh && front->ch == back->ch) continue;

		// ceiling height difference
		int open_top = std::min(back->ch, front->ch);

		// ceiling height difference
		int open_bottom = std::max(back->fh, front->fh);

		// maybe equals is enough? closed doors only
		if (open_bottom >= open_top) return false;

		float i = StraightLine::line_intersection(ray, div);

		if (front->fh != back->fh) {
			bottom_slope = std::max((open_bottom - start_z) / i, bottom_slope);
		}
		if (front->ch != back->ch) {
			top_slope = std::min((open_top - start_z) / i, top_slope);
		}

		// any part of the ray does not cross the open hole
		if (top_slope <= bottom_slope) return false;

	}

	return true;
}