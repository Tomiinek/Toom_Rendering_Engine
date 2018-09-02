#include "Weapon.h"
#include "Player.h"
#include "Person.h"
#include "GameMap.h"


void Weapon::fire(const Enemy& shooter) {

	if (cooldown_ != 0) return;
	cooldown_ = max_cooldown_;
	Sounds::global().play(Sounds::fire_enemy, 100);

	bool valid;
	float int_param;

	std::tie(int_param, valid) = shoot_thing(Player::instance(), shooter);
	if (valid) {	
		Player::instance().damage(int_param * damage_);
	}
	else {
		Sounds::global().play(Sounds::bullet_wall, 100);
	}
}

void Weapon::fire(const Player& shooter) {

	if (cooldown_ != 0) return;
	cooldown_ = max_cooldown_;
	Sounds::global().play(Sounds::fire_player, 100);

	std::vector<std::pair<Person*, float>> shooted_things;
	bool valid;
	float int_param;

	for (auto&& thing : shooter.get_map()->get_things()) {

		std::tie(int_param, valid) = shoot_thing(*thing, shooter);

		if (valid) {
			Enemy * enemy = dynamic_cast<Enemy*>(thing.get());
			if (enemy != nullptr) shooted_things.push_back({ enemy, int_param });
		}
	}

	Vector pos = shooter.get_position();

	//sort shooted_things
	std::sort(shooted_things.begin(), shooted_things.end(), [&](const std::pair<Person*, float>& lhs, const std::pair<Person*, float>& rhs) {
		return lhs.first->get_distance(pos) < rhs.first->get_distance(pos);
	});

	// damage nearest of them
	if (!shooted_things.empty()) shooted_things.front().first->damage(shooted_things.front().second * damage_);
}

std::tuple<float, bool> Weapon::shoot_thing(const Thing& thing, const Person& shooter) {

	float psin = std::sinf(shooter.get_direction() * PI_180);
	float pcos = std::cosf(shooter.get_direction() * PI_180);

	Vector pos = shooter.get_position();
	Vector dir = { psin, pcos };
	Vector perp = { dir.Y, -dir.X };

	// thing is out of range
	if ((thing.get_position() - pos).length() > range_) return { 0.0f, false };

	// same object
	if (&shooter == &thing) return { 0.0f, false };

	// bullet vs enemy plane intersection
	Vector change = thing.get_position() - pos;
	Vector perp_tng = { change.Y, -change.X };

	Vector start = thing.get_position() - thing.get_radius() / perp_tng.length() * perp_tng;
	Vector end = thing.get_position() + thing.get_radius() / perp_tng.length() * perp_tng;

	StraightLine ray(pos, pos + dir);

	float t = ray.ray_segment_intersection(start, end);

	if (t == -1) return { 0.0f, false };

	// thing is not reachable by bullet
	if (!shooter.get_map()->check_los( shooter.get_head_level(), pos,
										start + t * (end - start), 
										thing.get_head_level(), 
										thing.get_foot_level())) 
		return { 0.0f, false };

	return { 1 - std::abs(2.0f * t - 1.0f), true };

}
