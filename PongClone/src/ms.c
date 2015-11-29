
#include "sdlcommon.h"

int mouseX;
int mouseY;

int ms_update() {
	SDL_GetMouseState(&mouseX, &mouseY);
}