#ifndef SPHERE_H_INCLUDED
#define SPHERE_H_INCLUDED

class Sphere
{
public:
	Sphere(): radius(1)
    {
	    center[0] = center[1] = center[2] = 0;
	    color[0] = color[1] = color[2] = 1;
    }	
	
    float center[3];
    float color[3];
    float radius;
};

#endif
