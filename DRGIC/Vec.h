#include <iostream>
#include <cmath>

#define v(a,b) Vec2(a,b)

class Vec2 {
public:
	double xy[2];
	double& x = xy[0];
	double& y = xy[1];
	Vec2() {
		x = 0;
		y = 0;
	}
	Vec2(double a, double b) {
		x = a;
		y = b;
	}
	void operator =(Vec2 other) {
		x = other.x;
		y = other.y;
	}
	Vec2 operator +(Vec2 other) {
		return Vec2(x + other.x, y + other.y);
	}
	Vec2 operator -(Vec2 other) {
		return Vec2(x - other.x, y - other.y);
	}
	Vec2 operator *(double k) {
		return Vec2(x * k, y * k);
	}
	Vec2 operator /(double k) {
		return Vec2(x / k, y / k);
	}
	void operator +=(Vec2 other) {
		x += other.x;
		y += other.y;
	}
	void operator -=(Vec2 other) {
		x -= other.x;
		y -= other.y;
	}
	void operator *=(double k) {
		x *= k;
		y *= k;
	}
	void operator /=(double k) {
		x /= k;
		y /= k;
	}
	double& operator [](size_t a) {
		return xy[a];
	}
	double len_sq() {
		return x * x + (y * y);
	}
	double len() {
		return std::sqrt(len_sq());
	}
};

class Vec3 {
public:
	double xyz[3];
	double& x = xyz[0];
	double& y = xyz[1];
	double& z = xyz[2];
	Vec3() {
		x = 0;
		y = 0;
		z = 0;
	}
	Vec3(double a, double b, double c) {
		x = a;
		y = b;
		z = c;
	}
	void operator =(Vec3 other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	Vec3 operator +(Vec3 other) {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}
	Vec3 operator -(Vec3 other) {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}
	Vec3 operator *(double k) {
		return Vec3(x * k, y * k, z * k);
	}
	Vec3 operator /(double k) {
		return Vec3(x / k, y / k, z / k);
	}
	void operator +=(Vec3 other) {
		x += other.x;
		y += other.y;
		z += other.z;
	}
	void operator -=(Vec3 other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
	}
	void operator *=(double k) {
		x *= k;
		y *= k;
		z *= k;
	}
	void operator /=(double k) {
		x /= k;
		y /= k;
		z /= k;
	}
	double& operator [](size_t a) {
		return xyz[a];
	}
	double len_sq() {
		return x * x + (y * y) + (z*z);
	}
	double len() {
		return std::sqrt(len_sq());
	}
	Vec3 normalized(){
		return (*this) / len();
	}
	Vec3 rotated_yz(double ry, double rz) {
		return Vec3(
			(((x * cos(ry)) - (z * sin(ry))) * cos(rz)) + (y * sin(rz)),
			(y * cos(rz)) + (sin(rz)*((z * sin(ry)) - (x * cos(ry)))),
			(x * sin(ry)) + (z * cos(ry))
		);
	}
	Vec3 rotated_z(double rz) {
		return Vec3(
			(x * cos(rz)) + (y * sin(rz)),
		    (-x * sin(rz)) + (y * cos(rz)),
		    z
		);
	}
	void rotate_yz(double ry, double rz) {
		x = (((x * cos(ry)) - (z * sin(ry))) * cos(rz)) + (y * sin(rz));
		y = (y * cos(rz)) + (sin(rz) * ((z * sin(ry)) - (x * cos(ry))));
		z = (x * sin(ry)) + (z * cos(ry));
	}
};