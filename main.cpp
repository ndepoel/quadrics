#include "Common.h"
#include "ShapeParser.h"
#include "Sphere.h"
#include "Cylinder.h"

CGcontext	context;
CGprofile	vertexProfile, fragmentProfile;

GLuint color_tex[2], depthbuffer, fbo;

struct VertexProgram
{
    CGprogram program;
	CGparameter	modelViewIT, 
				modelViewProj,
				wsize;
};

struct FragmentProgram
{
    CGprogram program;
	CGparameter unproj_scale,
				unproj_offset,
				near,
				zb_scale,
				zb_offset,
				lightpos;
};

struct DeferredProgram
{
    CGprogram program;
    CGparameter surface,
                color,
                unproj_scale,
                unproj_offset,
                near,
                tex_res,
                lightpos;
};

static struct
{
    VertexProgram sphere;
    VertexProgram cylinder;
} 
vertexPrograms = {{0}};

static struct
{
    FragmentProgram sphere_direct;
    FragmentProgram sphere_deferred;
    FragmentProgram cylinder_direct;
} 
fragmentPrograms = {{0}};

static struct
{
    DeferredProgram normal;
    DeferredProgram outline;
}
deferredPrograms = {{0}};

std::vector<Sphere> spheres;
std::vector<Cylinder> cylinders;

int w_width=1024, w_height=768;

static const GLdouble fovy = 30, zNear = 0.5f, zFar = 10;
static GLdouble aspect, xmin, xmax, ymin, ymax;

int benchmark = 0, frameCount = 0;
int deferred = 0, outlines = 0;
int drawSpheres = 1, drawCylinders = 0;
int paused = 0;
float campos[3] = { 0, 0, 5 };
float currentFps = 0;

float backgrounds[][3] =
{
    { 0, 0, 0 },
    { 0.5, 0.5, 0.5 },
    { 1, 1, 1 }
};
int numBackgrounds = sizeof(backgrounds) / sizeof(backgrounds[0]);
int currentBackground = 0;

float lightpos[3] = { 5, 0, 0 };

void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
        case 'q':
        case 'Q':
        case 27:
            exit(0);
            break;
        case 'd':
            deferred = !deferred;
            break;
        case 'o':
            outlines = !outlines;
            break;
        case 'b':
            currentBackground = (currentBackground + 1) % numBackgrounds;
            break;
        case 'c':
            drawCylinders = !drawCylinders;
            break;
        case 's':
            drawSpheres = !drawSpheres;
            break;
        case 32:
        	paused = !paused;
        	break;
    }
}

void keyboardSpecial( int key, int x, int y )
{
	switch(key)
	{
        case GLUT_KEY_UP:
        	campos[2] -= 1;
        	break;
        case GLUT_KEY_DOWN:
        	campos[2] += 1;
        	break;
	}
}

void handleCgError() 
{
    fprintf(stderr, "Cg error: %s\n", cgGetErrorString(cgGetError()));
}

void updateWindowTitle()
{
    static char buf[128];
    
    sprintf( buf, "%.1ffps %s %s %s", 
        currentFps, 
        benchmark ? "| benchmark" : "",
        deferred ? "| deferred shading" : "",
        (deferred && outlines) ? "| silhouette outlines" : "" );
        
	glutSetWindowTitle( buf );
}

void updateFramerate( int curTime )
{
	static int numFrames = 0;
	static int prevTime = 0;

	if ( curTime > prevTime + 1000 )
	{
		float deltaTime = float(curTime - prevTime) / 1000.0f;
		currentFps = (float)numFrames / deltaTime;
	
		numFrames = 0;
		prevTime = curTime;
	}
	
	numFrames++;
}

