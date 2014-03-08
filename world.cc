#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <string>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include "world.h"
#include "ship.h"
#include "playership.h"
#include "reef.h"
#include "statetable.h"

World::World(int w, int h, int gensize, int battle_mode, bool load_graphics) :
	width(w),
	height(h),
	step_count(0),
	gen_count(0),
	act_gen_count(1),
	end_gen(0),
	max_gen_steps(25),
	battle_mode(battle_mode)
{
	set_gensize(gensize);

	/* load graphics */
	if (load_graphics)
		img_data.init();

	/* create map array */
	map = new int*[width];

	for (int i = 0; i < width; i++) {
		map[i] = new int[height];
		/* clear map column */
		for (int j = 0; j < height; j++) {
			map[i][j] = 0;
		}
	}

	/* create ship map array */
	ship_map = new Agent*[width * height];

	/* start */
	reset();
}

World::~World()
{
	/* destroy map */
	for (int i = 0; i < width; i++) {
		delete [] map[i];
	}

	delete [] map;
	
	/* destroy ship map */
	delete [] ship_map;

	/* destroy agent list */
	std::list<Agent*>::iterator j;
	for (j = agents.begin(); j != agents.end(); j++)
		delete *j;
	
	/* delete reefs */
	for (j = reefs.begin(); j != reefs.end(); j++)
		delete *j;
}


void World::render(BITMAP* buffer, double step_fraction, bool status)
{
	/* clear background */
	clear_to_color(buffer, makecol(1, 91, 154));

	/* draw background */
	for (int j = 0; j < buffer->w; j += img_data.water->w) {
		for (int k = 0; k < buffer->h; k += img_data.water->h) {
			blit(img_data.water, buffer, 0, 0, j, k, img_data.water->w, img_data.water->h);
		}
	}

	/* render all reefs */
	std::list<Agent*>::iterator i;
	for (i = reefs.begin(); i != reefs.end(); i++) {
		if ((*i)->is_active())
			(*i)->render(buffer, step_fraction, status);
	}

	/* render all agents */
	for (i = agents.begin(); i != agents.end(); i++) {
		if ((*i)->is_active())
			(*i)->render(buffer, step_fraction, status);
	}

	/* text */
	textout_ex(buffer, font, "[SPACE] toggles stats",
		buffer->w - 195, 5, makecol(255,255,255), -1);
	
	textout_ex(buffer, font, "[W] saves to file",
		buffer->w - 195, 18, makecol(255,255,255), -1);
	
	textout_ex(buffer, font, "[S] saves screenshot",
		buffer->w - 195, 31, makecol(255,255,255), -1);
	
	textout_ex(buffer, font, "[P] injects manual ship",
		buffer->w - 195, 50, makecol(255,255,255), -1);
	
	textout_ex(buffer, font, "    [A] and [D] steer",
		buffer->w - 195, 63, makecol(255,255,255), -1);
	
	textout_ex(buffer, font, "    [J] and [L] fire",
		buffer->w - 195, 76, makecol(255,255,255), -1);

	draw_gen(buffer);

	textprintf_ex(buffer, font, 10, 18, makecol(255,255,255), -1,
		"Day: %3u", (unsigned int)step_count);
	
	textprintf_ex(buffer, font, 10, 31, makecol(255,255,255), -1,
		"Next: %2u/%u", (unsigned int)end_gen, (unsigned int)max_gen_steps);
}

void World::draw_gen(BITMAP* buffer)
{
	if (battle_mode > 0) {
		textprintf_ex(buffer, font, 10, 5, makecol(255,255,255), -1,
			"Battle: %u of %u", (unsigned int)act_gen_count, (unsigned int)battle_mode);
	} else {
		textprintf_ex(buffer, font, 10, 5, makecol(255,255,255), -1,
			"Fleet: %u", (unsigned int)gen_count);
	}
}

