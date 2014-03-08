#include "img.h"
#include <cstdio>

Img::Img() : initd(false)
{ }

void Img::init()
{
	initd = true;

	load(ship, 8, "ship");
	load(shoot, 8, "shoot");
	load(ball, 8, "ball");
	load(fire, 2, "fire");
	load(reef, 2, "reef");
	load(&water, 1, "water");
}

Img::~Img()
{
	if (!initd)
		return;

	destroy(ship, 8);
	destroy(shoot, 8);
	destroy(ball, 8);
	destroy(fire, 2);
	destroy(reef, 2);
	destroy(&water, 1);
}

/* load images */
bool Img::load(BITMAP* imgs[], int n, const char* fname_base)
{
	char ind[512];

	for (int i = 0; i < n; i++) {
		sprintf(ind, "img/%s%d.bmp", fname_base, i);
		imgs[i] = load_bitmap(ind, NULL);
		if (!imgs[i]) {
			fprintf(stderr, "error loading %s%d.bmp, here there be segfaults\n", fname_base, i);
			return false;
		}
	}

	return true;
}

void Img::destroy(BITMAP* imgs[], int n)
{
	for (int i = 0; i < n; i++)
		if (imgs[i])
			destroy_bitmap(imgs[i]);
}
