#include <stdio.h>
#include <unistd.h>
#include <qdos.h>

#include "image.h"

#define MUSHX 25
#define MUSHY 28

#define XMIN ((256-MUSHX*8)/2)
#define XMAX (256-(256-MUSHX*8)/2)

library lib,font;

sprite player,player_bullet,dropper,spider;
sprite centipede[12];
sprite mushrooms[MUSHX][MUSHY];
sprite numbers;

unsigned int mushroomCount,mushroomTarget;
unsigned int score,lives;
unsigned int highScores[8]={16543,15342,14320,13210,13010,12805,12201,12102};
unsigned char *highScorers[8]={"EJD","DFT","CAD","DCB","ED ","DEW","DFW","GJR"};

// Random stuff!
//
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

	scorePrint(128+1*8,0,5,highScores[0]);
}

void printHighScores()
{
	char *copyright="?1980 ATARI";
	unsigned int i,ok=1;
	char b[32];

	copyright[0]=127;

	setFontMasking(2);

	printAt(SCREEN,&font,8,84,24,"HIGH SCORES");
	for(i=0;i<8;i++)
	{
		sprintf(b,"%5d %3s",highScores[i],highScorers[i]);
		printAt(SCREEN,&font,8,92,32+i*8,b);
	}

	printAt(SCREEN,&font,8,76,102,"1 COIN 1 PLAY");
	printAt(SCREEN,&font,8,68,102+32,"BONUS EVERY 12000");

	printAt(SCREEN,&font,8,90,256-9,copyright);

	while(ok)
	{
		for(i=0;i<10;i++)
		{
			if(keyrow(i)) ok=0;
		}	
	}

	putchar('!');
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
	unsigned int x,y;

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

	for(x=0;x<MUSHX;x++)
		for(y=0;y<MUSHY;y++)
			mushrooms[x][y].active=0;

	for(i=0;i<50;i++)
	{
		do
		{
			x=(fastRand()*MUSHX)/65536;
			y=(fastRand()*MUSHY)/65536;
		}
		while(mushrooms[x][y].active);

		mushrooms[x][y].active=1;
		spritePlot(SCREEN,&mushrooms[x][y]);
	}

	mushroomCount=50;
	mushroomTarget=50;
}

sprite *getMushroom(unsigned int x,unsigned int y)
{
	if(y/8>=MUSHY) return NULL;
	else return &mushrooms[(x-XMIN)/8][y/8];
}

unsigned int isMushroom(unsigned int x,unsigned int y,unsigned int debug)
{
	sprite *m=getMushroom(x,y);

	if(debug)
	{
		if(m!=NULL)
		{
			box(SCREEN,m->x,m->y,m->x+m->image[m->currentImage]->x*4,m->y+m->image[m->currentImage]->y,m->active+4);
			sleep(1);
			box(SCREEN,m->x,m->y,m->x+m->image[m->currentImage]->x*4,m->y+m->image[m->currentImage]->y,0);
		}
	}

	return (m!=NULL)&&getMushroom(x,y)->active;
}

void removeMushroom(unsigned int x,unsigned int y)
{
	sprite *m=getMushroom(x,y);

	if((m!=NULL)&&(m->active))
	{
		//box(SCREEN,m->x,m->y,m->x+8,m->y+8,7); 
		//sleep(1);
		//box(SCREEN,m->x,m->y,m->x+8,m->y+8,0); 

		m->draw=0;
		spritePlot(SCREEN,m);
		m->draw=1;
		m->active=0;
		mushroomCount--;
	}
}

