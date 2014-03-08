#ifndef WORLD_H
#define WORLD_H

#include <allegro.h>
#include <list>
#include <vector>
#include <map>
#include <string>
#include "agent.h"
#include "img.h"

class Ship;
class PlayerShip;

typedef std::map<std::string, long unsigned int> stringmap;

class World
{
	public:
		World(int w, int h, int gensize, int battle_mode=0, bool load_graphics=true);
		~World();

		/* graphics */
		Img img_data;

		/* tile size */
		static const int tile_size = 55;

		/* update functions */
		void step();
		void render(BITMAP* buffer, double step_fraction=1.0, bool status=true);
		void draw_gen(BITMAP* buffer);
		long int get_steps() const { return step_count; }
		long int get_gen() const { return gen_count; }
		long int get_actgen() const { return act_gen_count; }

		/* map functions */
		void clear_map();
		bool onmap(int x, int y) { return (x >= 0 && x < width && y >= 0 && y < height); }
		void map_add(int x, int y) { if (onmap(x, y)) map[x][y]++; }
		void map_remove(int x, int y) { if (onmap(x, y)) map[x][y]--; }
		int get(int x, int y) { return (onmap(x, y) ? map[x][y] : -1); }
		bool move(int from_x, int from_y, int dir, int& to_x, int& to_y, bool wrap=true);
		void resolve_collision(int x, int y);
		void toscreen(int x, int y, int& screen_x, int& screen_y);

		/* size */
		int get_height() const { return height; }
		int get_width() const { return width; }

		void set_gensize(int g) { gen_size = (g < 3) ? 3 : g; }

		/* add agent */
		void add(Agent* a);
		void add_random(Agent* a);

		/* ship map */
		Agent* get_ship(int x, int y);
		void set_ship(int x, int y, Agent* s);

		/* state */
		int count_ships();
		PlayerShip* player_ship();
		bool load_file(const char* fn);
		bool write_file(const char* fn=NULL);
		void clear_endgen() { end_gen = 0; }
		void reset();
		void new_generation();
		void update_maxgensteps();

	private:
		Ship* get_parent(std::vector<Ship*>& ship_list, int max_score);		
		int battle_mode;
		long int step_count;
		long int gen_count;
		long int act_gen_count;
		int end_gen;
		int gen_size;
		int width, height;
		int** map;
		void clear_shipmap();
		void reset_shipmap();
		Agent** ship_map;
		std::list<Agent*> agents;
		std::list<Agent*> reefs;
		stringmap results;
		stringmap rcount;
		stringmap rtop;
		stringmap rtmax;

		void print_allexit();
		void print_allexit1();

		int max_gen_steps;
		static const int keepers = 3;
};

#endif
