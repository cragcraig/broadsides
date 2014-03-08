#ifndef BALL_H
#define BALL_H

#include "agent.h"

class Ship;

class Ball : public Agent
{
	public:
		void render(BITMAP* buffer, double step_fraction, int status);
		void step();
		void collision(Agent* a);
		void hurt();
		void destroy();
		Ball(World* world, Ship* s, int x, int y, int dir, bool startmove=false) : Agent(world, x, y, dir, startmove), source(s) { }
		~Ball() {};

		void inform_hit();

	private:
		Ship* source;
		
		static const int speed = 2;
		static const int distance = 2;
};

#endif