void activatePrograms( VertexProgram &vp, FragmentProgram &fp )
{
	cgGLEnableProfile( vertexProfile );
	cgGLEnableProfile( fragmentProfile );

	cgGLBindProgram( vp.program );
	cgGLBindProgram( fp.program );
    
	cgGLSetStateMatrixParameter( vp.modelViewIT, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE );
	cgGLSetStateMatrixParameter( vp.modelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY );
	cgGLSetParameter4f( vp.wsize, w_width, w_height, 1.0f / w_width, 1.0f / w_height );
	
	cgGLSetParameter2f( fp.unproj_scale, float(xmax-xmin)/w_width, float(ymax-ymin)/w_height );
	cgGLSetParameter2f( fp.unproj_offset, -xmin, -ymin );
	cgGLSetParameter1f( fp.near, zNear );
	cgGLSetParameter1f( fp.zb_scale, (zFar*zNear)/(zFar-zNear) );
	cgGLSetParameter1f( fp.zb_offset, zFar/(zFar-zNear) );
	cgGLSetParameter3fv( fp.lightpos, lightpos );
	
	glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
}

void deactivatePrograms()
{
    glDisable( GL_VERTEX_PROGRAM_POINT_SIZE );
    
    cgGLDisableProfile( vertexProfile );
    cgGLDisableProfile( fragmentProfile );
}

void renderSpheres()
{
    VertexProgram &vp = vertexPrograms.sphere;
    FragmentProgram &fp = deferred ? fragmentPrograms.sphere_deferred : fragmentPrograms.sphere_direct;

	cgGLEnableProfile( vertexProfile );
	cgGLEnableProfile( fragmentProfile );
	
	activatePrograms( vp, fp );
    
    glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
    
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );

	for ( GLuint texunit = GL_TEXTURE0; texunit <= GL_TEXTURE0; texunit++ )
	{
		glClientActiveTexture( texunit );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	}
	
	glVertexPointer( 3, GL_FLOAT, sizeof(Sphere), spheres[0].center );
	glColorPointer( 3, GL_FLOAT, sizeof(Sphere), spheres[0].color );
	
	glClientActiveTexture( GL_TEXTURE0 );
	glTexCoordPointer( 1, GL_FLOAT, sizeof(Sphere), &spheres[0].radius );
	
	glDrawArrays( GL_POINTS, 0, spheres.size() );
	
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	
	for ( GLuint texunit = GL_TEXTURE0; texunit <= GL_TEXTURE0; texunit++ )
	{
		glClientActiveTexture( texunit );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	}
	
	deactivatePrograms();
}

void renderCylinders()
{  
    VertexProgram &vp = vertexPrograms.cylinder;
    FragmentProgram &fp = fragmentPrograms.cylinder_direct;
	
	activatePrograms( vp, fp );
    
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );

	for ( GLuint texunit = GL_TEXTURE0; texunit <= GL_TEXTURE1; texunit++ )
	{
		glClientActiveTexture( texunit );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	}
	
	glVertexPointer( 3, GL_FLOAT, sizeof(Cylinder), cylinders[0].center );
	glColorPointer( 3, GL_FLOAT, sizeof(Cylinder), cylinders[0].color );
	
	glClientActiveTexture( GL_TEXTURE0 );
	glTexCoordPointer( 1, GL_FLOAT, sizeof(Cylinder), &cylinders[0].radius );

	glClientActiveTexture( GL_TEXTURE1 );
	glTexCoordPointer( 3, GL_FLOAT, sizeof(Cylinder), &cylinders[0].dir );
	
	glDrawArrays( GL_POINTS, 0, cylinders.size() );
	
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	
	for ( GLuint texunit = GL_TEXTURE0; texunit <= GL_TEXTURE1; texunit++ )
	{
		glClientActiveTexture( texunit );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	}
    
    deactivatePrograms();
}

