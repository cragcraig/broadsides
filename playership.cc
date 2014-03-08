#include "playership.h"
#include "world.h"

PlayerShip::PlayerShip(World* world) :
	Ship(world, NULL),
	d_key(0),
	f_key(0),
	kd_step(0),
	kf_step(0)
{
	set_fleet("*human*");
}

void PlayerShip::render(BITMAP* buffer, double step_fraction, int status)
{

	/* turn left */
	if (key[KEY_A] && !d_key) {
		d_key = 1;
		kd_step = get_steps();
	}
	
	/* turn right */
	if (key[KEY_D] && !d_key) {
		d_key = 2;
		kd_step = get_steps();
	}

	if (!key[KEY_A] && !key[KEY_D] && kd_step != get_steps())
		d_key = 0;

	/* fire left */
	if (key[KEY_J] && !f_key) {
		f_key = 1;
		kf_step = get_steps();
	}
	
	/* fire right */
	if (key[KEY_L] && !f_key) {
		f_key = 2;
		kf_step = get_steps();
	}
	
	if (!key[KEY_J] && !key[KEY_L] && kf_step != get_steps())
		f_key = 0;

	/* render */
	Ship::render(buffer, step_fraction, 2);
}

bool PlayerShip::inheritable()
{
	return false;
}

void PlayerShip::update_state()
{
	inc_steps();

	shoot_dir = -1;

	/* turn */
	switch (d_key) {
		case 1:
			dec_dir();
		break;

		case 2:
			inc_dir();
		break;

		default:
			set_dir(get_dir());
		break;
	}

	/* fire */
	if (f_key)
		fire(f_key == 1);
}
