#include <iostream>
#include <cmath>
#include <string>
#include <iostream>
#include "Vec.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define lerp(a,b,c) ((a)*(1-(c)))+((b)*(c))
#define clamp(a,b,c) min(max((a),(b)),(c))

class Surface {
public:
	Vec2 res = v(2, 2);
	double* surf = new double[4];
	double trans = 1;
	void resize(size_t new_size) {
		double* new_surf = new double[new_size];
		for (size_t i = 0; i < new_size; i++) {
			new_surf[i] = surf[i];
		}
		delete[] surf;
		surf = new_surf;
	}
	void init_surf() {
		resize((size_t)(res.x) * (size_t)res.y * 2);
		for (size_t ys = 0; ys < res.y; ys++) {
			for (size_t xs = 0; xs < res.x; xs++) {
				draw_pixel(v(xs, ys), rand() % 5, 1);
			}
		}
	}
	Surface() {

	}
	Surface(Vec2 s, double* spr) {
		res = s;
		init_surf();
		for (size_t i = 0; i < res.x * 2 * res.y; i++) {
			surf[i] = spr[i];
		}
	}
	double& color_at(Vec2 pos) {
		return surf[(size_t)(pos.y) * (size_t)res.x * 2 + ((size_t)pos.x * 2)];
	}
	double color_at_uv_bilinear(Vec2 pos) {
		Vec2 nu = pos;
		nu.x = std::abs(fmod(nu.x, 1));
		nu.y = std::abs(fmod(nu.y, 1));
		double lerp_x = lerp(color_at(nu * (res.x - 1)), color_at(nu * (res.x - 1) + Vec2(1, 0)), abs(fmod((nu * (res.x - 1)).x, 1)));
		double lerp_y = lerp(color_at(nu * (res.x - 1) + Vec2(0, 1)), color_at(nu * (res.x - 1) + Vec2(1, 1)), abs(fmod((nu * (res.x - 1)).x, 1)));
		return lerp(lerp_x,lerp_y, abs(fmod((nu * (res.x - 1)).y, 1)));
	}
	double color_at_uv(Vec2 pos) {
		Vec2 nu = pos;
		nu.x = std::abs(fmod(nu.x, 1));
		nu.y = std::abs(fmod(nu.y, 1));
		return color_at(nu * (res.x - 1));
	}
	double& alpha_at(Vec2 pos) {
		return surf[(size_t)(pos.y) * (size_t)res.x * 2 + ((size_t)pos.x * 2) + 1];
	}
	void draw_pixel(Vec2 pos, double color, double alpha) {
		color_at(pos) = color;
		alpha_at(pos) = alpha;
	}
};

class Screen {
public:
	Vec2 res;
	std::string scr;
	Vec2 disp = { 0,0 };
	const char pal[5] = { ' ',(char)176,(char)177,(char)178,(char)219 };
	double pal_to_val(char a) {
		for (size_t i = 0; i < 5; i++) {
			if (a == pal[i]) { return i; }
		}
		return 0;
	}
	void init_screen() {
		scr = " ";
		scr.resize((res.x + 1) * res.y, ' ');
		for (size_t ys = 0; ys < res.y; ys++) {
			scr[(res.x + 1) * ys + res.x] = '\n';
		}
	}
	Screen() {
		res = v(64, 32);
		init_screen();
	}
	Screen(Vec2 size) {
		res = size;
		init_screen();
	}
	void print() {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0,0 });
		std::cout << scr;
	}
	char& at_2d(Vec2 pos) {
		return scr.at((res.x + 1) * (long)(pos.y - disp.y) + ((long)(pos.x - disp.x)));
	}
	int color_at_2d(Vec2 pos) {
		const char sas = at_2d(pos);
		for (int i = 0; i < 5; i++) {
			if (pal[i] == sas) {
				return i;
			}
		}
		return 0;
	}
	bool draw_pixel(Vec2 p, char a) {
		if (p.x - disp.x < 0 || p.x - disp.x > res.x - 1 || p.y - disp.y < 0 || p.y - disp.y > res.y - 1) {
			return false;
		}
		at_2d(p) = a;
		return true;
	}

	void draw_circle(Vec2 center, double radius, char a) {
		double dx, dy;
		for (size_t ys = center.y - radius; ys <= center.y; ys++) {
			for (size_t xs = center.x - radius; xs <= center.x; xs++) {
				dx = xs - center.x;
				dy = ys - center.y;
				if ((dx * dx) + (dy * dy) <= radius * radius) {
					draw_pixel(v(xs, ys), a);
					draw_pixel(v(center.x - dx, ys), a);
					draw_pixel(v(xs, (center.y - dy)), a);
					draw_pixel(v(center.x - dx, (center.y - dy)), a);
				}
			}
		}
	}

	void fill(char a) {
		for (long ys = disp.y; ys < res.y + disp.y; ys++) {
			for (long xs = disp.x; xs < res.x + disp.x; xs++) {
				draw_pixel(v(xs, ys), a);
			}
		}
	}

	bool blit(Surface& s, Vec2 pos, bool flip = 0) {
		if (pos.x + s.res.x < disp.x || pos.x > disp.x + res.x || pos.y + s.res.y < disp.y || pos.y > disp.y + res.y) { return false; }
		double value;
		Vec2 spr_pos;
		for (long ys = (long)max(pos.y, disp.y); ys < s.res.y + (long)(pos.y); ys++) {
			if (ys - disp.y > res.y) {
				break;
			}
			for (long xs = (long)(pos.x); xs < s.res.x + (long)(pos.x); xs++) {
				if (xs - disp.x < 0 || xs - disp.x > res.x - 1) {
					continue;
				}
				spr_pos = flip ? v(xs - (long)(pos.x), ys - (long)(pos.y)) : v(s.res.x - 1 - xs + (long)(pos.x), ys - (long)(pos.y));
				if (s.trans * s.alpha_at(spr_pos) == 1) {
					value = s.color_at(spr_pos);
				}
				value = clamp(lerp(pal_to_val(at_2d(v(xs, ys))), s.color_at(spr_pos), s.trans * s.alpha_at(spr_pos)), 0, 4);
				draw_pixel(v(xs, ys), pal[(long)value]);
			}
		}
		return true;
	}
};