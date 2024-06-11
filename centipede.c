#include <qdos.h>

#include "image.h"

#define MUSHX 25
#define MUSHY 25

#define XPOS (256-MUSHX*8)/2

library lib;

sprite player;
sprite player_bullet;

sprite mushrooms[MUSHX][MUSHY];

static unsigned int g_seed;

void fastSrand(int seed)
{
        g_seed = seed;
}

// Compute a pseudorandom integer.
// Output value in range [0, 32767]

inline unsigned int fastRand(void)
{
        g_seed = (214013*g_seed+2531011);
        return (g_seed>>16);
}

void setupMushrooms()
{
	unsigned int i,x,y;

	for(x=0;x<MUSHX;x++)
		for(y=0;y<MUSHY;y++)
	{
		spriteSetup(&mushrooms[x][y],"Mushroom");
		spriteAddImageFromLibrary(&mushrooms[x][y],&lib,0);
		spriteAddImageFromLibrary(&mushrooms[x][y],&lib,1);
		spriteAddImageFromLibrary(&mushrooms[x][y],&lib,2);
		spriteAddImageFromLibrary(&mushrooms[x][y],&lib,3);

		mushrooms[x][y].active=0;
		mushrooms[x][y].currentImage=0;
		mushrooms[x][y].draw=1;
		mushrooms[x][y].mask=0;
		
		mushrooms[x][y].x=XPOS+x*8;
		mushrooms[x][y].y=8+y*8;
	}

	for(i=0;i<50;i++)
	{
		x=(fastRand()*MUSHX)/65536;
		y=(fastRand()*MUSHY)/65536;

		mushrooms[x][y].active=1;
		spritePlot(SCREEN,&mushrooms[x][y]);
	}
}

int main(int argc,char *argv[])
{
	fastSRand(0);
	init(8);
	loadLibrary(&lib,"centipede_lib",1);

        spriteSetup(&player,"Player");
        spriteAddImageFromLibrary(&player,&lib,4);

        player.active=1;
        player.currentImage=0;
        player.draw=1;
        player.mask=0;

        player.x=128;
        player.y=252-8;

        spriteSetup(&player_bullet,"PB");
        spriteAddImageFromLibrary(&player_bullet,&lib,9);

        player_bullet.active=0;
        player_bullet.currentImage=0;
        player_bullet.draw=1;
        player_bullet.mask=0;

	cls(SCREEN);

	setupMushrooms();
	spritePlot(SCREEN,&player);

	player.timer.value=getFrames();

	while(1)
	{
		unsigned int f=getFrames();

		if(f>player.timer.value) // Time to move the player?
		{
			unsigned int key=keyrow(1);
			unsigned int newx=player.x,newy=player.y;

			if((key&2)&&(player.x>XPOS)) newx-=2;
			if((key&16)&&(player.x<XPOS+MUSHX*8-8)) newx+=2;	
			if((key&4)&&(player.y>(256-5*8))) newy-=2;
			if((key&128)&&(player.y<(255-8-1))) newy+=2;

			// Move the player?
			if((player.x!=newx)||(player.y!=newy))
			{
				player.mask=1; player.draw=0;
				spritePlot(SCREEN,&player);

				player.x=newx; player.y=newy;

				player.mask=0; player.draw=1;
				spritePlot(SCREEN,&player);
			}

			// Fire a bullet?
			if((key&64)&&!player_bullet.active)
			{
				player_bullet.active=1;
				player_bullet.x=player.x+3;
				player_bullet.y=player.y-8;
				player_bullet.timer.value=f;
				spritePlot(SCREEN,&player_bullet);
			}

			player.timer.value=getFrames()+1;
		}

		// Move player bullet?
		if(player_bullet.active&&(f>player_bullet.timer.value))
		{
			player_bullet.mask=1; player_bullet.draw=0;
			spritePlot(SCREEN,&player_bullet);

			if(player_bullet.y<8)
			{
				player_bullet.active=0;
			}
			else
			{
				player_bullet.y-=8;

				if(mushrooms[(player_bullet.x-XPOS)/8][player_bullet.y/8].active)
				{
					sprite *m=&mushrooms[(player_bullet.x-XPOS)/8][player_bullet.y/8];
					m->mask=1; m->draw=0;
					spritePlot(SCREEN,m);

					if(m->currentImage==3)
					{
						m->currentImage=0;
						m->active=0;
					}
					else
					{
						m->currentImage++;
						m->mask=0; m->draw=1;
						spritePlot(SCREEN,m);
					}

					player_bullet.active=0;
				}
				else
				{
					player_bullet.mask=0; player_bullet.draw=1;
					spritePlot(SCREEN,&player_bullet);
				}
			}

			player_bullet.timer.value=f+1;
		}
	}
}