void World::step()
{
	int shipcount = 0;
	int deads = 0;

	/* perform actions */
	std::list<Agent*>::iterator n;
	for (n = agents.begin(); n != agents.end(); n++) {
		if ((*n)->is_active())
			(*n)->update_state();
	}
	
	/* clear ship map */
	clear_shipmap();

	/* step all agents */
	std::list<Agent*>::reverse_iterator i;
	for (i = agents.rbegin(); i != agents.rend(); i++) {
		if ((*i)->is_active()) {
			shipcount++;
			(*i)->step();
		}
		if ((*i)->is_dead())
			deads++;
	}

	/* delete dead agents */
	if (deads) {
		std::list<Agent*>::iterator m;
		for (n = agents.begin(); n != agents.end(); ) {
			m = n;
			n++;
			if ((*m)->is_dead()) {
				delete *m;
				agents.erase(m);
			}
		}
	}

	/* increment step count */
	step_count++;

	if (++end_gen > max_gen_steps || shipcount < 2) {
		act_gen_count++;
		if (!battle_mode || act_gen_count <= battle_mode)
			new_generation();
		else if (battle_mode > 1)
			print_allexit();
		else
			print_allexit1();
	}
}

void World::resolve_collision(int x, int y)
{
	if (!onmap(x, y) || map[x][y] < 2) return;

	std::vector<Agent*> v;
	v.reserve(map[x][y]);

	/* create vector */
	std::list<Agent*>::iterator i;
	for (i = agents.begin(); i != agents.end(); i++) {
		if ((*i)->is_active() && (*i)->get_x() == x && (*i)->get_y() == y) {
			v.push_back(*i);
		}
	}

	/* perform collision */
	for (int j = 0; j < v.size(); j++) {
		for (int k = 0; k < v.size(); k++) {
			if (j != k) {
				v[k]->collision(v[j]);
			}
		}
	}
}

/* place a new agent in the world */
void World::add(Agent* a)
{
	a->set_world(this);
	
	if (dynamic_cast<Ship*>(a)) {
		agents.push_back(a);
	} else {
		reefs.push_back(a);
	}

	map_add(a->get_x(), a->get_y());
}

/* add agent with random placement */
void World::add_random(Agent* a)
{
	int px, py;

	do {
		px = rand()%width;
		py = rand()%height;
	} while (get(px, py));

	a->set_pos(px, py);
	a->set_dir(rand());
	a->reset_prev();

	add(a);
}

/* hex movement */
bool World::move(int from_x, int from_y, int dir, int& to_x, int& to_y, bool wrap)
{
	dir = (dir + 8) % 8;
	
	to_x = from_x;
	to_y = from_y;
	
	/* x */
	switch (dir) {
		case 0:
		case 1:
		case 7:
			to_x++;
		break;
		
		case 3:
		case 4:
		case 5:
			to_x--;
		break;
	}

	/* y */
	switch (dir) {
		case 1:
		case 2:
		case 3:
			to_y--;
		break;
		
		case 5:
		case 6:
		case 7:
			to_y++;
		break;
	}

	/* wrap */
	if (!wrap || onmap(to_x, to_y))
		return false;
	
	if (to_y >= height)
		to_y -= height;
	if (to_y < 0)
		to_y += height;
	
	if (to_x >= width)
		to_x -= width;
	if (to_x < 0)
		to_x += width;

	return true;
}

/* convert hex coordinates to screen coordinates */
void World::toscreen(int x, int y, int& screen_x, int& screen_y)
{
	screen_x = x * tile_size - tile_size;
	screen_y = y * tile_size/2 - tile_size/2;
}

/* ship map */
void World::clear_shipmap()
{
	/* clear except reefs */
	for (Agent** p = ship_map + width * height - 1; p >= ship_map; p--) {
		if (*p && dynamic_cast<Ship*>(*p)) {
			*p = NULL;
		}
	}
}

/* reset ship map */
void World::reset_shipmap()
{
	/* clear */
	for (Agent** p = ship_map + width * height - 1; p >= ship_map; p--) {
			*p = NULL;
	}
}

Agent* World::get_ship(int x, int y)
{
	if (!onmap(x, y)) return NULL;
	return ship_map[x * width + y];
}

void World::set_ship(int x, int y, Agent* s)
{
	if (!onmap(x, y)) return;
	Agent* a = ship_map[x * width + y];
	if (!a || dynamic_cast<Reef*>(s))
		ship_map[x * width + y] = s;
}

