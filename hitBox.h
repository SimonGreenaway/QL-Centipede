typedef struct
{
	unsigned int x1,y1,x2,y2;
} hitbox;

inline unsigned int hitBoxContains(hitbox *h,unsigned int x,unsigned int y);
unsigned int hit(hitbox *h1,hitbox *h2);
