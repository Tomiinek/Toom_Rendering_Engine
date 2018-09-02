#ifndef Geometry_h
#define Geometry_h

/**
	Geometry.h
	Description: 
		Definition of PI constant
		Vectors in 2D and 3D (operations over them, their length and normalization)
		StraightLine defines true line (not segment)
*/

#include <cmath>

const static float PI_180 = 3.14159265358979f / 180.0f;

struct Vector {
	float X;
	float Y;

	bool operator== (const Vector& v) const { return (X == v.X) && (Y == v.Y); }
	bool operator!= (const Vector& v) const { return (X != v.X) || (Y != v.Y); }
	friend Vector operator* (float f, const Vector& v) { return Vector{ f * v.X, f * v.Y }; }
	friend Vector operator- (const Vector& v1, const Vector& v2) { return Vector{ v1.X - v2.X, v1.Y - v2.Y }; }
	friend Vector operator+ (const Vector& v1, const Vector& v2) { return Vector{ v1.X + v2.X, v1.Y + v2.Y }; }
	friend float operator*  (const Vector& v1, const Vector& v2) { return v1.X*v2.X + v1.Y*v2.Y; } // dot
	friend float operator%  (const Vector& v1, const Vector& v2) { return v1.X*v2.Y - v1.Y*v2.X; } // cross

	static Vector normalize(const Vector& v) { float l = v.length();  return Vector{ v.X / l, v.Y / l }; }
	float length() const { return std::sqrt(X * X + Y * Y); }

	Vector& operator+= (const Vector& v) { X += v.X; Y += v.Y; return *this; }
	Vector& operator-= (const Vector& v) { X -= v.X; Y -= v.Y; return *this; }
};

struct Vector3 {
	float X;
	float Y;
	float Z;

	Vector3& operator+= (const Vector3& v) { X += v.X; Y += v.Y; Z += v.Z; return *this; }
};

struct StraightLine {

	StraightLine() = default;
	StraightLine(const Vector& p1, const Vector& p2) {
		point = p1;
		diff = p2 - p1;
	}

	Vector point;
	Vector diff;

	/**
	@param s The point to be checked.
	@return 1 if points is on left side of line vector, 0 on right and -1 if on line.
	*/
	int determine_pnt_side(const Vector& s) const;

	/**
	@param l1 One line.
	@param l2 Another line.
	@return parameter for second argument, -1 if there is no intersection
	*/
	static float line_intersection(const StraightLine& l1, const StraightLine& l2);

	/**
	@param p1 This line acts like segment.
	@param p2 This line acts like ray.
	@return The volume of the sphere.
	*/
	float ray_segment_intersection(const Vector& p1, const Vector& p2);
};

#endif // !Geometry_h
