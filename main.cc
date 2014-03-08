/*
 * Craig Harrison 2011
 * Genetic Broadsides
 */

#include <allegro.h>
#include <string>
#include "screen.h"
#include "world.h"
#include "img.h"
#include "ship.h"
#include "playership.h"
#include "statetable.h"

/* global endgame */
bool DIE_NOW = false;

/* functions */
int handle_flags(int argc, char* argv[], int& gen_size,
	double& mut_rate, bool& rand_seed, int& b_mode, bool& disp, int& wmax);
void init_libraries(int& w, int& h);
void exit_game(void);
void update_speed(int speed_state, int& step_length, const char*& speed_text);
void draw_screen(SCREEN* video_screen, World* world, int speed_state,
	const char* speed_text, int& step_length, int step_frame, bool show_status);
void update_keystates(World* world, int& speed_state, bool& show_status);
void initialize(World*& world, SCREEN*& video_screen, int argc, char* argv[], int& wmax);

/* timer */
volatile long speed_counter = 0;
void increment_speed_counter()
{
	speed_counter++;
}
END_OF_FUNCTION(increment_speed_counter);

/* main */
int main(int argc, char* argv[])
{
	World* world;
	SCREEN* video_screen;
	int wmax = 0;

	initialize(world, video_screen, argc, argv, wmax);

	/* states */
	bool show_status = false;
	int speed_state = 1;

	/* step control */
	int step_length;
	int step_frame = 0;
	const char* speed_text;
	
	/* init */
	speed_counter = 0;
	
	if (!video_screen) {
		speed_state = 6;
		step_length = 0;
	}

	/* main game loop */
	while (!DIE_NOW) {
		if (speed_counter > 0 || !step_length) {
			speed_counter--;

			/* update speed */
			if (video_screen)
				update_speed(speed_state, step_length, speed_text);

			/* update simulation */
			if (speed_state)
				step_frame++;
			if (step_frame >= step_length) {
				step_frame = 0;
				world->step();
			}

			/* draw */
			if (video_screen)
				draw_screen(video_screen, world, speed_state, speed_text, step_length, step_frame, show_status);
			
			/* keys */
			if (video_screen)
				update_keystates(world, speed_state, show_status);

			/* command line exit */
			if (wmax && world->get_actgen() > wmax) {
				world->write_file();
				DIE_NOW = true;
			}
		}
	}

	/* destructors must happen before allegro exits */
	delete world;
	if (video_screen)
		delete video_screen;

	if (video_screen)
		allegro_exit();

	return 0;
}
END_OF_MAIN();

void update_keystates(World* world, int& speed_state, bool& show_status)
{
	/* states */
	static bool sp_key = false;
	static bool st_key = false;
	static bool w_key = false;
	static bool p_key = false;
	
	/* exit */
	if (key[KEY_ESC])
		DIE_NOW = true;

	/* speed state */
	if ((key[KEY_LEFT] || key[KEY_RIGHT]) && !sp_key) {
		sp_key = true;
		speed_state += (key[KEY_LEFT] ? -1 : 1);
		if (speed_state > 6)
			speed_state = 6;
		if (speed_state < 0)
			speed_state = 0;
	}
	if (!key[KEY_LEFT] && !key[KEY_RIGHT])
		sp_key = false;

	/* status state */
	if (key[KEY_SPACE] && !st_key) {
		st_key = true;
		show_status = !show_status;
	}
	if (!key[KEY_SPACE])
		st_key = false;
	
	/* write file */
	if (key[KEY_W] && !w_key) {
		w_key = true;
		world->write_file();
	}
	if (!key[KEY_W])
		w_key = false;
		
	/* player ship */
	if (key[KEY_P] && !p_key) {
		p_key = true;
		if (!world->player_ship()) {
			world->add_random( new PlayerShip(world) );
		}
	}
	if (!key[KEY_P])
		p_key = false;
}

