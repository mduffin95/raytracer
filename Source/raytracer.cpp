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

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
float focalLength = SCREEN_HEIGHT / 2.0f;
vec3 cameraPos(0,0,-1.5);
vector<Triangle> triangles;
int t;

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw();
bool ClosestIntersection(
  vec3 start,
  vec3 dir,
  const vector<Triangle>& triangles,
  Intersection& closestIntersection );

bool CheckIntersection( float u, float v );

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
    if (u > 0 && 
        v > 0 && 
        u + v < 1)
    {
      return true;
    }
    return false;
}


bool ClosestIntersection(
  vec3 start,
  vec3 dir,
  const vector<Triangle>& triangles,
  Intersection& closestIntersection )
{
  bool result = false;
  float m = std::numeric_limits<float>::max(); 
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
    if (CheckIntersection( u, v ))
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
      Intersection inter;
      vec3 color;
      if (ClosestIntersection(cameraPos, d, triangles, inter))
      {
        color = triangles[inter.triangleIndex].color; 
      } 
      else
      { 
        color = vec3(0, 0, 0);
      }
			//vec3 color( 1.0, 0.0, 0.0 );
			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}