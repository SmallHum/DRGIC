#include <iostream>
#include <string>
#include <time.h>
#include <cmath>
#include <windows.h>
#include "Viewport.h"

#define MEASURE_SEC (double)clock()/CLOCKS_PER_SEC
#define lerp(a,b,c) ((a)*(1-(c)))+((b)*(c))
#define clamp(a,b,c) min(max(a,b),c)
#define sign(a) (a) > 0? 1 : -1
#define FARZ 30

class Sphere {
public:
	Vec3 pos;
	double radius;
	Vec2 rand_uv;
	int id;
	double ld = 0; // linear distance in a gut cave

	Sphere() {}
	Sphere(double a, Vec3 p, int j) {
		radius = a; pos = p;
		rand_uv.x = (rand() % 100 - 50) / 50.0;
		rand_uv.y = (rand() % 100 - 50) / 50.0;
		id = j;
	}
	Sphere(double a, Vec3 p, int j, Vec2 r) {
		radius = a; pos = p;
		rand_uv = Vec2(r.x, r.y);
		id = j;
	}

	double dist_f(Vec3 o) {
		return  (o - pos).len() - radius;
	}
	Vec3 normal_f(Vec3 o) {
		return (pos - o).normalized();
	}
	Vec2 UV(Vec3 o) {
		double ux = ((radius / 5) * atan2((o - pos).y, (o - pos).x) / (2 * 3.14159265));
		double uy = (o - pos).normalized().z;
		return Vec2(ux * sqrt(1 - pow(uy, 2)) + 0.5 + rand_uv.x, ((radius / 5) * asin(uy) / (3.14159265)) + 0.5 + rand_uv.y);
	}
};

class KgbObj { //KGB because it's always directed at you like in DOOM
public:
	Surface* s;
	Vec3 pos;
	KgbObj(Vec3 a, Surface* b) {
		pos = a;
		s = b;
	}
};

double dot(Vec3 a, Vec3 b) {
	return a.x * b.x + (a.y * b.y) + (a.z * b.z);
}

