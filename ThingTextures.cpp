#include "ThingTextures.h"

void ThingTextures::load(const std::string & name, std::size_t height, std::size_t frame_width, std::size_t frames_number, const std::string& filename) {

	sf::Image image;
	if (!image.loadFromFile(filename)) throw std::exception("Failed to load a texture file!");

	std::vector<std::unique_ptr<sf::Texture>> frames;

	for (size_t i = 0; i < frames_number; i++) {
		auto t = std::make_unique<sf::Texture>();
		t->loadFromImage(image, sf::IntRect(frame_width* i, 0, frame_width, height));
		frames.push_back(move(t));
	}

	textures_.insert({ name, move(frames) });
}

sf::Texture * ThingTextures::get(const std::string & name, int angle) const {
	
	auto it = textures_.find(name);
	if (it != textures_.end() && it->second.size() != 0) {

		angle += 180 / it->second.size();
		while (angle < 0)   angle += 360;
		while (angle > 359) angle -= 360;

		int index = angle * (it->second.size() / 360.0f);
		return it->second[index].get();
	}
	else throw std::exception("Missing texture for a sprite!");

}
