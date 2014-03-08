#include "ball.h"
#include "ship.h"
#include "world.h"

void Ball::render(BITMAP* buffer, double step_fraction, int status)
{
	int dx, dy, dpx, dpy;

	world->toscreen(get_x(), get_y(), dx, dy);
	world->toscreen(get_prevx(), get_prevy(), dpx, dpy);

	BITMAP* t = world->img_data.ball[get_dir()];
	
	draw_sprite(buffer, t, 
		(dx - dpx) * step_fraction + dpx - t->w/2,
		(dy - dpy) * step_fraction + dpy - t->h/2);
}

void Ball::step()
{
	int dx, dy;
	bool w;

	/* move */
	for (int i = 0; i < speed; i++) {
		/* move */
		w = world->move(get_x(), get_y(), get_dir(), dx, dy);
		
		if (i == 0) {
			set_pos(dx, dy);
		} else {
			set_x(dx);
			set_y(dy);
		}
		
		if (w) reset_prev();
		if (!world->onmap(dx, dy)) destroy();
	}
	
	/* increment step count */
	inc_steps();

	/* hit water */
	if (get_steps() >= distance)
		destroy();
}

void Ball::collision(Agent* a)
{
	if (dynamic_cast<Ball*>(a)) {
		/* do nothing */
	} else {
		destroy();
	}
}

void Ball::hurt()
{

}

void Ball::destroy()
{
	kill();
}

void Ball::inform_hit()
{
	if (!source) return;
	source->inc_score();
}