int main() {

	LPCWSTR title = L"CAVE GAME AAA";
	SetConsoleTitle(title);
	Sleep(40);

	srand(time(NULL));

	//init mouse values
	POINT mouse;
	Vec2 ingame_mouse, prev_mouse, dmouse;
	tagRECT wind;
	Vec2 winpos;
	HWND found = FindWindowW(NULL, title);

	//change font size
	static CONSOLE_FONT_INFOEX  fontex;
	fontex.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetCurrentConsoleFontEx(hOut, 0, &fontex);
	fontex.FontWeight = 700;
	fontex.dwFontSize.X = 6;
	fontex.dwFontSize.Y = 6;
	SetCurrentConsoleFontEx(hOut, NULL, &fontex);

	std::cout << "Enter your char res: ";
	Vec2 scr_res;
	std::cin >> scr_res.x >> scr_res.y;

	Screen one = Screen(scr_res);

	//creates prompt to change cmd resolution
	std::string property_cmd = "mode con: cols=" + std::to_string((size_t)scr_res.x + 1) + " lines=" + std::to_string((size_t)scr_res.y + 15);
	char* prompt = new char[property_cmd.size()];
	for (size_t i = 0; i < property_cmd.size(); i++) {
		prompt[i] = property_cmd[i];
	}

	//writes this prompt
	system(prompt);
	delete[] prompt;
	delete[] & property_cmd;

	double t1, dt = 0.001;
	bool running = true;

	//creating texture array of stone
	double stone_spr[128 * 128 * 2];
	//adding noise to it
	for (size_t i = 0; i < 128 * 128 * 2; i += 2) {
		stone_spr[i] = rand() % 5;
		stone_spr[i + 1] = 1.0;
	}
	//blur to make it look like 1.14 minecraft stone
	//x blur
	for (size_t t = 0; t < 5; t++) {
		for (size_t i = 0; i < 128 * 128 * 2 - 2; i += 2) {
			stone_spr[i] = lerp(stone_spr[i], stone_spr[i + 2], 0.5);
		}
	}
	//y blur
	for (size_t ys = 0; ys < 128 - 2; ys += 2) {
		for (size_t xs = 0; xs < 128 * 2 - 2; xs += 2) {
			stone_spr[(ys * 128) + xs] = lerp(stone_spr[(ys * 128) + xs], stone_spr[((ys + 2) * 128) + xs], 0.5);
		}
	}
	//implementing it into stone Surface object
	Surface stone = Surface(Vec2(128, 128), &(stone_spr[0]));

	double sprite_c[128 * 128 * 2];

	for (size_t i = 0; i < 128 * 128 * 2; i += 2) {
		sprite_c[i] = sqrt(pow((i % 64 - 64) / 64, 2) + pow(((long)(i / 64) - 64) / 64, 2));
		sprite_c[i + 1] = 1.0;
	}
	Surface Shine = Surface(Vec2(128, 128), &(sprite_c[0]));
	KgbObj Shine_kgb = KgbObj(Vec3(10, 0, 0), &Shine);

	//creates spheres array
	const size_t AMMOUNT = 64;
	Sphere spheres[AMMOUNT];
	Sphere* to_process[AMMOUNT];
	size_t first_unloaded = 0;

	//cave generation
	Vec3 v = Vec3(0, 0, 0), d = Vec3(1, 0, 0);
	int rand_radius = 3, max_height = -9999;
	double linear_d = 0, curr_ld = 0;
	for (size_t i = 0; i < AMMOUNT; i++) {
		rand_radius = rand() % 3 + 5;
		spheres[i] = Sphere(rand_radius, v, i);
		to_process[i] = &(spheres[i]);
		if (v.z + rand_radius > max_height)max_height = v.z + rand_radius;
		if (i % 4 == 0) {
			d.rotate_yz((rand() % 100 - 50) * 0.01, (rand() % 2 == 0 ? -0.7 : 0.7));
		}
		else {
			d.rotate_yz((rand() % 100 - 50) * 0.005, (rand() % 100 - 50) * 0.005);
		}
		if (i > 0) {
			linear_d += (spheres[i].pos - spheres[i - 1].pos).len();
			spheres[i].ld = linear_d;
		}
		v += d * rand_radius * (rand() % 125 + 10) / 100;
	}

	//variables used for rendering and physics
	Vec3 pl_pos, ray_pos, pl_dir, ray_dir, normal;
	double dist, min_dist, cam_dist = 0, pl_ang_y = 0, pl_ang_z = 0;
	double color;
	Vec2 uv;
	long mi;
	double fov = 3.14159265 / 2;
	Vec3 pl_vel, pl_accel;
	Vec3 camp_visual;

	bool is_start = 1; //corrects dmouse value at the start
	bool prev_input[3] = { 0,0,0 };
	bool input[3] = { 0,0,0 };

	bool on_ground = 0;

	double g = -9.8, jump_cd = 0;

	ShowCursor(0);

	while (running) {
		t1 = MEASURE_SEC;

		//find mouse, dmouse
		prev_mouse = ingame_mouse;
		if (GetCursorPos(&mouse)) {}
		GetWindowRect(found, &wind);
		winpos = Vec2(wind.left, wind.top);
		ingame_mouse.x = (mouse.x - winpos.x) / 6.0;
		ingame_mouse.y = (mouse.y - winpos.y) / 9.0 - 1;
		dmouse = ingame_mouse - prev_mouse;

		for (int i = 0; i < 3; i++) {
			prev_input[i] = input[i];
		}
		input[0] = GetKeyState('W') & 0x8000;
		input[1] = GetKeyState('S') & 0x8000;
		input[2] = GetKeyState(' ') & 0x8000;

		//corrects dmouse value if it's the first frame
		if (is_start == 1) { dmouse = Vec2(0, 0); is_start = 0; }

		if (ingame_mouse.x < 0 || ingame_mouse.y < 0 || ingame_mouse.x >= one.res.x || ingame_mouse.y >= one.res.y) {
			SetCursorPos(6 * 0.5 * one.res.x + winpos.x, 8.5 * 0.5 * one.res.y + winpos.y);
			is_start = 1;
		}

		pl_ang_y -= (dmouse.y) * 0.1;
		pl_ang_z -= (dmouse.x) * 0.1;
		pl_ang_y = clamp(pl_ang_y, -3.14159265 * 0.5, 3.14159265 * 0.5);
		if (input[0]) {
			pl_accel = Vec3(6, 0, 0).rotated_z(pl_ang_z);
		}
		else if (input[1]) {
			pl_accel = Vec3(-6, 0, 0).rotated_z(pl_ang_z);
		}
		else {
			pl_accel = Vec3(0, 0, 0);
		}
		pl_accel.z += g;

		pl_dir = Vec3(1, 0, 0).rotated_yz(pl_ang_y, pl_ang_z);

		min_dist = 999999;

		//unload spheres
		for (long s_i = 0; s_i < first_unloaded; s_i++) {
			dist = (*to_process[s_i]).dist_f(pl_pos);
			//dist = abs((*to_process[s_i]).ld-curr_ld);
			if (dist >= FARZ || (dist >= 3 && dot(pl_dir, ((*to_process[s_i]).pos - pl_pos).normalized()) <= 0.3)) {
				Sphere* unl = to_process[s_i];
				first_unloaded--;
				for (long unl_i = s_i; unl_i < first_unloaded; unl_i++) {
					to_process[unl_i] = to_process[unl_i + 1];
				}
				to_process[first_unloaded] = unl;
				s_i--;
			}
		}

		//load spheres
		for (long s_i = first_unloaded; s_i < AMMOUNT; s_i++) {
			dist = (*to_process[s_i]).dist_f(pl_pos);
			//dist = abs((*to_process[s_i]).ld - curr_ld);
			if (!(dist >= FARZ || (dist >= 3 && dot(pl_dir, ((*to_process[s_i]).pos - pl_pos).normalized()) <= 0.3))) {
				Sphere* l = to_process[s_i];
				if (s_i != first_unloaded) {
					for (long l_i = s_i; l_i > first_unloaded; l_i--) {
						to_process[l_i] = to_process[l_i - 1];
					}
					to_process[first_unloaded] = l;
				}
				first_unloaded++;
			}
		}

		//find minimun distance from sphere to player
		for (size_t s_i = 0; s_i < first_unloaded; s_i++) {
			dist = (*to_process[s_i]).dist_f(pl_pos);
			if (dist < min_dist) {
				min_dist = dist;
				mi = s_i;
			}
		}

		curr_ld = (*to_process[mi]).ld;

		jump_cd = max(0, jump_cd - (clamp(pl_vel.len(), 2, 10) * 0.5 * dt));

		//collision detection and resolution
		if (min_dist + 1 > 0) {
			normal = (*to_process[mi]).normal_f(pl_pos);
			pl_pos += normal * (min_dist + 1);
			double dvel = dot(normal, pl_vel);
			on_ground = 1;
			pl_vel -= normal * dvel;
			if (!prev_input[2] && input[2] && jump_cd == 0) {
				pl_vel += normal * 4;
				jump_cd = 3;
			}
			pl_vel = pl_vel * pow(0.8, dt);
		}
		else {
			on_ground = 0;
		}

		double pl_speed = pl_vel.len();
		camp_visual = on_ground ? normal * sin(pl_speed*t1*100) * 0.01 * min(5,pl_speed) : Vec3(0, 0, 0);

		pl_vel += pl_accel * dt;
		pl_pos += pl_vel * dt;

		one.fill(' ');

		//renderint
		for (long ys = 0; ys < one.res.y; ys++) {
			for (long xs = 0; xs < one.res.x; xs++) {
				ray_pos = pl_pos+camp_visual;
				double rang_z = (xs / (one.res.x * 0.5) - 1) * fov / 2.5;
				double rang_y = ((1 - (ys / (one.res.y * 0.5)))) * fov / 2.5;

				//1 - projecting onto a plane
				//2 - projecting onto a sphere

				//ray_dir = Vec3(1, ((xs/(one.res.x * 0.5))-1)*sin(fov/2), (1 - (ys / (one.res.y * 0.5))) * sin(fov / 2)*(one.res.y/one.res.x)).rotated_yz(pl_ang_y, pl_ang_z);
				ray_dir = Vec3(1, 0, 0).rotated_yz(rang_y, -rang_z).rotated_yz(pl_ang_y, pl_ang_z);

				min_dist = 1;
				size_t iters = 0;
				cam_dist = 0;
				while ((abs(min_dist) > 0.01 && iters < 20) && cam_dist < FARZ) {
					iters++;
					min_dist = 999999;
					for (size_t s_i = 0; s_i < first_unloaded; s_i++) {
						dist = (*to_process[s_i]).dist_f(ray_pos);
						if (dist < min_dist) {
							min_dist = dist;
							mi = s_i;
						}
					}
					ray_pos -= ray_dir * min_dist;
					cam_dist -= min_dist;
				}

				uv = (*to_process[mi]).UV(ray_pos);
				normal = (*to_process[mi]).normal_f(ray_pos);
				if (cam_dist >= FARZ) { color = 0; }
				else {
					color = clamp((0.25 * ((xs + ys) % 2)) + (max(dot(normal, (ray_dir - (pl_dir * 5) + Vec3(0, 0, 1)).normalized()), 0) * stone.color_at_uv(uv) * (1 / (max(cam_dist * 0.1, 0.75)) - (1 / FARZ))), 0, 4);
				}
				one.draw_pixel(Vec2(xs, ys), one.pal[(long)color]);
			}
		}

		one.draw_pixel(one.res * 0.5, one.pal[4 - one.color_at_2d(one.res * 0.5)]); //crosshair that inverts

		one.print();
		std::cout << "\nDEPTH: " << max_height - pl_pos.z << "\nTIME: " << t1 << "\nFPS: " << 1 / dt << " " << curr_ld;
		std::cout << "\n\n";

		for (int i = 0; i < first_unloaded; i++) {
			std::cout << (*to_process[i]).id << " ";
		}
		for (int i = first_unloaded; i < AMMOUNT; i++) {
			std::cout << "\x1B[31m" << (*to_process[i]).id << " ";
		}
		std::cout << "\033[0m";


		std::cout << "\n\n";

		for (int i = 0; i < jump_cd * one.res.x / 10; i++) {
			std::cout << "\x1B[31m" << one.pal[4];
		}
		for (int i = 0; i < one.res.x; i++) {
			std::cout << " ";
		}
		std::cout << "\033[0m";

		dt = MEASURE_SEC - t1;
	}

	return 0;
}
