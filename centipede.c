#include <qdos.h>

#include "image.h"

#define MUSHX 25
#define MUSHY 25

#define XMIN ((256-MUSHX*8)/2)
#define XMAX (256-(256-MUSHX*8)/2)

library lib;

sprite player,player_bullet,dropper,spider;
sprite centipede[12];
sprite mushrooms[MUSHX][MUSHY];
sprite numbers;

unsigned int hiscore=0,score,lives;

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

unsigned int divu10(unsigned int n)
{
    unsigned q, r;

    q = (n >> 1) + (n >> 2);
    q = q + (q >> 4);
    q = q + (q >> 8);
    q = q + (q >> 16);
    q = q >> 3;
    r = n - (((q << 2) + q) << 1);

    return q + (r > 9);
}

// Fast *10

unsigned int mul10(unsigned int z)
{
        return (z<<3)+(z<<1);
}

void scorePrint(unsigned int x,unsigned int y,unsigned int digits,unsigned int z)
{
        int a;
        unsigned int zz;

	numbers.x=x;
	numbers.y=y;

        for(a=digits;a>=0;a--)
        {
                zz=divu10(z);
		numbers.currentImage=z-mul10(zz);
                z=zz;

		spritePlot(SCREEN,&numbers);
		numbers.x-=8;

		if(z==0) break;
        }
}

void printScore()
{
	unsigned int a;
	fill(SCREEN,0,8,0);
	scorePrint(5*8,0,5,score);

	numbers.currentImage=10;
	numbers.x=5*8+8;

	for(a=1;a<lives;a++)
	{
		spritePlot(SCREEN,&numbers);
		numbers.x+=8;
	}

	scorePrint(128+1*8,0,5,hiscore);
}

struct
{
	unsigned int head;
	unsigned int set;
	unsigned int position;
	unsigned int direction;
} animations[12];

const int centipedeAnimation[2][8]={{7,8,9,10,11,10,9,8},{1,2,3,4,5,4,3,2}};

void initCentipede()
{
	unsigned int i,j;
	unsigned int images[]={5 ,26,25, 6, 7,24,
			       27,28,29,30,31,32};

	for(i=0;i<12;i++)
	{
		spriteSetupFull(&centipede[i],"C",0,0,1);

		for(j=0;j<12;j++)
			spriteAddImageFromLibrary(&centipede[i],&lib,images[j]);
	}
}

void setupCentipede(unsigned int frames)
{
	unsigned int i,si=6,sd=-1;

	for(i=0;i<12;i++)
	{
		centipede[i].active=(i==0);

		centipede[i].currentImage=si;

		if(si==6)
			si=7;
		else
		{
			if((si==11)||(si==7)) sd=-sd;

			si+=sd;
		}

		centipede[i].x=128;
		centipede[i].y=8;
		centipede[i].dy=1;
		centipede[i].dx=2;

		centipede[i].timer.value=frames+6*i;
		centipede[i].timer2.value=1; // Alive flag

		animations[i].head=i==0;
		animations[i].set=1;
		animations[i].position=0;
		animations[i].direction=1;
	 }
}

void initMushrooms()
{
	unsigned int i,x,y;

	for(x=0;x<MUSHX;x++)
		for(y=0;y<MUSHY;y++)
	{
		spriteSetupFull(&mushrooms[x][y],"Mushroom",0,0,1);

		spriteAddImageFromLibrary(&mushrooms[x][y],&lib,0);
		spriteAddImageFromLibrary(&mushrooms[x][y],&lib,1);
		spriteAddImageFromLibrary(&mushrooms[x][y],&lib,2);
		spriteAddImageFromLibrary(&mushrooms[x][y],&lib,3);

		mushrooms[x][y].x=XMIN+x*8;
		mushrooms[x][y].y=8+y*8;
	}
}

void setupMushRooms()
{
	unsigned int i,x,y;

	for(i=0;i<50;i++)
	{
		x=(fastRand()*MUSHX)/65536;
		y=(fastRand()*MUSHY)/65536;

		mushrooms[x][y].active=1;
		spritePlot(SCREEN,&mushrooms[x][y]);
	}
}

void runCentipede(unsigned int frames)
{
	unsigned int i;

	for(i=0;i<12;i++)
	{
		if(centipede[i].timer2.value)
		{
			if(centipede[i].active)
			{
				if(centipede[i].timer.value<frames)
				{
					centipede[i].draw=0;
					spritePlot(SCREEN,&centipede[i]);

					centipede[i].x+=centipede[i].dx;
					if((centipede[i].x==XMIN)
					  ||(centipede[i].x==XMAX-8)
                                          ||mushrooms[(centipede[i].x-XMIN+(centipede[i].dx>0?6:0))/8][(centipede[i].y-8)/8].active)
					{
						centipede[i].dx=-centipede[i].dx;
						centipede[i].y+=centipede[i].dy*8; // BODGE
						animations[i].set=1-animations[i].set;

						if(animations[i].head)
							centipede[i].currentImage=(animations
[i].set==0)?0:6;
					}

					if(centipede[i].y==256-8)
					{
						centipede[i].dy=-1;
					}

					// Set animation frame
				
	
					if(!animations[i].head)
					{
						animations[i].position+=animations[i].direction;

						centipede[i].currentImage=centipedeAnimation[animations[i].set][animations[i].position];

						if((animations[i].position==7)||(animations[i].position==0))
						{
							animations[i].direction=-animations[i].direction;
						}
					}
	
					centipede[i].draw=1;
					spritePlot(SCREEN,&centipede[i]);

					centipede[i].timer.value=frames+1;
				}
			}
			else if(centipede[i].timer.value<frames)
			{	
				centipede[i].active=1;
			}
		}	
	}
}