void World::clear_map()
{
	for (int j = 0; j < width; j++) {
		for (int k = 0; k < height; k++) {
			map[j][k] = 0;
		}
	}
}

/* reset world */
void World::reset()
{
	/* clear maps */
	clear_map();
	
	/* delete agents */
	std::list<Agent*>::iterator j;
	for (j = agents.begin(); j != agents.end(); j++)
		delete *j;
	
	agents.clear();

	/* update state */
	gen_count++;
	end_gen = 0;
	step_count = 0;
	
	/* delete reefs */
	for (j = reefs.begin(); j != reefs.end(); j++)
		delete *j;

	reefs.clear();

	reset_shipmap();
	
	/* create reefs */
	for (int i = 0; i < 15; i++)
		add_random(new Reef(this));
}

bool ship_compare(Ship* s1, Ship* s2)
{
	return s1->get_score() < s2->get_score();
}

/* compute next generation */
void World::new_generation()
{
	std::vector<Ship*> ship_list;
	ship_list.reserve(agents.size());
	std::list<Ship*> new_list;
	int score = 0;

	/* clear max count */
	if (battle_mode > 0) {
		stringmap::iterator km;
		for (km = rtmax.begin(); km != rtmax.end(); km++)
			km->second = 0;
	}

	/* construct ship list */
	std::list<Agent*>::iterator j;
	for (j = agents.begin(); j != agents.end(); j++) {
		Ship* sp = dynamic_cast<Ship*>(*j);
		if (sp && sp->inheritable()) {
			ship_list.push_back(sp);
			score += sp->get_score() + 1;
			if (battle_mode > 0) {
				results[sp->get_fleet()] += sp->get_score();
				++rcount[sp->get_fleet()];
				if (rtmax[sp->get_fleet()] < sp->get_score())
					rtmax[sp->get_fleet()] = sp->get_score();				
			}
		}
	}

	if (battle_mode > 0) {
		for (int i = 0; i < ship_list.size(); i++) {
			Ship* sp = new Ship(this, new StateTable(ship_list[i]->get_statetable()) );
			sp->set_fleet(ship_list[i]->get_fleet());
			new_list.push_back(sp);
		}
	} else {
		/* sort list */
		if (score > INT_MAX)
			fprintf(stderr, "error: sum_scores > INT_MAX\n");

		std::sort(ship_list.begin(), ship_list.end(), ship_compare);
	
		StateTable* st;
	
		/* keep top few as is */
		for (int i = 0; i < keepers; i++) {
			if (i == ship_list.size())
				break;

			/* add */
			st = new StateTable( ship_list[i]->get_statetable() );
			new_list.push_back( new Ship(this, st) );
		}

		/* fill with children */
		int n = gen_size - new_list.size();
		Ship *p1, *p2;

		for (int i = 0; i < n; i++) {

			p1 = get_parent(ship_list, score);		
			p2 = get_parent(ship_list, score);		
	
			st = new StateTable(p1->get_statetable(), p2->get_statetable());
			new_list.push_back( new Ship(this, st) );
		}
	}

	/* launch new generation */
	reset();
	
	/* add next gen ships */
	std::list<Ship*>::iterator k;
	for (k = new_list.begin(); k != new_list.end(); k++) {
		add_random(*k);
	}

	/* update max counts */
	if (battle_mode > 0) {
		stringmap::iterator km;
		for (km = rtmax.begin(); km != rtmax.end(); km++)
			rtop[km->first] += km->second;
	}

	update_maxgensteps();
}

Ship* World::get_parent(std::vector<Ship*>& ship_list, int max_score)
{
	int s = rand() % max_score;

	for (int i = ship_list.size() - 1; i >= 0; i--) {
		s -= ship_list[i]->get_score() + 1;

		if (s <= 0)
			return ship_list[i];
	}

	return ship_list[ship_list.size() - 1];
}

