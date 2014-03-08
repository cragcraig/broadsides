#ifndef WOT_SCREEN_H
#define WOT_SCREEN_H

#include <allegro.h>

class SCREEN
{
	private:
		BITMAP* video_page[3];
		BITMAP* offscreen;
		int video_page_offscreen;
		int w, h;
		bool doublebuffer;

	public:
		SCREEN(int width, int height, bool windowed=false, bool db=false);
		~SCREEN(void);
		BITMAP* flip_page(void);
		BITMAP* get_page(void);
		BITMAP* get_extrapage(void);
		BITMAP*& get_page_reference(void);

		void take_screenshot(const char* filename);

		int get_width(void);
		int get_height(void);
};

#endif