int main(int argc,char *argv[])
{
	unsigned int i;

	fastSRand(0);
	init(8);

	// Load data from media
	
	loadScreen((unsigned char *)SCREEN,"flp1_","centipede_scr");

	i=getFrames(); //+250;

	loadLibrary(&lib,"centipede_lib",1,0);

	while(i>getFrames()); // Make sure we have shown the splash for at least 5 seconds

	// Initialise stuff

	initMushrooms();
	initCentipede();

	// Set up numbers for score

        spriteSetupFull(&numbers,"Numbers",1,0,1);
	for(i=14;i<24;i++) spriteAddImageFromLibrary(&numbers,&lib,i);
	spriteAddImageFromLibrary(&numbers,&lib,4);

	// Set up player
        spriteSetupFull(&player,"Player",1,0,1);
        spriteAddImageFromLibrary(&player,&lib,4);
        player.x=128;
        player.y=252-8;

	// Set up player's bullet
        spriteSetupFull(&player_bullet,"PB",0,0,0);
        spriteAddImageFromLibrary(&player_bullet,&lib,9);

	// Set up mushroom dropper
	spriteSetupFull(&dropper,"Dropper",0,0,1);
	spriteAddImageFromLibrary(&dropper,&lib,10);

	// Set up spider
	spriteSetupFull(&spider,"Spider",0,0,1);
	spriteAddImageFromLibrary(&spider,&lib,8);

	cls(SCREEN);

	setupMushrooms();
	spritePlot(SCREEN,&player);

	player.timer.value=getFrames();
	score=0;
	lives=3;

	printScore();

	spider.active=0;
	spider.timer.value=getFrames()+50;

	setupCentipede(getFrames());

	while(1)
	{
		unsigned int f=getFrames();

		runCentipede(f);

		if(f>player.timer.value) // Time to move the player?
		{
			unsigned int key=keyrow(1);
			unsigned int newx=player.x,newy=player.y;

			if((key&2)&&(player.x>XMIN)) newx-=2;
			if((key&16)&&(player.x<XMAX-8)) newx+=2;	
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
				player_bullet.y=player.y;
				player_bullet.timer2.value=0;
				player_bullet.timer.value=f;
				//spritePlot(SCREEN,&player_bullet);
			}

			player.timer.value=getFrames()+1;
		}

		// Move player bullet?
		if(player_bullet.active&&(f>player_bullet.timer.value))
		{
			//if(player_bullet.timer2.value++)
			{
				player_bullet.mask=1; player_bullet.draw=0;
				spritePlot(SCREEN,&player_bullet);
			}

			if(player_bullet.y<8)
			{
				player_bullet.active=0;
			}
			else
			{
				player_bullet.y-=8;

				if(mushrooms[(player_bullet.x-XMIN)/8][player_bullet.y/8].active)
				{
					sprite *m=&mushrooms[(player_bullet.x-XMIN)/8][player_bullet.y/8];
					m->mask=1; m->draw=0;
					spritePlot(SCREEN,m);

					if(m->currentImage==3)
					{
						m->currentImage=0;
						m->active=0;
						score++;
						printScore();
					}
					else
					{
						m->currentImage++;
						m->mask=0; m->draw=1;
						spritePlot(SCREEN,m);
					}

					player_bullet.active=0;
				}
				else if(peek(SCREEN,player_bullet.y+4,player_bullet.x)
			    	      ||peek(SCREEN,player_bullet.y+8,player_bullet.x))
				{
					player_bullet.active=0;

					if(spider.active)
					{
						if((spider.x<=player_bullet.x)&&(spider.x+16>=player_bullet.x)&&(spider.y<=player_bullet.y)&&(spider.y+8>=player_bullet.y))
						{
							spider.draw=0;
							spritePlot(SCREEN,&spider);
							spider.draw=1;

							spider.active=0;

							score+=300;
							printScore();
						}
					}
				}
				else
				{
					player_bullet.mask=0; player_bullet.draw=1;
					spritePlot(SCREEN,&player_bullet);
				}
			}

			player_bullet.timer.value=f+1;
		}

		if(f>spider.timer.value)
		{
			spider.timer.value=f+1;

			if(spider.active)
			{
				spider.mask=1; spider.draw=0;
				spritePlot(SCREEN,&spider);

				spider.x+=spider.dx;
				spider.y+=spider.dy;

				if((spider.x<=XMIN)||(spider.x>=XMAX-16))
				{
					spider.active=0;
					spider.timer.value=f+50;
				}
				else
				{
					if(spider.y>=246)
					{
						spider.dy=-spider.dy;	
						spider.dx=fastRand()&3?spider.timer2.value:0;
					}
					else if(spider.y<=256-5*8)
					{
						spider.dy=-spider.dy;	
						spider.dx=fastRand()&3?spider.timer2.value:0;
					}

					spider.mask=0; spider.draw=1;
					spritePlot(SCREEN,&spider);
				}
			}
			else
			{
				spider.active=1;
				spider.y=256-5*8;
				spider.x=fastRand()&1?XMIN:XMAX-16;
				spider.dy=2;
				spider.dx=spider.x<128?2:-2;
				spider.timer2.value=spider.dx;
			}
		}
	}
}
