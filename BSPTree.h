#ifndef BSPTree_h
#define BSPTree_h

/**
	BSPTree.h
	Description:
		This structure divides entire map into convex areas. It is used for drawing walls
		in right order near -> far. BSP tree is builded from list of walls which forms 
		areas called sectors. If every sector is convex, there is no need to divide any area.
		Otherwise a cut must be made. In order to do this, algorithm try division along every
		wall in map and save its weight (a heuristics is used to determine the best cut). Than
		the wall with the lowest weight is used to cut the map into two halves.
		Convex sub-sectors are stored in leafs. Inner nodes hold division line and bounding box.
		Bounding box is implemented here, but is not used futher in this program, because it 
		is fast enough, so why bother.
*/

#include <vector>
#include <memory>
#include <stack>

#include "MapTypes.h"
#include "Geometry.h"

class BSPTree {
public:

	struct node;
	using node_p = std::unique_ptr<node>;

	struct node {
		bool leaf;
		segments_t lines;
		StraightLine split;
		Vector min_bound;
		Vector max_bound;
		node_p left;
		node_p right;
	};

	class iterator {
	public:
		iterator(Vector pos) : n_(nullptr), pos_(pos) {}
		iterator(const node* &n, const Vector& pos) : n_(n), pos_(pos) {}

		const segments_t & operator*() const {
			return n_->lines;
		}
		bool operator!=(const iterator & it) const {
			return it.n_ != n_;
		}
		iterator operator++();

	private:
		friend class BSPTree;

		const node* n_;
		std::stack<const node*> s_;
		Vector pos_;
	};

	/**
	@param position  Position of point which defines order of the traversal.
	*/
	iterator begin(const Vector& position) const;
	iterator end(const Vector& position) const {
		return iterator(position);
	}

	/**
	@param lines  Lines defining map.
	*/
	static BSPTree build(const lines_t& lines);

	/**
	@return  Returns reference to the tree root.
	*/
	const node& get_root() const { return *root_.get(); }

	/**
	@param point  A point which defines sub-sector being returned.
	@return       Returns sub-sector specified by point argument.
	*/
	const segments_t* get_subsector(const Vector& point) const;

private:

	/**
	@param lines  Lines to be converted to Segments, see MapTypes.h
	@return       Returns new vector of created segments.
	*/
	static segments_t lines_to_segments(const lines_t& lines);

	/**
	Checks if segments are convex; if so, new leaf node builded,
	otherwise a cut is made and new inner node is created.
	@param segments  Segments to be checked.
	@return			 A pointer to node, see method description.
	*/
    node_p partition_segments(segments_t& segments);

	static void div_line_from_segment(StraightLine& d, const Segment& s);

	/**
	@param segments    Sub-sector to be splitted, all segments are candidates.
	@param split_line  This arguments is filled by the best split candidate.
	@param best_value  Already found splitting value, suitable for clipping.
	@return			   Returns the best split weight.
	*/
	static int split_evaluate(const segments_t& segments, Segment& split_line, int best_value);

	/**
	@param s  Segment to be checked.
	@param d  Division line.
	@return   Returns 0 if entire segment is on the left side, 1 on the right side and
			  -1 if the segment must be splitted.
	*/
	static int determine_seg_side(const Segment& s, const StraightLine& d);

	/**
	@param segments  Segments to be checked divided into two halves.
	@param splitter  Division segment (converted to line).
	@param front     Argument is filled with segments on the left side
	@param back      Argument is filled with segments on the right side
	*/
	static void split_segments(const segments_t& segments, const Segment& splitter, segments_t& front, segments_t& back);

	static Segment split_segment(Segment& segment, const StraightLine& splitter);

	node_p root_;

};

#endif 
