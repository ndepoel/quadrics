#ifndef CYLINDER_H_INCLUDED
#define CYLINDER_H_INCLUDED

class Cylinder
{
public:
	Cylinder(): radius(1)
	{
		center[0] = center[1] = center[2] = 0;
	    color[0] = color[1] = color[2] = 1;
	    dir[0] = dir[1] = 0;
	    dir[2] = 1;
    }
	
    float center[3];
    float color[3];
    float radius;
    float dir[3];
};

#endif
