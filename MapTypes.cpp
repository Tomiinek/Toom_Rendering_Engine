#include "MapTypes.h"

bool Line::in_collision(const Vector& test_s, const Vector& test_e) const {

	int o1 = orientation(*start, *end, test_s);
	int o2 = orientation(*start, *end, test_e);
	int o3 = orientation(test_s, test_e, *start);
	int o4 = orientation(test_s, test_e, *end);

	if (o1 != o2 && o3 != o4) return true;
	if (o1 == 0 && on_segment(*start, test_s, *end))   return true;
	if (o2 == 0 && on_segment(*start, test_e, *end))   return true;
	if (o3 == 0 && on_segment(test_s, *start, test_e)) return true;
	if (o4 == 0 && on_segment(test_s, *end, test_e))   return true;

	return false;
}

int Line::orientation(Vector p, Vector q, Vector r) {

	float val = (q.Y-p.Y) * (r.X-q.X) - (q.X-p.X) * (r.Y-q.Y);
	if (val == 0) return 0;
	return (val > 0) ? 1 : 2;

}

bool Line::on_segment(Vector p, Vector q, Vector r) const {

	if (q.X <= std::max(p.X, r.X) && q.X >= std::min(p.X, r.X) &&
		q.Y <= std::max(p.Y, r.Y) && q.Y >= std::min(p.Y, r.Y))
		return true;

	return false;

}
