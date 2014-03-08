#include "agent.h"
#include "world.h"

Agent::Agent(World* world, int x, int y, int dir, bool startmove) :
	x(x),
	y(y),
	prev_x(x),
	prev_y(y),
	active(true),
	dead(false),
	steps(0),
	world(world)
{
	set_dir(dir);
	set_dir(dir);
	if (startmove) {
		int dx, dy;
		bool w;

		w = world->move(x, y, dir, dx, dy);
		
		set_pos(dx, dy);
		if (w)
			reset_prev();
	}
}

void Agent::set_pos(int to_x, int to_y)
{
	prev_x = x;
	x = to_x;
	prev_y = y;
	y = to_y;
}
