#include "Geometry.h"

int StraightLine::determine_pnt_side(const Vector & p) const {

	float position = diff % (p - point);

	if (position < 0.1f && position > -0.1f) return -1;
	if (position < 0) return 0;
	if (position > 0) return 1;

	return 0;
}

float StraightLine::line_intersection(const StraightLine& l1, const StraightLine& l2) {

	float d = l2.diff % l1.diff;

	// lines are parallel
	if (d == 0) return -1;

	float t = ((l1.point.X - l2.point.X) * l1.diff.Y + (l2.point.Y - l1.point.Y) * l1.diff.X) / d;

	// lines do not intersect
	return (t <= 0.0 || t >= 1.0) ? -1 : t;
}

float StraightLine::ray_segment_intersection(const Vector & p1, const Vector & p2) {
	
	StraightLine line(p1, p2);
	return StraightLine::line_intersection(*this, line);;
}
