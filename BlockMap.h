#ifndef BlockMap_h
#define BlockMap_h

/**
	BlockMap.h
	Description:
		This structure divides whole map into blocks. Size of blocks is defined by block_size.
		Lines which are in more than one block are included in each of them. Building of 
		block map uses DDA algorithm.
		Block map is used to determine thing vs wall collisions. Then there is no necessity 
		to check every wall vs the particular thing.
*/

#include <vector>

#include "MapTypes.h"
#include "Geometry.h"

class BlockMap {
public:

	BlockMap() = default;
	BlockMap(int size, int width) {
		blocks_ = std::vector<lines_p_t>(size);
		width_ = width;
	}

	/**
	@param point Point which specifies the block being returned.
	@return      Returns block associated with a point in the map
				 or nullptr if point is out of the map.
	*/
	const lines_p_t * get_block(const Vector& point) const;

	/**
	@param lines	Lines which are situated in the map.
	@param min		Minimal point in map (i.e. lower left corner).
	@param max		Maximal point in map (i.e. upper right corner).
	@return			Returns builded block map.
	*/
	static BlockMap build(const lines_t& lines, const Vector& min, const Vector& max);

	const static int block_size = 200; // square

private:		
	std::vector<lines_p_t> blocks_;
	int width_;
};


#endif // !BlockMap_h
