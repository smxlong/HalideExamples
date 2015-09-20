#ifndef HalideExamples_Vec_h
#define HalideExamples_Vec_h

#include <Halide.h>

namespace HalideExamples {

class Vec {
public:
	Vec(Halide::Expr x, Halide::Expr y, Halide::Expr z)
		: x(x)
		, y(y)
		, z(z)
	{
	}

	Vec normalized() const {
		Halide::Expr mag = magnitude();
		return Vec(x / mag, y / mag, z / mag);
	}

	Halide::Expr magnitude() const {
		return Halide::sqrt(x * x + y * y + z * z);
	}

	Halide::Expr magnitudeSquared() const {
		return x * x + y * y + z * z;
	}

	Halide::Expr x;
	Halide::Expr y;
	Halide::Expr z;
};

inline Vec operator+(Vec a, Vec b) {
	return Vec(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vec operator-(Vec a, Vec b) {
	return Vec(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vec operator*(Halide::Expr k, Vec a) {
	return Vec(k * a.x, k * a.y, k * a.z);
}

inline Vec operator*(Vec a, Halide::Expr k) {
	return k * a;
}

inline Vec operator/(Vec a, Halide::Expr k) {
	return Vec(a.x / k, a.y / k, a.z / k);
}

inline Halide::Expr dot(Vec a, Vec b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec cross(Vec a, Vec b) {
	return Vec(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}


}

#endif // HalideExamples_Vec_h
