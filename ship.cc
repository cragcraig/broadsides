#include <cstdio>
#include <cstring>
#include "ship.h"
#include "reef.h"
#include "world.h"
#include "statetable.h"

int Ship::A = 30;
int Ship::B = -12;
int Ship::C = 1;
int Ship::D = 10;

Ship::Ship(World* world, StateTable* st, int x, int y, int dir, int lives, int reload_delay) :
	Agent(world, x, y, dir),
	state_table(st),
	start_lives(lives),
	lives(lives),
	reload_delay(reload_delay),
	reload_count(0),
	on_fire(0),
	on_hurt(0),
	shoot_dir(-1),
	fires(0),
	score(0)
{
	set_fleet("evolved");
}

Ship::~Ship()
{
	if (state_table)
		delete state_table;
}

void Ship::render(BITMAP* buffer, double step_fraction, int status)
{
	int dx, dy, dpx, dpy;

	world->toscreen(get_x(), get_y(), dx, dy);
	world->toscreen(get_prevx(), get_prevy(), dpx, dpy);
	
	BITMAP* f = (shoot_dir >= 0) ? world->img_data.shoot[shoot_dir] : NULL;
	BITMAP* t = (on_fire && step_fraction > 0.1) ? world->img_data.fire[0] : world->img_data.ship[get_dir()];

	int drx = (dx - dpx) * step_fraction + dpx;
	int dry = (dy - dpy) * step_fraction + dpy;

	/* draw */
	if ((shoot_dir > 0 && shoot_dir < 4) && step_fraction < 0.5)
		draw_sprite(buffer, f, drx - f->w/2, dry - f->h/2);
	draw_sprite(buffer, t, drx - t->w/2, dry - t->h/2);
	if (((shoot_dir > 3 && shoot_dir < 8) || shoot_dir == 0) && step_fraction < 0.5)
		draw_sprite(buffer, f, drx - f->w/2, dry - f->h/2);

	/* hurt */
	if (on_hurt && step_fraction < 0.9)
		draw_sprite(buffer, world->img_data.fire[1],
			drx - world->img_data.fire[1]->w/2, dry - world->img_data.fire[1]->h/2);

	if (status) {
		/* hits */
		textprintf_ex(buffer, font, drx + 10 - t->w/2, dry + 10 - t->h/2,
			makecol(0,0,0), -1, "%d", score);
		textprintf_ex(buffer, font, drx + 9 - t->w/2, dry + 9 - t->h/2,
			(status == 2) ? makecol(150,0,0) : makecol(255,255,255), -1, "%d", score);
	
		/* lives */
		if (lives) {
			int cx = drx;
			int cy = dry - t->h/2 - 1;
			double pct = static_cast<double>(lives)/start_lives;
			double pct2 = (pct - 1./start_lives < 0.) ? 0. : pct - 1./start_lives;
			rectfill(buffer, cx - 15*pct, cy, cx + 15*pct, cy + 2,
				(status == 2) ? makecol(0,200,0) : makecol(255*(1-pct2),0,80*pct2));
		}
	}
}

void Ship::step()
{
	/* on fire */
	if (on_fire && on_fire != get_steps())
		destroy();
	if (on_hurt && on_hurt != get_steps())
		on_hurt = 0;

	/* skip if not active */
	if (!is_active())
		return;

	/* move */
	int dx, dy;

	bool w = world->move(get_x(), get_y(), get_dir(), dx, dy, true);
	
	/* update world maps */
	world->map_remove(get_x(), get_y());
	world->map_add(dx, dy);
	world->set_ship(dx, dy, this);

	/* set position */
	set_pos(dx, dy);
	if (w) reset_prev();

	if (!world->onmap(dx, dy)) destroy();

	/* update reload */
	if (reload_count > 0)
		reload_count--;
}

void Ship::update_state()
{
	/* increment step count */
	inc_steps();
	
	/* determine state */
	int state = check_state();

	int action = state_table->get_action(state);

	shoot_dir = -1;

	/* collision */
	if (world->get(get_x(), get_y()) > 1)
		hurt();

	/* set direction */
	switch (action) {
		case 3:
		case 4:
		case 5:
			inc_dir();
		break;

		case 6:
		case 7:
		case 8:
			dec_dir();
		break;

		default:
			set_dir(get_dir());
		break;
	}

	/* fire */
	switch (action) {
		case 1:
		case 4:
		case 7:
			fire(true);
		break;

		case 2:
		case 5:
		case 8:
			fire(false);
		break;

		default:
		break;
	}

}

void Ship::collision(Agent* a)
{
	hurt();
}

void Ship::hurt()
{
	lives--;
	world->clear_endgen();

	if (lives <= 0) {
		lives = 0;
		set_onfire();
	} else {
		set_onhurt();
	}
}

void Ship::destroy()
{
	lives = 0;
	world->clear_endgen();
	
	if (is_active()) {
		deactivate();
		world->map_remove(get_x(), get_y());
		world->add( new Reef(world, get_x(), get_y(), 1) );
	}
}

void Ship::fire(bool left_notright)
{
	if (reload_count) return;
	reload_count = reload_delay;

	int ndir = (get_dir() + (left_notright ? 2 : -2) + 8) % 8;

	shoot_dir = ndir;
	fires++;

	int dx = get_x();
	int dy = get_y();
	Agent* a;
	Ship* s;
	
	for (int i = 0; i < 4; i++) {
		world->move(dx, dy, ndir, dx, dy);
		a = world->get_ship(dx, dy);
		s = dynamic_cast<Ship*>(a);

		/* hit ship */
		if (s) {
			s->hurt();
			inc_score();
			break;
		}
	}
}

int Ship::check_state()
{
	int state = 0x00;
	int s = 0;
	int dist, type;

	for (int d = 6; d <= 10; d += 1) {
		check_dir((get_dir() + d) % 8, dist, type);
		state |= ( (((dist << 1) | type) << s) & (0x7 << s) );
		s += 2;
	}

	return state;
}

void Ship::check_dir(int d, int& rd, int& rt)
{
	int cx = get_x();
	int cy = get_y();
	int i;
	Agent* s;

	for (i = 0; i < 2; i++) {
		world->move(cx, cy, d, cx, cy);
		s = world->get_ship(cx, cy);
		/* the lookout spotted something */
		if (s) {
			//rd = i/2 + 1;
			/* danger of a broadside */
			//rt = ( ((s->get_dir() + 2) % 8 == d || (s->get_dir() + 6) % 8 == d) ? 1 : 0 );
			rd = 1;
			rt = (bool)dynamic_cast<Ship*>(s);
			return;
		}
	}

	rd = 0;
	rt = 0;
	return;
}

bool Ship::inheritable()
{
	return true;
}

void Ship::set_fleet(const char* fid)
{
	strncpy(fleetid, fid, 31);
	fleetid[31] = '\0';
}

void Ship::print_stats()
{
	printf(" %6d    %-9s\n", get_score(), fleetid);
}
