#ifndef SHAPEPARSER_H_INCLUDED
#define SHAPEPARSER_H_INCLUDED

#include "Sphere.h"
#include "Cylinder.h"

class ShapeParser
{
public:
	ShapeParser( std::vector<Sphere> &spheres, std::vector<Cylinder> &cylinders );

	bool parseFile( const std::string &filename );
	
private:
	void parseFloats( const std::string &str, size_t &pos, float *dst, size_t num );
	void parseSphere( const std::string &str, size_t &pos );
	void parseCylinder( const std::string &str, size_t &pos );

	typedef std::vector<Sphere> SphereVector;
	SphereVector &mSpheres;
	
	typedef std::vector<Cylinder> CylinderVector;
	CylinderVector &mCylinders;
};

#endif
