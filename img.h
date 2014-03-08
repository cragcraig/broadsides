#ifndef IMG_H
#define IMG_H

#include <allegro.h>

class Img
{
	public:
		Img();
		~Img();
		
		void init();

		BITMAP* ship[8];
		BITMAP* shoot[8];
		BITMAP* ball[8];
		BITMAP* fire[2];
		BITMAP* reef[2];
		BITMAP* water;
	
	private:
		bool load(BITMAP* imgs[], int n, const char* fname_base);
		void destroy(BITMAP* imgs[], int n);
		bool initd;
};

#endif
