#include "Thing.h"

bool Thing::in_collision(const Thing& collider, const Vector& new_pos) const {

	Vector diff = pos_ - new_pos;
	float distance = diff.length();

	if (distance < radius_ + collider.get_radius()) return true;

	return false;
}

sf::Texture * Thing::get_texture(const Vector& reference) const {

	Vector dir = reference - pos_;
	float angle = atan2(dir.X, dir.Y) / PI_180;
	angle -= get_direction();
	while (angle < 0)   angle += 360;
	while (angle > 359) angle -= 360;
	return ThingTextures::global().get(name_, angle);

}
