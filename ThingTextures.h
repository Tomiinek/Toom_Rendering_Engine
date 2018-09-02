#ifndef ThingTextures_h
#define ThingTextures_h

/**
	ThingTextures.h
	Description:
		Class which manages textures of all sprites. Textures of a one thing are loaded from single file.
		This file consists of few frames in a row, which are representing thing's look from particular angle.
		Thing is defined by its name and textures for specific angles are stored in vector.
		Singleton structure.
*/

#include <SFML/Graphics.hpp>

#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>

class ThingTextures {
public:

	ThingTextures(ThingTextures const&) = delete;
	void operator=(ThingTextures const&) = delete;

	static ThingTextures& global() {
		static ThingTextures global;
		return global;
	}

	/**
	@param name		      Name of thing associated with this textures.
	@param height	      Height of thing associated with this textures.
	@param frame_width    Width of one single frame in the file.
	@param frames_number  Number of frames.
	@param filename       Name of file to be loaded.
	*/
	void load(const std::string& name, std::size_t height, std::size_t frame_width, std::size_t frames_number, const std::string& filename);
	
	/**
	@param name	   Name of thing.
	@param angle   Angle - headig to the thing (from player).
	@return        Pointer to texture appropriate for the angle.
	*/
	sf::Texture* get(const std::string& name, int angle) const;

private:

	ThingTextures() {}

	std::unordered_map< std::string, std::vector<std::unique_ptr<sf::Texture>>> textures_;
};


#endif // !ThingTextures_h
