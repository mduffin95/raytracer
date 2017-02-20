#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <limits>

using namespace std;
using glm::vec3;
using glm::mat3;

struct Intersection
{
  vec3 position;
  float distance;
  int triangleIndex;
};

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

#define MOVE 0.1f
#define PI 3.14159265359f
const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
float focalLength = SCREEN_HEIGHT / 1.2f;
vec3 cameraPos(0,0,-2.5);

mat3 R;
float angle = 0.0f;

vector<Triangle> triangles;
int t;

//Light values
vec3 lightPos( 0, -0.5, -0.7 );
vec3 lightColor = 14.f * vec3( 1, 1, 1 ) * 5.0f;
vec3 indirectLight = 0.5f*vec3( 1, 1, 1 );
float p = 0.75;

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw();
bool ClosestIntersection(
  vec3 start,
  vec3 dir,
  const vector<Triangle>& triangles,
  Intersection& closestIntersection, int index );

bool CheckIntersection( float u, float v );


/* ----------------------------------------------------------------------------*/
/* EXTAS                                                                       */

//TODO: Soft shadows
//TODO: Experiment with better illumination, i.e see slides
//TODO: Optimisation techniques (Serial + Parallel)
//TODO: Shadings / textures
//TODO: Anti Aliasing




int main( int argc, char* argv[] )
{
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.

  LoadTestModel( triangles );
	while( NoQuitMessageSDL() )
	{
		Update();
		Draw();
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

bool CheckIntersection( float u, float v )
{
    if (u >= 0 &&
        v >= 0 &&
        u + v <= 1)
    {
      return true;
    }
    return false;
}


bool ClosestIntersection(
  vec3 start,
  vec3 dir,
  const vector<Triangle>& triangles,
  Intersection& closestIntersection , int index )
{
  bool result = false;
  for (unsigned i=0; i<triangles.size(); i++)
  {
    vec3 v0 = triangles[i].v0;
    vec3 v1 = triangles[i].v1;
    vec3 v2 = triangles[i].v2;

    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    vec3 b = start - v0;

    mat3 A( -dir, e1, e2 );
    vec3 x = glm::inverse( A ) * b;      
    float t = x.x;
    float u = x.y;
    float v = x.z;
		//Changed t < d to t <= d, to fix shadow bug
    if (t <= closestIntersection.distance &&
        t >= 0 &&
        CheckIntersection( u, v) &&
				i != index )
    {
      closestIntersection.distance = t;
      closestIntersection.position = start + t * dir;
      closestIntersection.triangleIndex = i;
      result = true;
    }
  }
  return result;
}


void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
  
  Uint8* keystate = SDL_GetKeyState( 0 );


	float yaw;
	yaw =  (-angle /180) * PI ;
	R = mat3(cos(yaw), 0.0, -sin(yaw),  // 1. column
					 0, 1.0, 0.0,  // 2. column
					 sin(yaw), 0, cos(yaw)); // 3. column


	vec3 forward(R[2][0], R[2][1], R[2][2]);
	vec3 down(R[1][0], R[1][1], R[1][2]);
	vec3 right(R[0][0], R[0][1], R[0][2]);


	forward *= 0.1;
	right *= 0.1;


	//Camera Position
  if( keystate[SDLK_UP] )
  {
  // Move camera forward
    cameraPos +=  forward ;

  }
  if( keystate[SDLK_DOWN] )
  {
  // Move camera backward
    cameraPos -= forward;
  }
  if( keystate[SDLK_z] )
  {
  // Move camera to the left
    cameraPos -= right;
  }
  if( keystate[SDLK_x] )
  {
  // Move camera to the right
    cameraPos += right;
  }

	//Rotation
	if( keystate[SDLK_LEFT]){
		angle += 5.0;
	}

	if( keystate[SDLK_RIGHT]){
		angle -= 5.0;
	}

	//Light position
	if( keystate[SDLK_w] )
		lightPos += forward;

	if( keystate[SDLK_s] )
		lightPos -= forward;

	if( keystate[SDLK_a] )
		lightPos -= right;

	if( keystate[SDLK_d] )
		lightPos += right;

	if( keystate[SDLK_q] )
		lightPos -= down;

	if( keystate[SDLK_e] )
		lightPos += down;

	yaw =  (angle /180) * PI ;
	//vec3 forward(R[2][0], R[2][1], R[2][2]);
	R = mat3(cos(yaw), 0.0, -sin(yaw),  // 1. column
					 0, 1.0, 0.0,  // 2. column
					 sin(yaw), 0, cos(yaw)); // 3. column

}


vec3 DirectLight( const Intersection& i, const vector<Triangle>& triangles ){


	vec3 r = lightPos - i.position ;
	vec3 r_normal = normalize(r);
	float light_distance  = r.length();

	//Check closest intersection between camera position
	Intersection lightIntersection;
	lightIntersection.distance = 1;
	if(ClosestIntersection(i.position,r,triangles,lightIntersection, i.triangleIndex)){
		return vec3(0,0,0);
	}

	float max1 =  max((float)dot(triangles[i.triangleIndex].normal , r_normal),0.0f);

	vec3 illuminationColour = max1 * lightColor / ( 4.0f * powf(light_distance,2) * PI )  ;

	return illuminationColour;
}

void Draw()
{



	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{

			vec3 d(x - SCREEN_WIDTH / 2.0f, y - SCREEN_HEIGHT / 2.0f, focalLength);

			d = d*R;

      Intersection inter;
      inter.distance = numeric_limits<float>::max();
      vec3 colour;
      if (ClosestIntersection(cameraPos, d, triangles, inter, -1))
      {
        colour = triangles[inter.triangleIndex].color;
				colour *= p*(DirectLight(inter, triangles)+indirectLight);
      }
      else
      { 
        colour = vec3(0, 0, 0);
      }
			//vec3 color( 1.0, 0.0, 0.0 );
			PutPixelSDL( screen, x, y, colour );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}
