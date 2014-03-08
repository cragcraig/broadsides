#include "reef.h"
#include "world.h"

Reef::Reef(World* world) :
	Agent(world, 0, 0, 0),
	type(0)
{ }

Reef::Reef(World* world, int x, int y, int type) :
	Agent(world, x, y, 0),
	type(type)
{
	world->set_ship(get_x(), get_y(), this);
}

void Reef::render(BITMAP* buffer, double step_fraction, int status)
{
	int dx, dy;

	world->toscreen(get_x(), get_y(), dx, dy);

	BITMAP* t = world->img_data.reef[type];
	
	draw_sprite(buffer, t, dx - t->w/2, dy - t->h/2);
}

void Reef::step()
{ }

void Reef::collision(Agent* a)
{

}

void Reef::set_pos(int to_x, int to_y)
{
	Agent::set_pos(to_x, to_y);
	world->set_ship(get_x(), get_y(), this);
}
