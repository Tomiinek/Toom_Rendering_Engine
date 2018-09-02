#include "Enemy.h"

void Enemy::make_action(float seconds_ellapsed) {

	// Simple behavior, after activation:
	// (0*) if enemy hits wall, it chooses random direction
	//     and move this way for min_move_time
	// (0) if min_move_time elapsed
	// (1) if target is in weapon range, roll dice:
	//	   enemy is either shooting for state_change_time
	//     or moving in random direction for min_move_time
	// (2) if target is not in weapon range, choose random
	//     direction and move this way for min_move_time
	// (*) if hits wall while moving, chooses another direction
	//     and tries moving again

	weapon_.update(static_cast<int>(seconds_ellapsed * 1000.0f));

	if (move_time_ > 0) {

		move_to(seconds_ellapsed, angle_, true);
		move_time_ -= seconds_ellapsed;

		return;
	}
	else move_time_ = 0;

	bool in_range = weapon_.in_range(pos_, target_->get_position());
	float threshold = (float)rand() / (RAND_MAX);

	if (in_range &&
		change_time_ <= 0 && 
		move_time_ == 0 &&
		threshold > 0.5f) {

		shooting_ = true;
		move_time_ = 0;
		change_time_ = state_change_time;  
	} 
	else {

		if(change_time_ > 0) change_time_ -= seconds_ellapsed;
		else { 
			
			change_time_ = 0; 
			shooting_ = false; 

			float rand_angle = (float)std::rand() / (RAND_MAX / 360.0f);
			if (rand_angle < 0)   rand_angle += 360;
			if (rand_angle >= 360) rand_angle -= 360;

			angle_ = rand_angle;
			move_time_ = min_move_time / 2.0f;
		
			return;
		}

	}

	if (!shooting_) {

		if (blocked_) {

			blocked_ = false; 
		
			float rand_angle = angle_ + 90 + (float)rand() / (RAND_MAX / 180.0f);
			if (rand_angle < 0)   rand_angle += 360;
			if (rand_angle >= 360) rand_angle -= 360;

			angle_ = rand_angle;

			move_to(seconds_ellapsed, angle_, true);
			move_time_ = min_move_time;

		}
		else {

			Vector dir = target_->get_position() - pos_;
			angle_ = atan2(dir.X, dir.Y) / PI_180;

			move_to(seconds_ellapsed, angle_, true);

		}

	}
	else {

		Vector dir = target_->get_position() - pos_;
		angle_ = atan2(dir.X, dir.Y) / PI_180;

		move_to(seconds_ellapsed, angle_, false);
	
		// gun variation
		angle_ += 2.5f * (float)std::rand() / RAND_MAX;
		weapon_.fire(*this);

	}

}

void Enemy::move_to(float seconds_elapsed, float dir, bool pushing) {

	float sin = std::sinf(dir * PI_180);
	float cos = std::cosf(dir * PI_180);

	blocked_ = !Person::move_to(seconds_elapsed, { sin, cos }, pushing);
}

void Enemy::move_to(float seconds_elapsed, Vector dir, bool pushing) {

	blocked_ = !Person::move_to(seconds_elapsed, dir, pushing);
}
