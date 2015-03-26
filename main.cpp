#include <iostream>
#include "world.h"

#include <SDL.h>
#include <SDL_opengl.h>

#ifdef __APPLE__
//why, apple?   why????
#include <OpenGL/glu.h>
#else
#include <gl/glu.h>
#endif

void loop(SDL_Window* window,
		  World& world);

int main(int argc, char** argv){
  
  if(argc != 2){
	std::cout << "usage: ./runSimulator <inputfile.json>" << std::endl;
	exit(1);
  }
  
  World world;
  world.loadFromJson(argv[1]);

  if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
	std::cout << "couldn't init SDL" << std::endl;
	exit(1);
  }
  
  std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> 
	window{SDL_CreateWindow("3.2", SDL_WINDOWPOS_CENTERED,
							SDL_WINDOWPOS_CENTERED,
							800,800,
							SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN),
	  &SDL_DestroyWindow};
  //automatically destory the window when we're done

  SDL_SetWindowSize(window.get(), 800, 800);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  
  loop(window.get(), world);
  return 0;
}


void loop(SDL_Window* window, World& world){

  auto context = SDL_GL_CreateContext(window);  
  int frame = 0;

  //loop
  bool readyToExit = false;

  bool mouseDown = false;
  Eigen::Vector2i mousePosition;

  while(!readyToExit){

	SDL_Event event;
	while(SDL_PollEvent(&event)){
	  switch(event.type){
	  case SDL_KEYDOWN:
		if(event.key.keysym.sym == SDLK_ESCAPE){
		  readyToExit = true;
		} else if(event.key.keysym.sym == SDLK_r){
		  world.restart();
		} else if(event.key.keysym.sym == SDLK_UP){
		  world.move(true);
		} else if(event.key.keysym.sym == SDLK_DOWN){
		  world.move(false);
		} else if(event.key.keysym.sym == SDLK_c){
		  world.drawClusters = !world.drawClusters;
		} else if(event.key.keysym.sym == SDLK_p){
		  world.paused = !world.paused;
		}
		break;
	  case SDL_QUIT:
		readyToExit = true;
		break;

	  case SDL_MOUSEBUTTONDOWN:
		mouseDown = true;
		mousePosition = Eigen::Vector2i{event.button.x, event.button.y};
		break;
	  case SDL_MOUSEBUTTONUP:
		mouseDown = false;
		mousePosition = Eigen::Vector2i{event.button.x, event.button.y};
		break;
		
	  case SDL_MOUSEWHEEL:
		world.zoom(event.wheel.y);
		break;
	  case SDL_MOUSEMOTION:
		if(mouseDown){
		  Eigen::Vector2i newPosition{event.motion.x, event.motion.y};
		  world.pan(mousePosition, newPosition);
		  mousePosition = newPosition;
		}
		break;
	  default:
		; // do nothing
	  }
	  if(readyToExit){break;}
	}

	//uncomment to save output files
	/*
	char fname[80];
	sprintf (fname, "particles.%05d.txt", 10000+(frame++));
	world.saveParticleFile(std::string(fname));
	if(frame > 600){break;}
	*/
	if(world.paused){
	  world.drawSingleCluster(window, frame);
	  SDL_Delay(300);
	} else {
	  world.timestep();
	  world.draw(window);
	}
	++frame;
  }
  world. prof.dump<std::chrono::duration<double>>(std::cout);
  SDL_GL_DeleteContext(context);
}
