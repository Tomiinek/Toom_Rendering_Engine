#ifndef GameMap_h
#define GameMap_h

/**
	GameMap.h
	Description:
		Holds entire map. Map consists of sectors_, lines_, line sides_ and things_. Lines are formed
		using vertices, these are stored in vertices_. Special sectors are doors.
*/

#include <vector>
#include <istream>
#include <limits>
#include <array>
#include <algorithm>
#include <fstream>

#include "MapTypes.h"
#include "BSPTree.h"
#include "BlockMap.h"
#include "Player.h"
#include "Geometry.h"
#include "Weapon.h"
#include "Enemy.h"
#include "Sounds.h"

#include <SFML/Graphics.hpp>

class GameMap {
public:

	/**
	@param filename  Name of the map's file.
	@return          False if map could not be loaded from the given file.
	*/
	bool load(const std::string& filename);

	const BSPTree& get_bsp_tree() const { return bsp_tree_; }
	const BlockMap& get_block_map() const { return block_map_; }
	things_up_t & get_things() { return things_; }

	/**
	@param from_eyes  Spectator's eye level (z-coordinate).
	@param from       Spectator's position.
	@param to         Target's position.
	@param to_head    Target's eye level (z-coordinate).
	@param to_foot    Target's foot level (z-coordinate).
	*/
	bool check_los(float from_eyes, Vector from, Vector to, float to_head, float to_foot) const;
	bool check_los(Thing& from, Thing& to) const {
		return check_los(from.get_eye_level(), from.get_position(), to.get_position(), to.get_head_level(), to.get_foot_level());
	}

	/**
	@param block  Checks given block doors and change their state appropriately.
	@param pos    Typically position of the player in map.
	*/
	static void check_block_doors(const lines_p_t* block, const Vector& pos);

	void update(float seconds_elapsed);

private:

	bool los_crosses_bsp_node(const BSPTree::node& node,    StraightLine ray, float z_start, float bottom_slope, float top_slope) const;
	bool los_crosses_subsector(const segments_t& sub_sector, StraightLine ray, float z_start, float bottom_slope, float top_slope) const;

	vertices_t vertices_;
	sides_t sides_;
	sectors_t sectors_;
	doors_p_t doors_;
	textures_up_t textures_;
	things_up_t things_;
	lines_t lines_;

	BSPTree bsp_tree_;
	BlockMap block_map_;
};


#endif // !GameMap_h
 