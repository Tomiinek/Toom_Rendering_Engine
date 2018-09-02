#include "BSPTree.h"

const segments_t* BSPTree::get_subsector(const Vector& point) const {

	node* curr = root_.get();
	while (!curr->leaf) {

		if (curr->split.determine_pnt_side(point)) {
			curr = curr->right.get();
		}
		else {
			curr = curr->left.get();
		}
	}
	return &curr->lines;
}

BSPTree::iterator BSPTree::begin(const Vector& position) const {

	iterator new_it(position);

	const node* curr = root_.get();
	while (!curr->leaf) {

		if (curr->split.determine_pnt_side(position)) {
			new_it.s_.push(curr->left.get());
			curr = curr->right.get();		
		}
		else {
			new_it.s_.push(curr->right.get());
			curr = curr->left.get();		
		}
	}

	new_it.n_ = curr;
	return new_it;
}

BSPTree::iterator BSPTree::iterator::operator++() {

	if (s_.empty()) {
		n_ = nullptr;
	}
	else {
		const node* n = s_.top();
		s_.pop();

		while (!n->leaf) {

			if (n->split.determine_pnt_side(pos_)) {
				s_.push(n->left.get());
				n = n->right.get();				
			}
			else {
				s_.push(n->right.get());
				n = n->left.get();				
			}
		}

		n_ = n;
	}

	return *this;
}

BSPTree BSPTree::build(const lines_t & lines) {

	BSPTree new_tree;

	segments_t segs = new_tree.lines_to_segments(lines);
	new_tree.root_ = new_tree.partition_segments(segs);

	return new_tree;

}

void BSPTree::div_line_from_segment(StraightLine & d, const Segment & s) {

	d.point = s.start;
	d.diff.X = s.end.X - s.start.X;
	d.diff.Y = s.end.Y - s.start.Y;

}

int BSPTree::determine_seg_side(const Segment & s, const StraightLine & d) {

	//  0 - left 
	//  1 - right
	// -1 - must be splitted

	int start = d.determine_pnt_side(s.start);
	int end   = d.determine_pnt_side(s.end);

	if (start == end) {

		if (start == -1){

			// colinear lines, we care about direction
			float dx = s.end.X - s.start.X;
			float dy = s.end.Y - s.start.Y;

			// same direction is on left
			return !((dx > 0) - (dx < 0) == (d.diff.X > 0) - (d.diff.X < 0) &&
					 (dy > 0) - (dy < 0) == (d.diff.Y > 0) - (d.diff.Y < 0));
		}

		return start;
	}

	// one point is on split line
	if (start == -1) return end;
	if (end == -1)   return start;

	// there exists an intersection point
	return -1;

}

int BSPTree::split_evaluate(const segments_t & segments, Segment & split_segment, int best_value) {

	int new_value;
	int left_c = 0;
	int right_c = 0;

	// create splitting line from a segment
	StraightLine split;
	div_line_from_segment(split, split_segment);

	int c = segments.size();
	int side;

	// iterate through all segments and put them into right half-plane
	for (int i = 0; i != c; ++i) {

		Segment curr_segment = segments[i];
		side = (curr_segment == split_segment ? 0 : determine_seg_side(curr_segment, split));

		switch (side) {
			case 0: ++left_c;  break;
			case 1: ++right_c; break;
			case -1: ++left_c; ++right_c; break;
		}

		// heuristic, less splitting is better
		new_value = (left_c > right_c ? left_c : right_c) + (left_c + right_c - c) * 8;

		// cannot be better, should interrupt
		if (new_value > best_value) return new_value;
	}

	// splitting by boundary, we dont want this
	if (left_c == 0 || right_c == 0)
		return std::numeric_limits<int>::max();

	return new_value;

}

segments_t BSPTree::lines_to_segments(const lines_t & lines) {

	segments_t segs;

	for (auto &line : lines) {

		Segment s;
		s.start = *line.start;
		s.end = *line.end;
		s.line = &line;
		s.offset = 0;
		s.direction = false;

		segs.push_back(s);

		// line is two sided
		if (line.right != nullptr) {
			s.start = *line.end;
			s.end = *line.start;
			s.direction = true;
			segs.push_back(s);
		}
	}

	return segs;
}

Segment BSPTree::split_segment(Segment & segment, const StraightLine& splitter) {

	Segment new_segment;

	StraightLine seg_line;
	div_line_from_segment(seg_line, segment);

	float t = StraightLine::line_intersection(splitter, seg_line);
	Vector intersection = { seg_line.point.X + seg_line.diff.X * t,
							seg_line.point.Y + seg_line.diff.Y * t };
	
	new_segment = segment;
	new_segment.offset = segment.offset + t * seg_line.diff.length();

	int side = splitter.determine_pnt_side(segment.start);

	// front
	if (side == 0) {
		segment.end = intersection;
		new_segment.start = intersection;
	}
	// back
	else {
		segment.start = intersection;
		new_segment.end = intersection;
	}

	return new_segment;
}

void BSPTree::split_segments(const segments_t & segments, const Segment& splitter, segments_t & front, segments_t & back) {

	StraightLine line;
	div_line_from_segment(line, splitter);

	Segment segment;
	for (size_t i = 0; i != segments.size(); ++i) {

		segment = segments[i];
		int side;

		if (segment == splitter) side = 0;
		else side = determine_seg_side(segment, line);

		switch (side) {
		case 0: front.push_back(segment); break;
		case 1: back.push_back(segment);  break;
		case -1:
			back.push_back(split_segment(segment, line));
			front.push_back(segment);
			break;
		}

	}
}

BSPTree::node_p BSPTree::partition_segments(segments_t & segments) {

	// search for the best split candidate

	int best_value = std::numeric_limits<int>::max();
	Segment best_line, line;
	node_p new_node = std::make_unique<node>();

	int c = segments.size();
	//int step = (c / 40) + 1;
	int step = 1;

	float min_x = std::numeric_limits<float>::max();
	float min_y = std::numeric_limits<float>::max();
	float max_x = std::numeric_limits<float>::lowest();
	float max_y = std::numeric_limits<float>::lowest();

	for (int i = 0; i < c; i += step) {

		line = segments[i];

		if (line.end.X   < min_x) min_x = line.end.X;
		if (line.end.Y   < min_y) min_y = line.end.Y;
		if (line.start.X < min_x) min_x = line.start.X;
		if (line.start.Y < min_y) min_y = line.start.Y;

		if (line.end.X   > max_x) max_x = line.end.X;
		if (line.end.Y   > max_y) max_y = line.end.Y;
		if (line.start.X > max_x) max_x = line.start.X;
		if (line.start.Y > max_y) max_y = line.start.Y;
		
		int v = split_evaluate(segments, line, best_value);

		if (v < best_value) {
			best_value = v;
			best_line = line;
		}

	}

	// make bounding box for better search
	new_node->min_bound = Vector{ min_x, min_y };
	new_node->max_bound = Vector{ max_x, max_y };

	//
	// remaining lines are convex, and form a leaf node
	//

	if (best_value == std::numeric_limits<int>::max()) {

		new_node->lines = move(segments);
		new_node->leaf = true;
		return new_node;

	}

	//
	// divide the line list into two nodes along the best split line
	//

	div_line_from_segment(new_node->split, best_line);

	segments_t front;
	segments_t back;

	split_segments(segments, best_line, front, back);

 	new_node->left  = move(partition_segments(front));
	new_node->right = move(partition_segments(back));
	new_node->leaf = false;

	return new_node;

}