/* add drawing */
void draw_screen(SCREEN* video_screen, World* world, int speed_state, const char* speed_text, int& step_length, int step_frame, bool show_status)
{
	static bool s_key = false;
	
	if (!video_screen)
		return;
	
	BITMAP* buf = video_screen->get_page();

	/* draw simulation */
	if (step_length > 0) {
		world->render(video_screen->get_page(),
			(double)step_frame / step_length, show_status);
		textout_centre_ex(buf, font, speed_text,
			buf->w/2, 15, makecol(255,255,255), -1);
		video_screen->flip_page();
	}

	/* screenshot */
	if (key[KEY_S] && !s_key) {
		video_screen->take_screenshot("screenshot.bmp");
		s_key = true;
	}
	if (!key[KEY_S]) s_key = false;
	
	/* screen draw disabled */
	if (speed_state == 6 && (step_length || !(world->get_gen() & 0xff))) {
		buf = video_screen->get_page();
		clear_bitmap(video_screen->get_page());
		/* instant text */
		textout_centre_ex(buf, font, speed_text,
			buf->w/2, 15, makecol(255,255,255), -1);
		textout_centre_ex(buf, font, "RENDER DISABLED",
			buf->w/2, buf->h/2 - 5, makecol(255,255,255), -1);
		/* gen */
		world->draw_gen(buf);
		video_screen->flip_page();
		step_length = 0;
	}
}

/* update simulation speed */
void update_speed(int speed_state, int& step_length, const char*& speed_text)
{
	/* set speed */
	switch (speed_state) {	
		case 0:
			speed_text = "   PAUSE -->";
		break;

		case 1:
			step_length = 10;
			speed_text = "<-- SLOW -->";
		break;

		case 2:
			step_length = 5;
			speed_text = "<-- NORM -->";
		break;

		case 3:
			step_length = 3;
			speed_text = "<-- FAST -->";
		break;

		case 4:
			step_length = 1;
			speed_text = "<-- SPED -->";
		break;

		case 5:
			step_length = 1;
			speed_counter = 1;
			speed_text = "<-- FULL -->";
		break;

		case 6:
			speed_text = "<-- HYPER   ";
			speed_counter = 0;
			// step_length = 0; // set elsewhere
		break;

		default:
			speed_text = "error setting speed";
		break;
	}
}

/* library initialization */
void init_libraries(int& w, int& h)
{
	/* install allegro */
	set_uformat(U_ASCII);
	allegro_init();
	install_keyboard();
	install_timer();
	install_mouse();

	/* get default screen resolution */
	get_desktop_resolution(&w, &h);
	w = w * 4 / 5;
	h = h * 4 / 5;

	/* setup allegro stuff */
	LOCK_VARIABLE(speed_counter);
	LOCK_FUNCTION(increment_speed_counter);
	install_int_ex(increment_speed_counter, BPS_TO_TIMER(30));
	set_close_button_callback(exit_game);
	set_window_title("Broadsides");
	show_os_cursor(MOUSE_CURSOR_ARROW);
}

/* end program nicely */
void exit_game(void)
{
	DIE_NOW = true;
}

void initialize(World*& world, SCREEN*& video_screen, int argc, char* argv[], int& wmax)
{
	/* read flags */
	int gen_size = 0;
	double mut_rate = 0.001;
	bool rand_seed = false;
	bool disp = true;
	int battle_mode = 0;
	int num_ships = handle_flags(argc, argv, gen_size, mut_rate,
							rand_seed, battle_mode, disp, wmax);
	
	/* misc stuff */
	srand(time(0));

	/* setup screen */
	int width = 1500;
	int height = 1000;
	
	if (disp)
		init_libraries(width, height);

	video_screen = disp ? new SCREEN(width, height, true, false) : NULL;

	/* create world */
	world = new World(width/World::tile_size + 3, height/(World::tile_size/2) + 3,
						num_ships, battle_mode, disp);

	/* set global flag settings */
	StateTable::set_mutrate(mut_rate);

	/* seed world */
	StateTable* st;
	for (int i = 0; i < num_ships; i++) {
		st = new StateTable;
		if (rand_seed)
			st->randomize();
		world->add_random( new Ship(world, st) );
	}

	/* seed from files */
	if (!num_ships) {
		int nf = 0;
		int ns = 0;
		int sf = 0;
		std::string flag;
		for (int i = 1; i < argc; i++) {
			flag = argv[i];
			if (flag == "-f") {
				sf = i;
				break;
			}
		}
			
		for (int i = sf + 1; i < argc; i++) {
			if (argv[i][0] == '-')
				break;
			if (world->load_file(argv[i]))
				nf++;
			else
				fprintf(stderr, "warning: bad file '%s'\n", argv[i]);
		}

		ns = world->count_ships();
		if (ns < 3) {
			fprintf(stderr, "error: all bad files\n");
			DIE_NOW = true;
		} else if (gen_size) {
			world->set_gensize(gen_size);
		} else if (nf) {
			world->set_gensize(ns / nf);
		}
	}

	world->update_maxgensteps();
}