void display() 
{
	static int startTime = 0, prevTime = 0;

    int curTime = glutGet(GLUT_ELAPSED_TIME);
    if ( !startTime )
        startTime = curTime;

    // Stop the benchmark once the program has run for 10 seconds
    if ( benchmark && curTime >= startTime + 10000 )
    {
        float deltaTime = float(curTime - startTime) / 1000;
        printf( "Rendered %i frames in %.1f seconds (avg. fps = %.2f)\n", 
            frameCount, deltaTime, (float)frameCount / deltaTime );
        exit(0);
    }

	updateFramerate( curTime );

    if ( deferred )
    {
    	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo );
	
	    static const GLenum bufs[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
    	glDrawBuffers( 2, bufs );
    }

    float *bg = backgrounds[currentBackground];
    glClearColor( bg[0], bg[1], bg[2], 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Manual implementation of gluPerspective
    aspect = (float)w_width / w_height;
	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
        
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(campos[0], campos[1], campos[2], 0, 0, 0, 0, 1, 0);
    
//    glTranslatef(0, 0, sin((float)curTime / 200.0) * 4);

	static float rotation = 0;
    glRotatef(rotation, 0, 1, 0);
    
    if ( !paused )
	    rotation += float(curTime - prevTime) / 20;
    
    glColor3f(1.0,1.0,1.0);
    
    // Visible light source
//    glutSolidSphere( 0.25f, 10, 10 );
    
    if ( drawSpheres )
        renderSpheres();
    
    if ( drawCylinders )
        renderCylinders();
    
    if ( deferred )
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
    
    if ( deferred )
    {
        // Render the color texture to a full-screen quad
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        glDisable( GL_DEPTH_TEST );
        
        DeferredProgram &dp = outlines ? deferredPrograms.outline : deferredPrograms.normal;
        
        cgGLEnableProfile( fragmentProfile );
        cgGLBindProgram( dp.program );
        
        cgGLSetTextureParameter( dp.surface, color_tex[0] );
       	cgGLEnableTextureParameter( dp.surface );
        cgGLSetTextureParameter( dp.color, color_tex[1] );
       	cgGLEnableTextureParameter( dp.color );
       	
	    cgGLSetParameter2f( dp.unproj_scale, float(xmax-xmin), float(ymax-ymin) );
	    cgGLSetParameter2f( dp.unproj_offset, -xmin, -ymin );
	    cgGLSetParameter1f( dp.near, zNear );
	    cgGLSetParameter2f( dp.tex_res, 1.0f / w_width, 1.0f / w_height );
	    cgGLSetParameter3fv( dp.lightpos, lightpos );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
            
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
       
        glBegin( GL_QUADS );
        	glTexCoord2f( 0, 0 ); glVertex2i( -1, -1 );
        	glTexCoord2f( 0, 1 ); glVertex2i( -1, 1 );
        	glTexCoord2f( 1, 1 ); glVertex2i( 1, 1 );
        	glTexCoord2f( 1, 0 ); glVertex2i( 1, -1 );
        glEnd();
        
        cgGLDisableTextureParameter( dp.surface );
        cgGLDisableTextureParameter( dp.color );

        cgGLDisableProfile( fragmentProfile );
    }    
    
    glutSwapBuffers();
    frameCount++;
    
    prevTime = curTime;
}

/* Choose profiles, set optimal options */
void chooseCgProfiles()
{
    vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
    cgGLSetOptimalOptions(vertexProfile);
    fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    cgGLSetOptimalOptions(fragmentProfile);
    printf("vertex profile:   %s\n",
         cgGetProfileString(vertexProfile));
    printf("fragment profile: %s\n",
         cgGetProfileString(fragmentProfile));
}

/* Load Cg program from disk */
CGprogram loadCgProgram(CGprofile profile, const char *filename)
{
    CGprogram program;
    assert(cgIsContext(context));

    fprintf(stderr, "Cg program %s creating.\n", filename);
    program = cgCreateProgramFromFile(context, CG_SOURCE,
            filename, profile, NULL, NULL);
    
    if(!cgIsProgramCompiled(program)) {
        printf("%s\n",cgGetLastListing(context));
        exit(1);
    }
    
    fprintf(stderr, "Cg program %s loading.\n", filename);
    cgGLLoadProgram(program);
    
    return program;
}

void loadVertexProgram( VertexProgram &vp, const char *filename )
{
    vp.program = loadCgProgram( vertexProfile, filename );
    
    vp.modelViewIT = cgGetNamedParameter( vp.program, "ModelViewIT" );
    vp.modelViewProj = cgGetNamedParameter( vp.program, "ModelViewProj" );
    vp.wsize = cgGetNamedParameter( vp.program, "wsize" );
}

void loadFragmentProgram( FragmentProgram &fp, const char *filename )
{
    fp.program = loadCgProgram( fragmentProfile, filename );
    
    fp.unproj_scale = cgGetNamedParameter( fp.program, "unproj_scale" );
    fp.unproj_offset = cgGetNamedParameter( fp.program, "unproj_offset" );
    fp.near = cgGetNamedParameter( fp.program, "near" );
    fp.zb_scale = cgGetNamedParameter( fp.program, "zb_scale" );
    fp.zb_offset = cgGetNamedParameter( fp.program, "zb_offset" );
    fp.lightpos = cgGetNamedParameter( fp.program, "lightpos" );
}

void loadDeferredProgram( DeferredProgram &dp, const char *filename )
{
    dp.program = loadCgProgram( fragmentProfile, filename );

    dp.surface = cgGetNamedParameter( dp.program, "surface" );
    dp.color = cgGetNamedParameter( dp.program, "color" );
    dp.unproj_scale = cgGetNamedParameter( dp.program, "unproj_scale" );
    dp.unproj_offset = cgGetNamedParameter( dp.program, "unproj_offset" );
    dp.near = cgGetNamedParameter( dp.program, "near" );
    dp.tex_res = cgGetNamedParameter( dp.program, "tex_res" );
    dp.lightpos = cgGetNamedParameter( dp.program, "lightpos" );
}

void loadCgPrograms()
{
    /* Load all Cg programs that are used with loadCgProgram */
    loadVertexProgram( vertexPrograms.sphere, "sphere_vp.cg" );
    loadVertexProgram( vertexPrograms.cylinder, "cylinder_vp.cg" );
    
    loadFragmentProgram( fragmentPrograms.sphere_direct, "sphere_fp.cg" );
    loadFragmentProgram( fragmentPrograms.sphere_deferred, "sphere_fp_deferred.cg" );
    loadFragmentProgram( fragmentPrograms.cylinder_direct, "cylinder_fp.cg" );
    
    loadDeferredProgram( deferredPrograms.normal, "deferred.cg" );
    loadDeferredProgram( deferredPrograms.outline, "deferred_outline.cg" );
}

void initRenderBuffers()
{
    for ( int i = 0; i < 2; i++ )
    {
	    glBindTexture( GL_TEXTURE_2D, color_tex[i] );
    	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, w_width, w_height, 0, GL_RGBA, GL_FLOAT, NULL );
    	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }
    
    glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, depthbuffer );
    glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, w_width, w_height );    
}

