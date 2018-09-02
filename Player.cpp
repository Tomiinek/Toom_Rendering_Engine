#include "Player.h"

void Player::move_to(float seconds_elapsed, Controller::Direction move_direction, float angle_change) {

	Vector move_vector = { 0, 0 };
	float sin = std::sinf(angle_ * PI_180);
	float cos = std::cosf(angle_ * PI_180);

	if (move_direction & Controller::Direction::fwd) { move_vector += Vector{ sin, cos }; }
	if (move_direction & Controller::Direction::bck) { move_vector -= Vector{ sin, cos }; }
	if (move_direction & Controller::Direction::lft) { move_vector += Vector{ -cos, sin }; }
	if (move_direction & Controller::Direction::rgt) { move_vector -= Vector{ -cos, sin }; }

	/*if (move_dir || velocity_.X != 0 || velocity_.Y != 0) Sounds::global().play(Sounds::walk, 25);
	else Sounds::global().stop(Sounds::walk);*/

	// TODO: add walking sound

	float speed = velocity_.Z;
	Person::move_to(seconds_elapsed, move_vector, move_direction);

	if (velocity_.Z == 0 && speed < -800) Sounds::global().play(Sounds::fall,100);

	angle_ += angle_change;
	if (angle_ < 0)	  angle_ += 360;
	if (angle_ > 360) angle_ -= 360;

}

void Player::set_gun_animation(const std::vector<std::string>& files) {

	for (auto&& file : files) {
		std::unique_ptr<sf::Texture> t = std::make_unique<sf::Texture>();
		if (t->loadFromFile(file)) animation_.push_back(std::move(t));
		else throw std::exception("Error while loading player's gun textures!");
	}

}

void Player::set_gun(const sf::RenderWindow & window, const std::string & file) {

	if (texture_.loadFromFile(file)) gun_drawing_.setTexture(texture_, true);
	else throw std::exception("Error while loading player's gun textures!");

	// gun placement computation
	gun_drawing_.setPosition({ window.getSize().x / 2.0f - gun_drawing_.getTextureRect().width / 2.0f,
		(float)window.getSize().y - (float)gun_drawing_.getTextureRect().height });

}

void Player::update_shooting(int millis_elapsed) {

	if (weapon_.get_cooldown_progress() == 0) return;

	int phase = static_cast<int>((1.0f - weapon_.get_cooldown_progress()) * (int)animation_.size());
	weapon_.update(millis_elapsed);

	if (phase < (int)animation_.size()) gun_drawing_.setTexture(*animation_[phase]);
	else gun_drawing_.setTexture(texture_);

}
