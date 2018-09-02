#include "BlockMap.h"

const lines_p_t * BlockMap::get_block(const Vector& point) const {

	int n = (int)(point.X / block_size) + (int)(point.Y / block_size) * width_;
	if (n < 0 || (size_t)n >= blocks_.size()) return nullptr;

	return &blocks_[n];
}

BlockMap BlockMap::build(const lines_t& lines, const Vector& min, const Vector& max) {

	Vector size = max - min;
	int width = (int)(size.X / block_size) +1;
	int n = width * (int)(size.Y / block_size +1);

	BlockMap bm(n, width);

	for (auto&& line : lines) {

		// DDA algorithm

		Vector s = { (float)(int)(line.start->X / block_size), (float)(int)(line.start->Y / block_size) };
		Vector e = { (float)(int)(line.end->X   / block_size), (float)(int)(line.end->Y   / block_size) };

		Vector dir = *line.end - *line.start;
		Vector step, side;

		Vector delta = { block_size * sqrt(1 + (dir.Y * dir.Y) / (dir.X * dir.X)),
						 block_size * sqrt(1 + (dir.X * dir.X) / (dir.Y * dir.Y)) };

		if (dir.X < 0) {
			step.X = -1;
			side.X = (line.start->X / block_size - s.X) * delta.X;
		}
		else {
			step.X = 1;
			side.X = ((s.X + 1) - line.start->X / block_size) * delta.X;
		}

		if (dir.Y < 0) {
			step.Y = -1;
			side.Y = (line.start->Y / block_size - s.Y) * delta.Y;
		}
		else {
			step.Y = 1;
			side.Y = ((s.Y + 1) - line.start->Y / block_size) * delta.Y;
		}

		int curr_n = (int)s.Y * width + (int)s.X;

		bm.blocks_[curr_n].push_back(&line);

		while (((int)s.X != (int)e.X) || ((int)s.Y != (int)e.Y)) {

			if (side.X < side.Y) {
				side.X += delta.X;
				s.X += step.X;
			}
			else {
				side.Y += delta.Y;
				s.Y += step.Y;
			}

			int new_n = (int)s.Y * width + (int)s.X;

			if (new_n != curr_n) {

				bm.blocks_[new_n].push_back(&line);
				curr_n = new_n;

			}
		}
	}

	return bm;

}