/* handle flags */
int handle_flags(int argc, char* argv[], int& gen_size, double& mut_rate, bool& rand_seed, int& b_mode, bool& disp, int& wmax)
{
	int num_ships = 20;
	bool n_flag = false;
	bool f_flag = false;
	bool m_flag = false;
	bool s_flag = false;
	bool r_flag = false;
	bool b_flag = false;
	bool h_flag = false;
	bool d_flag = false;
	bool w_flag = false;
	std::string flag;
	
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			flag = argv[i];

			/* help */
			if (flag == "-h") {
				h_flag = true;
				break;
			}

			/* number */
			if (flag == "-n" && i + 1 < argc) {
				sscanf(argv[i + 1], "%u", &gen_size);
				if (gen_size < 3)
					gen_size = 3;
				if (gen_size > 1000)
					gen_size = 1000;
				n_flag = true;
			}

			/* mutation rate */
			if (flag == "-m" && i + 1 < argc) {
				sscanf(argv[i + 1], "%lf", &mut_rate);
				if (mut_rate < 0.)
					mut_rate = 0.;
				if (mut_rate > 1.)
					mut_rate = 1.;
				m_flag = true;
			}

			/* random seed */
			if (flag == "-r") {
				rand_seed = true;
				r_flag = true;
			}
			
			/* disable display */
			if (flag == "-d") {
				d_flag = true;
			}

			/* command line mode */
			if (flag == "-w" && i + 1 < argc) {
				w_flag = true;
				sscanf(argv[i + 1], "%u", &wmax);
			}
	
			/* battle mode */
			if (flag == "-b") {
				if (i + 1 < argc)
					sscanf(argv[i + 1], "%d", &b_mode);
				if (!b_mode)
					b_mode = 1;
				b_mode = abs(b_mode);
				b_flag = true;
			}
				
			/* fitness function */
			if (flag == "-s" && i + 1 < argc) {
				sscanf(argv[i + 1], "%d:%d:%d:%d", &Ship::A, &Ship::B, &Ship::C, &Ship::D);
				s_flag = true;
			}

			/* load */
			if (flag == "-f" && i + 1 < argc) {
				f_flag = true;
			}
		}
		
		/* disable display during battle mode */
		if ((d_flag && b_mode > 0) || w_flag)
			disp = false;
		
		/* number of random ships to generate */
		if (f_flag)
			num_ships = 0;
		else if (gen_size)
			num_ships = gen_size;

		/* help */
		if (h_flag || (!n_flag && !f_flag && !m_flag && !s_flag && !r_flag && !b_flag && disp)) {
			printf("usage:\n\n broadsides [flags]\n\n");
			printf(" -h\n     Print this help message and exit.\n\n");
			printf(" -r\n     Randomly seed starting ship states.\n");
			printf("     By default states start cleared.\n\n");
			printf(" -b [<n>]\n     Battle mode. Run n times and\n");
			printf("     and print results to stdout.\n");
			printf("     Special results when n is 1.\n\n");
			printf(" -d \n     Disable graphics in battle mode.\n\n");
			printf(" -w <g> \n     Run for g generations with no\n");
			printf("     display and save resulting fleet.\n\n");
			printf(" -f <infile1> [<infile2> [...]]\n     Seed from fleet files. Multiple\n");
			printf("     loaded fleets will be merged.\n\n");
			printf(" -n <ships>\n     Fleet size. Defaults to 20 or the\n");
			printf("     average size of all loaded fleets.\n\n");
			printf(" -m <rate>\n     Mutation rate in the range 0 to 1.\n");
			printf("     Defaults to 0.001.\n\n");
			printf(" -s <A:B:C:D>\n     Fitness function coefficients.\n");
			printf("     Defaults to A=%d B=%d C=%d D=%d.\n", Ship::A, Ship::B, Ship::C, Ship::D);
			printf("     A*hits B*shots C*days D*lives\n\n");
			exit(0);
		}
	}

	return num_ships;
}