void runCentipede(unsigned int frames)
{
	unsigned int i;

	for(i=0;i<12;i++) // Run through 12 centipede segments
	{
		if(centipede[i].timer2.value)
		{
			if(centipede[i].active)
			{
				if(centipede[i].timer.value<frames)
				{
					unsigned int newx=centipede[i].x,newy=centipede[i].y;
					unsigned int newImage=centipede[i].currentImage;

					newx+=centipede[i].dx; // Move segment

					// Check for edge or mushroom bounce
				
					if((newx==XMIN)
					  ||(newx==XMAX-8)
                                          ||isMushroom(newx+(centipede[i].dx>0?6:0),newy-4,0))
					{
						centipede[i].dx=-centipede[i].dx; // Reverse direction
						newy+=centipede[i].dy*8;	// Move down
						animations[i].set=1-animations[i].set; // Swap animation set to the other direction

						if(animations[i].head)
							newImage=(animations[i].set==0)?0:6;

						if(isMushroom(newx+2,newy-4,0))
						{
							sprite *mushroom=getMushroom(newx+2,newy-4);
							mushroom->draw=0;
							spritePlot(SCREEN,mushroom);
							mushroom->active=0;
							mushroomCount--;
						}
					}

					if(newy>=248) // Bottom of screen?
					{
						newy=256-6*8;
						newx=centipede[i].dx<0?XMAX-8:XMIN;
					}

					// Set animation frame
				
					if(!animations[i].head) // Don't animate the head
					{
						animations[i].position+=animations[i].direction;

						newImage=centipedeAnimation[animations[i].set][animations[i].position];

						if((animations[i].position==7)||(animations[i].position==0))
						{
							animations[i].direction=-animations[i].direction;
						}
					}

					centipede[i].draw=0;
					spritePlot(SCREEN,&centipede[i]);

					centipede[i].x=newx;
					centipede[i].y=newy;
					centipede[i].currentImage=newImage;

					centipede[i].draw=1;
					spritePlot(SCREEN,&centipede[i]);

					centipede[i].timer.value=getFrames()+1;
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

	i=getFrames()+100;

	loadLibrary(&font,"atari_lib",1,0);

	printAt(SCREEN,&font,8,15,215,"Centipede by Simon Greenaway");

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
	dropper.draw=1; dropper.mask=1;
	spriteAddImageFromLibrary(&dropper,&lib,10);

	// Set up spider
	spriteSetupFull(&spider,"Spider",0,0,1);
	spriteAddImageFromLibrary(&spider,&lib,34);
	spriteAddImageFromLibrary(&spider,&lib,33);
	spriteAddImageFromLibrary(&spider,&lib,8);
	spriteAddImageFromLibrary(&spider,&lib,35);

	cls(SCREEN);
	setupMushrooms();
	printHighScores();
	putchar('a');

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

		////////////
		// Player //
		////////////

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
				player_bullet.y=player.y-8;
				player_bullet.timer2.value=0;
				player_bullet.timer.value=f;
				//spritePlot(SCREEN,&player_bullet);
			}

			player.timer.value=getFrames()+1;
		}

		/////////////////////////
		// Move player bullet? //
		/////////////////////////
	
		if(player_bullet.active&&(f>player_bullet.timer.value))
		{
			unsigned int i;

			player_bullet.mask=1; player_bullet.draw=0;
			spritePlot(SCREEN,&player_bullet);

			for(i=0;i<2;i++)
			{
				if(player_bullet.y<8)
				{
					player_bullet.active=0;
					break;
				}
				else
				{
					player_bullet.y-=8;
	
					if(isMushroom(player_bullet.x,player_bullet.y,0))
					{
						sprite *m=getMushroom(player_bullet.x,player_bullet.y);
						m->mask=1; m->draw=0;
						spritePlot(SCREEN,m);
	
						if(m->currentImage==3)
						{
							m->currentImage=0;
							m->active=0;
							mushroomCount--;
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
						break;
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

						break;
					}
				}
			}

			if(player_bullet.active)
			{
				player_bullet.mask=0; player_bullet.draw=1;
				spritePlot(SCREEN,&player_bullet);
			}

			player_bullet.timer.value=getFrames()+1;
		}

		////////////
		// Spider //
		////////////

		if(f>spider.timer.value)
		{
			if(spider.active)
			{
				spider.mask=1; spider.draw=0;
				spritePlot(SCREEN,&spider);

				spider.x+=spider.dx;
				spider.y+=spider.dy;

				if((spider.x<=XMIN)||(spider.x>=XMAX-16))
				{
					spider.active=0;
					spider.timer.value=getFrames()+50;
				}
				else
				{
					if(spider.y>=246)
					{
						spider.dy=-spider.dy;	
						spider.dx=fastRand()&3?spider.timer2.value:0;
					}
					else if(spider.y<=256-6*8)
					{
						spider.dy=-spider.dy;	
						spider.dx=fastRand()&3?spider.timer2.value:0;
					}

					removeMushroom(spider.x,spider.y-2);
					removeMushroom(spider.x+8,spider.y-2);

					spider.currentImage=(spider.currentImage+1)&3;

					spider.mask=0; spider.draw=1;
					spritePlot(SCREEN,&spider);
				}
			}
			else
			{
				spider.active=1;
				spider.y=256-6*8;
				spider.x=fastRand()&1?XMIN:XMAX-16;
				spider.dy=2;
				spider.dx=spider.x<128?2:-2;
				spider.timer2.value=spider.dx;
			}

			spider.timer.value=getFrames()+1;
		}

		//////////////////////
		// Mushroom dropper //
		//////////////////////
	
		if(f>dropper.timer.value)
		{
			if(dropper.active)
			{
				if(!isMushroom(dropper.x,dropper.y-2,0))
				{
					dropper.draw=0;
					spritePlot(SCREEN,&dropper);
					dropper.draw=1;

					if((fastRand()&7)==0)
					{
						sprite *m=getMushroom(dropper.x,dropper.y-2);

						m->active=1;
						mushroomCount++;
						spritePlot(SCREEN,m);
					}
				}

				// Check mushroom?
				dropper.y+=8;

				if(dropper.y>=256-8-8)
				{
					dropper.active=0;
				}
				else if(!isMushroom(dropper.x,dropper.y-2,0))
				{
					spritePlot(SCREEN,&dropper);
				}
			}
			else if((mushroomCount<mushroomTarget)&&((fastRand()&127)==0))
			{
				// Start dropper
			
				dropper.x=mushrooms[(fastRand()*MUSHX)/65536][0].x;
				dropper.y=8;
				dropper.active=1;
	
				if(isMushroom(dropper.x,dropper.y,0))
							spritePlot(SCREEN,&dropper);
	
			}
 	
			dropper.timer.value=getFrames()+1;
		}

		if((f&63)==0) mushroomTarget++;
	}
}
