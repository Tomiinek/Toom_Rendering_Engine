#ifndef Sounds_h
#define Sounds_h

/**
	Sounds.h
	Description:
		Singleton class
		Sounds storage, all sounds are preloaded on beginning via load method
		Sounds can be invoked in game by calling play method
*/

#include <SFML/Audio.hpp>

#include <unordered_map>
#include <memory>

class Sounds {
public:

	Sounds(Sounds const&) = delete;
	void operator=(Sounds const&) = delete;

	static Sounds& global() {
		static Sounds global;
		return global;
	}

	enum Type {
		activation,
		bullet_wall,
		death,
		death_enemy,
		door_end,
		door,
		fall,
		fire_enemy,
		fire_player,
		intro,
		jump,
		walk,
		environment
	};

	/**
	@param type Type of sound to be checked (see Type enum above).
	@return True if new sound of this type can be created, false otherwise.
	*/
	bool is_playing(Type type);
	/**
	@param type Type of sound to be stopped.
	*/
	void stop(Type type) {
		auto search = sounds_.find(type);
		if (search != sounds_.end()) search->second.instances.clear();
	}

	/**
	@param type     Type of sound to be loaded.
	@param filename Name of the sound's file.
	@param one_off  True if only one sound of this type can be played in the particular moment.
	@param loop     True if sound should be played in loop.
	*/
	void load(Type type, std::string filename, bool one_off, bool loop) {

		auto buffer = std::make_unique<sf::SoundBuffer>();
		if (!buffer->loadFromFile(filename)) throw std::exception("Failed to load a sound file!");
		sounds_.insert({ type, SoundInstances(one_off, loop, move(buffer)) });
	}

	/**
	@param type     Type of sound to be played.
	@param volume   Volume of the new sound.
	*/
	void play(Type type, int volume);

private:

	struct SoundInstances {

		SoundInstances(bool o, bool l, std::unique_ptr<sf::SoundBuffer> b) : one_off(o), loop(l), buffer(move(b)) {}

		bool one_off;
		bool loop;
		std::unique_ptr<sf::SoundBuffer> buffer;
		std::vector<sf::Sound> instances;

		/**
		Removes instances of sound which are already completed.
		*/
		void refresh() {
			instances.erase(std::remove_if(instances.begin(), instances.end(),
				[](sf::Sound const& s) { return s.getStatus() != sf::SoundSource::Playing; }), instances.end());
		}
	};

	Sounds(){}

	std::unordered_map< Type, SoundInstances, std::hash<int> > sounds_;

};

#endif // !Sounds_h
