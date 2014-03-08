#ifndef PLAYERSHIP_H
#define PLAYERSHIP_H

#include "agent.h"
#include "ship.h"

class PlayerShip : public Ship
{
	public:
		PlayerShip(World* world);

		void render(BITMAP* buffer, double step_fraction, int status);
		bool inheritable();
		void update_state();
	
	private:
		int d_key;
		int f_key;
		int kd_step;
		int kf_step;
};

#endif