void createFBO()
{
    glGenTextures( 3, color_tex );    
    glGenRenderbuffersEXT( 1, &depthbuffer );
    
    initRenderBuffers();
    
    glGenFramebuffersEXT( 1, &fbo );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo );
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex[0], 0 );
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, color_tex[1], 0 );
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, color_tex[2], 0 );
    glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer );

    assert( glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) == GL_FRAMEBUFFER_COMPLETE_EXT );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

void idle()
{
    glutPostRedisplay();
    updateWindowTitle();
}

void reshape(int width, int height)
{
    w_width = width;
    w_height = height;
    glViewport(0, 0, w_width, w_height);
    
    initRenderBuffers();
}

bool loadShapes( const char *filename )
{
	ShapeParser parser( spheres, cylinders );
	return parser.parseFile( filename );
}

int main(int argc, char *argv[])
{
	assert(argc>=2 && argv[1]);
	if ( loadShapes( argv[1] ) )
		printf( "Loaded %i spheres, %i cylinders\n", (int)spheres.size(), (int)cylinders.size() );

    benchmark = (strstr(argv[0], "benchmark") != NULL);
    if ( benchmark )
        printf( "Benchmark mode enabled\n" );
        
    deferred = (argc >= 3);
    outlines = (argc >= 4);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(w_width, w_height);
    glutCreateWindow("GPU-based ray casting of quadratic surfaces");
    
    glewInit();
    
    createFBO();
        
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboardSpecial);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    
    cgSetErrorCallback(handleCgError);
    context = cgCreateContext();
    chooseCgProfiles();
    loadCgPrograms();
    
    glutMainLoop();
    
    return 0;
}


