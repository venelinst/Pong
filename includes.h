#include<iostream>
#include<SDL2/SDL.h>
#include<SDL_mixer.h>

#include<stdlib.h>
#include<time.h>//for angle generation
#include<math.h>//for sin and cos

double getRandomAngle();
void draw();
void eventHandler(SDL_Event &e);
void playerMovement();
void ballMovement( double &ballSpeed, double &ballAngle);
void makeComputerMove( double ballAngle);
char getKeyPress(SDL_Event &e);
bool belowTopBorder(SDL_Rect playerx);
bool aboveBottomBorder(SDL_Rect playerx);

Uint32 timer(Uint32 interval, void *param);
