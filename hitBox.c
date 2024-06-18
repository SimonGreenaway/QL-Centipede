#include "hitBox.h"

inline unsigned int hitBoxContains(hitbox *h,unsigned int x,unsigned int y)
{
	return (h->x1<=x)&&(x<=h->x2)&&(h->y1<=y)&&(y<=h->y2);
}

inline unsigned int hit(hitbox *h1,hitbox *h2)
{
	return    hitBoxContains(h1,h2->x1,h2->y1)
		||hitBoxContains(h1,h2->x1,h2->y2)
		||hitBoxContains(h1,h2->x2,h2->y2)
		||hitBoxContains(h1,h2->x2,h2->y1);
}
