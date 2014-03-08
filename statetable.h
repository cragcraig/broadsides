#ifndef STATETABLE_H
#define STATETABLE_H

#include <cstdio>
#include <iostream>
#include <fstream>

class StateTable
{
	public:
		/* settings */
		static const int max_states = 1024;
		static const int max_actions = 9;
		static void set_mutrate(double mr) { StateTable::mutation_rate = mr; }

		/* functions */
		StateTable();
		StateTable(StateTable& p1, StateTable& p2);
		
		void randomize();
		void clear();
		
		int get_action(int state) const
		{
			if (state < max_states)
				return state_map[state];
			fprintf(stderr, "error: bad state check\n");
			return 0;
		};
		
		int set_action(int state, int action)
			{ if (state < max_states) state_map[state] = action; }

		/* save and load */
		void write_to(std::ofstream& f);
		bool read_from(std::ifstream& f);

	private:
		static double mutation_rate;
		char state_map[max_states];
};

#endif
