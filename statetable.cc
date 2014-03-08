#include "statetable.h"
#include <cstdlib>
#include <string>

double StateTable::mutation_rate = 0.001;

StateTable::StateTable(StateTable& p1, StateTable& p2)
{
	/* select random crossover section */
	int b = rand() % StateTable::max_states;
	int e = rand() % StateTable::max_states;
	if (e < b) {
		int t = e;
		e = b;
		b = t;
	}

	/* copy genome with crossing over */
	for (int i = 0; i < max_states; i++) {
		state_map[i] = (i < b || i > e) ? p1.get_action(i) : p2.get_action(i);
	}
	
	/* introduce mutation */
	int m = static_cast<int>(mutation_rate * max_states);

	/* handle really small mutation rates */
	if (!m && mutation_rate != 0.) {
		unsigned int d = static_cast<int>(1./mutation_rate/max_states + 0.5);
		if (d && !( rand() % d )) {
			m = 1;
		}
	}

	/* perform mutations */
	for (int i = 0; i < m; i++) {
		state_map[rand() % max_states] = rand() % max_actions;
	}
}

StateTable::StateTable()
{
	clear();
}

void StateTable::randomize()
{
	char* p = state_map + max_states - 1;

	while (p >= state_map) {
		*p-- = rand() % max_actions;
	}
}

void StateTable::clear()
{
	char* p = state_map + max_states - 1;

	while (p >= state_map) {
		*p-- = 0;
	}
}

void StateTable::write_to(std::ofstream& f)
{
	f << "START" << std::endl;

	for (int i = 0; i < max_states; i++) {
		f << static_cast<unsigned int>(state_map[i]) << std::endl;
	}
	
	f << "END" << std::endl;
}

bool StateTable::read_from(std::ifstream& f)
{
	int j;
	std::string s;

	f >> s;
	if (s != "START")
		return false;

	for (int i = 0; i < max_states; i++) {
		f >> j;
		if (j < 0 || j >= max_actions)
			return false;
		state_map[i] = j;
	}
	
	f >> s;
	if (s != "END")
		return false;

	return true;
}
