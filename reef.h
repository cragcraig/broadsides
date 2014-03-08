#ifndef REEF_H
#define REEF_H

#include "agent.h"

class Reef : public Agent
{
	public:
		void render(BITMAP* buffer, double step_fraction, int status);
		void step();
		void collision(Agent* a);
		void set_pos(int to_x, int to_y);
		Reef(World* world);
		Reef(World* world, int x, int y, int type);
		~Reef() {};

	private:
		int type;
};

#endif