bool World::load_file(const char* fn)
{
	StateTable s;
	Ship* sp;
	bool e;
	int sa = 0;
	std::string name(fn);

	/* create good name */
	int p1 = name.find("fleet_");
	int p2 = name.find(".shp");
	if (p1 == std::string::npos)
		p1 = 0;
	else
		p1 = 6;

	name = name.substr(p1, p2 - p1);

	/* open file */
	std::ifstream f(fn, std::ifstream::in | std::ifstream::binary);

	if (!f.good())
		return false;

	/* read number of generations */
	long int g;
	f >> g;
	gen_count += g;

	/* read ships */
	do {
		e = s.read_from(f);
		if (e) {
			sp = new Ship(this, new StateTable(s));
			sp->set_fleet(name.c_str());
			add_random(sp);
			sa++;
		}
	} while (e);

	f.close();

	/* init scores */
	if (sa > 0) {
		results[name] = 0;
		rcount[name] = 0;
		rtop[name] = 0;
		rtmax[name] = 0;
	}

	return (sa > 0);
}

bool World::write_file(const char* fn)
{
	/* use gen_count as filename */
	char fnb[1024];
	if (!fn) {
		snprintf(fnb, 1024, "fleet_%lu.shp", gen_count);
		fn = fnb;
	}

	printf("save: '%s'\n", fn);
	
	Ship* s;

	std::ofstream f(fn, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

	if (!f.good())
		return false;

	/* write number of generations */
	f << gen_count << std::endl;

	/* write out ships */
	std::list<Agent*>::iterator j;
	for (j = agents.begin(); j != agents.end(); j++) {
		s = dynamic_cast<Ship*>(*j);
		if (s && s->inheritable()) {
			s->get_statetable().write_to(f);
		}
	}

	f.close();

	return true;
}

int World::count_ships()
{
	int r = 0;
	Ship* s;

	std::list<Agent*>::iterator j;
	for (j = agents.begin(); j != agents.end(); j++) {
		s = dynamic_cast<Ship*>(*j);
		if (s && s->inheritable()) {
			r++;
		}
	}

	return r;
}

PlayerShip* World::player_ship()
{
	PlayerShip* s;

	std::list<Agent*>::iterator j;
	for (j = agents.begin(); j != agents.end(); j++) {
		s = dynamic_cast<PlayerShip*>(*j);
		if (s && s->is_active())
			return s;
	}

	return NULL;
}

void World::update_maxgensteps()
{
	max_gen_steps = width * height / (count_ships() * 2) + 3;

	if (max_gen_steps < 15)
		max_gen_steps = 15;

	if (max_gen_steps > 100)
		max_gen_steps = 100;
}

extern bool DIE_NOW;

void World::print_allexit()
{
	/* add last round */
	std::list<Agent*>::iterator j;
	for (j = agents.begin(); j != agents.end(); j++) {
		Ship* sp = dynamic_cast<Ship*>(*j);
		if (sp && sp->inheritable()) {
			results[sp->get_fleet()] += sp->get_score();;
			++rcount[sp->get_fleet()];
		}
	}
	
	printf("\n");
	printf("  Avg Top | Avg Ship |  Total  |  Fleet Id\n");
	printf(" --------- ---------- --------- ------------\n");
	stringmap::iterator i;
	for (i = results.begin(); i != results.end(); i++) {
		double as = results[i->first] / static_cast<double>(rcount[i->first]);
		double at = rtop[i->first] / static_cast<double>(act_gen_count);
		printf(" %8.2lf  %8.2lf   %8lu   %-11s\n", at, as, i->second, i->first.c_str());
	}

	printf("\n");

	/* end game */
	DIE_NOW = true;
}

bool ship_rcompare(Ship* s1, Ship* s2)
{
	return s2->get_score() < s1->get_score();
}

void World::print_allexit1()
{
	
	std::vector<Ship*> ships;
	ships.reserve(agents.size());

	/* construct ship list */
	std::list<Agent*>::iterator j;
	for (j = agents.begin(); j != agents.end(); j++) {
		Ship* sp = dynamic_cast<Ship*>(*j);
		if (sp) {
			ships.push_back(sp);
		}
	}
	
	std::sort(ships.begin(), ships.end(), ship_rcompare);

	/* print all */
	printf("  Score  |  Fleet Id\n");
	printf(" -------   ----------\n");

	for (int i = 0; i < ships.size(); i++)
		ships[i]->print_stats();

	/* end game */
	DIE_NOW = true;
}
