#ifndef SHIP_H
#define SHIP_H

#include "agent.h"

class StateTable;

class Ship : public Agent
{
	public:
		void render(BITMAP* buffer, double step_fraction, int status);
		void step();
		void collision(Agent* a);
		void hurt();
		void destroy();
		Ship(World* world, StateTable* st, int x=0, int y=0, int dir=0, int lives=10, int reload_delay=3);
		~Ship();

		void inc_score() { score++; }
		
		virtual bool inheritable();
		void update_state();
		StateTable& get_statetable() { return *state_table; }
		
		/* fitness */
		static int A, B, C, D;

		int get_score()
		{
			int s = Ship::A*score + Ship::B*fires + Ship::C*get_steps() + Ship::D*lives;
			return (s < 0) ? 0 : s;
		}

		/* battle mode */
		void set_fleet(const char* fid);
		const char* get_fleet() const { return fleetid; };
		void print_stats();

	protected:
		void fire(bool left_notright);
		void set_onfire() { if (!on_fire) on_fire = get_steps(); }
		void set_onhurt() { on_hurt = get_steps(); }
		int check_state();
		int shoot_dir;

	private:
		void check_dir(int d, int& rd, int& rt);
		int lives;
		int start_lives;
		int score;
		int fires;
		int reload_delay, reload_count;
		int on_fire, on_hurt;
		char fleetid[32];

		StateTable* state_table;
};

#endif
