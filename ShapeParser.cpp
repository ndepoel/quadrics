#include "Common.h"
#include "ShapeParser.h"

static std::string parseToken( const std::string &str, size_t &pos )
{
    // Skip whitespace
    pos = str.find_first_not_of( " \n\r\t", pos );
    if ( pos == std::string::npos )
        return "";

    size_t start = pos;

    char c = str[pos++];
    if ( c == '#' )
    {
        // Rest of the line is commentary
        pos = std::string::npos;
        return "";
    }
    else if ( c == '\"' )
    {
        // Read quoted string
        size_t len = str.length();
        std::string result = "";
        while ( pos < len && (c = str[pos++]) != '\"' )
        {
            result += c;
        }
        return result;
    }

    // Read until the next whitespace or comment
	pos = str.find_first_of( " \n\r\t#", pos );
    if ( pos != std::string::npos )
        return str.substr( start, pos-start );
    else
        return str.substr( start );
}

ShapeParser::ShapeParser( ShapeParser::SphereVector &spheres, ShapeParser::CylinderVector &cylinders ):
	mSpheres( spheres ), mCylinders( cylinders )
{
}

void ShapeParser::parseFloats( const std::string &str, size_t &pos, float *dst, size_t num )
{
	std::string token;
	for ( size_t i = 0; i < num; i++ )
	{
		token = parseToken( str, pos );
		assert( !token.empty() );
		
		dst[i] = float( atof( token.c_str() ) );
	}
}

void ShapeParser::parseSphere( const std::string &str, size_t &pos )
{
	std::string token = parseToken( str, pos );
	assert( token == "{" );
	
	mSpheres.push_back( Sphere() );
	Sphere &sphere = mSpheres.back();
	
	while ( true )
	{
		token = parseToken( str, pos );
		if ( token.empty() || token == "}" )
			break;
			
		if ( token == "center" )
			parseFloats( str, pos, sphere.center, 3 );
		else if ( token == "color" )
			parseFloats( str, pos, sphere.color, 3 );
		else if ( token == "radius" )
			parseFloats( str, pos, &sphere.radius, 1 );
	}
}

void ShapeParser::parseCylinder( const std::string &str, size_t &pos )
{
	std::string token = parseToken( str, pos );
	assert( token == "{" );
	
	mCylinders.push_back( Cylinder() );
	Cylinder &cyl = mCylinders.back();
	
	while ( true )
	{
		token = parseToken( str, pos );
		if ( token.empty() || token == "}" )
			break;
			
		if ( token == "center" )
			parseFloats( str, pos, cyl.center, 3 );
		else if ( token == "color" )
			parseFloats( str, pos, cyl.color, 3 );
		else if ( token == "radius" )
			parseFloats( str, pos, &cyl.radius, 1 );
		else if ( token == "direction" )
		    parseFloats( str, pos, cyl.dir, 3 );
	}
}

bool ShapeParser::parseFile( const std::string &filename )
{
    std::ifstream ifs( filename.c_str() );
    if ( !ifs )
        return false;

	std::stringstream ss;
	std::string line;
	while ( !ifs.eof() )
	{
		std::getline( ifs, line );
		ss << line << std::endl;
	}
	
	std::string content = ss.str();
	size_t pos = 0;
	while ( true )
	{
		std::string token = parseToken( content, pos );
		if ( token.empty() )
			break;
			
		if ( token == "sphere" )
			parseSphere( content, pos );
		else if ( token == "cylinder" )
		    parseCylinder( content, pos );
	}
	
	return true;
}


