#ifndef AGENT_H
#define AGENT_H

#include <allegro.h>

class World;

class Agent
{
	public:
		virtual void render(BITMAP* buffer, double step_fraction, int status) = 0;
		virtual void step() = 0;
		virtual void collision(Agent* a) = 0;
		virtual void hurt() { }
		virtual void destroy() { }
		virtual void update_state() { }

		Agent(World* world, int x, int y, int dir, bool startmove=false);
		virtual ~Agent() { };
		
		/* get */
		int get_x() const { return x; }
		int get_y() const { return y; }
		int get_prevx() const { return prev_x; }
		int get_prevy() const { return prev_y; }
		int get_dir() const { return dir; }
		int get_prevdir() const { return prev_dir; }

		/* active and dead*/
		bool is_active() const { return active; }
		void deactivate() { active = false; }
		bool is_dead() const { return dead; }
		void kill() { dead = true; }

		/* set */
		void set_world(World* w) { if (!world) world = w; }
		virtual void set_pos(int to_x, int to_y);

		void set_dir(int v) { prev_dir = dir; dir = (v + 8) % 8; }
		void reset_prev() { prev_x = x; prev_y = y; }

		/* turn */
		void inc_dir() { set_dir(dir + 1); };
		void dec_dir() { set_dir(dir - 1); };

		/* lifespan */
		long int get_steps() const { return steps; }
	
	protected:
		void inc_steps() { steps++; }
		World* world;

	private:
		int x, y, prev_x, prev_y, prev_dir, dir;
		void set_x(int v) { x = v; }
		void set_y(int v) { y = v; }
		long int steps;
		bool active;
		bool dead;
};

#endif
