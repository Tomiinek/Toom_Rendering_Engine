#include "Person.h"
#include "GameMap.h"

bool Person::move_to(float seconds_elapsed, const Vector& direction, bool pushing) {
	
	// New position computation

	// time threshold dependent on max_speed and BlockMap::block_size
	// we need to prevent crossing more than two map blocks in a step
	if (seconds_elapsed > 1) return false;

	Vector move_vector{0,0};

	if (direction.length() != 0) move_vector = Vector::normalize(direction);
	else pushing = false;

	Vector new_pos = pos_ + seconds_elapsed * Vector{ velocity_.X, velocity_.Y };
	float new_z = Z + seconds_elapsed * velocity_.Z;

	float new_x = 0, new_y = 0;

	if (!pushing && !(velocity_.X == 0 && velocity_.Y == 0)) {

		Vector move_dir = Vector::normalize(Vector{ velocity_.X, velocity_.Y });
		new_x = velocity_.X - move_dir.X * acceleration_ * seconds_elapsed;
		new_y = velocity_.Y - move_dir.Y * acceleration_ * seconds_elapsed;

		if ((velocity_.X > 0 && new_x < 0) || (velocity_.X < 0 && new_x > 0)) { new_x = 0; }
		if ((velocity_.Y > 0 && new_y < 0) || (velocity_.Y < 0 && new_y > 0)) { new_y = 0; }

	}
	else if (pushing) {

		new_x = velocity_.X + move_vector.X * acceleration_ * seconds_elapsed;
		new_y = velocity_.Y + move_vector.Y * acceleration_ * seconds_elapsed;

		float l = Vector{ new_x, new_y }.length() / get_max_speed();
		if (l > 1) { new_x /= l; new_y /= l; }	

	}

	velocity_.X = ( std::isinf(new_x) ? 0 : new_x );
	velocity_.Y = ( std::isinf(new_y) ? 0 : new_y );


	// Handling z-coordinate

	if (velocity_.Z < 0) {
		if (new_z < sector_->fh) {
			Z = (float)sector_->fh;
			velocity_.Z = 0;
			falling_ = false;
		} else Z = new_z;
	}

	if (velocity_.Z > 0) {
		if (new_z + get_head_level() - Z > sector_->ch) {
			velocity_.Z = 0;
			falling_ = false;
		} else {
			// branch needed because of the "step" lower -> higher floor movement
			if (Z < sector_->fh && sector_->fh <= new_z) {
				Z = (float)sector_->fh;
				velocity_.Z = 0;
				falling_ = false;
			} else Z = new_z;
		}
	}

	if (Z != sector_->fh) { velocity_.Z -= gravity_a * seconds_elapsed; falling_ = true; }


	// Collision detection

	new_pos.X = std::isinf(new_pos.X) ? pos_.X : new_pos.X;
	new_pos.Y = std::isinf(new_pos.Y) ? pos_.Y : new_pos.Y;

	auto start_block = map_->get_block_map().get_block(pos_);
	auto end_block = map_->get_block_map().get_block(new_pos);

	Vector pos_memory = new_pos;
	bool collision = true;

	if (start_block == nullptr) new_pos = pos_;
	collision_detection(start_block, new_pos);
	
	if (&(*start_block) != &(*end_block)) {
		if (end_block == nullptr) new_pos = pos_;
		collision_detection(end_block, new_pos);
	}

	if (pos_memory != new_pos) collision = false;


	// Update position
	pos_ = new_pos;
	return collision;
}

void Person::collision_detection(const lines_p_t * block, Vector& new_pos) {

	Sector* new_sector;

	collision:;

	new_sector = nullptr;

	if ((new_pos - pos_).length() < 0.1) { new_pos = pos_;  return; }

	new_pos.X = std::isinf(new_pos.X) ? pos_.X : new_pos.X;
	new_pos.Y = std::isinf(new_pos.Y) ? pos_.Y : new_pos.Y;

	// detect collision with objects in certain map block
	for (auto it = block->begin(); it != block->end(); ++it) {

		// line has opposite orientation and is not two-sided, it cannot affect anything
		StraightLine line(*(*it)->start, *(*it)->end);
		int line_side = line.determine_pnt_side(pos_);

		if (line_side == 1 && (*it)->right == nullptr) continue;

		if ((*it)->in_collision(pos_, new_pos)) {

			if ((*it)->right != nullptr) {

				new_sector = (&(*(*it)->left->sector) == &(*sector_) ? (*it)->right->sector : (*it)->left->sector);

				int hole_low = std::max(sector_->fh, new_sector->fh);
				int hole_high = std::min(sector_->ch, new_sector->ch);
				int step = new_sector->fh - sector_->fh;

				// the hole can be entered
				if (hole_low <= get_foot_level() + (falling_ ? 0 : get_step_height()) && 
					hole_high >= get_head_level() + (step <= 0 || falling_ ? 0 : step))
					continue;
			}

			Vector n = Vector::normalize(*(*it)->end - *(*it)->start);
			Vector projection = (new_pos - pos_) * n * n;
			
			if ((new_pos - pos_ - projection).length() < 0.0001f) new_pos = pos_;
			else new_pos = pos_ + projection;

			// reset loop because we affected new_pos
			// go to start of this whole function
			goto collision;

		}
	}

	// detect collision with things in current sector and then in new sector
	for (auto it = sector_->thgs.begin(); it != sector_->thgs.end(); ++it) {

		if ((*it) == this) continue;

		if ((*it)->in_collision(*this, new_pos)) {

			Vector center_diff = (*it)->get_position() - pos_;
			Vector n = Vector::normalize({ center_diff.Y, -center_diff.X });
			Vector projection = (new_pos - pos_) * n * n;

			if ((new_pos - pos_ - projection).length() < 0.0001f) new_pos = pos_;
			else new_pos = pos_ + projection;

			// reset all loops because we affected new_pos and new_sector can be changed
			// go to start of this whole function
			goto collision;

		}
	}

	if (new_sector == nullptr) return;

	for (auto it = new_sector->thgs.begin(); it != new_sector->thgs.end(); ++it) {

		if ((*it) == this) continue;

		if ((*it)->in_collision(*this, new_pos)) {

			Vector center_diff = (*it)->get_position() - pos_;		
			Vector n = Vector::normalize({ center_diff.Y, -center_diff.X });
			Vector projection = (new_pos - pos_) * n * n;

			if ((new_pos - pos_ - projection).length() < 0.01f) new_pos = pos_;
			else new_pos = pos_ + projection;

			// reset all loops because we affected new_pos and new_sector can be changed
			// go to start of this whole function
			goto collision;

		}
	}

	// crossing to another unreachable sector when falling
	if (falling_ && new_sector->fh > Z) { new_pos = pos_; return; }

	// person is in the air and its vertical velocity 
	// must not be affected by crossing sectors

	int step = new_sector->fh - sector_->fh;
	if (step > 0 && Z == sector_->fh) {
		velocity_.Z = 1.5f * sqrt(2.0f * gravity_a * step);
		falling_ = true;
	}
	if (step < 0 && Z <= sector_->fh) {
		velocity_.Z = -300;
		falling_ = true;
	}

	// Update sectors list of things
	for (auto i = sector_->thgs.begin(); i != sector_->thgs.end(); ++i) {	
		if (*i == this) { sector_->thgs.erase(i); break; }
	}

	new_sector->thgs.push_back(this);
	sector_ = new_sector;
}
