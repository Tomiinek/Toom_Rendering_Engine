#include "Sounds.h"

bool Sounds::is_playing(Type type) {

	auto search = sounds_.find(type);
	if (search != sounds_.end()) {

		search->second.refresh();

		if (search->second.one_off) return !search->second.instances.empty();
		else return false;
	}

	return true;
}

void Sounds::play(Type type, int volume) {

	auto search = sounds_.find(type);
	if (search != sounds_.end()) {

		search->second.refresh();

		// sound is one off and is already playing
		if (search->second.one_off && !search->second.instances.empty()) return;

		sf::Sound sound;
		sound.setLoop(search->second.loop);
		sound.setVolume(volume);
		sound.setBuffer(*search->second.buffer);

		search->second.instances.push_back(sound);
		search->second.instances.back().play();
	}
}
