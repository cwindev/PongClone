
#include "sdlcommon.h"

int mouseX;
int mouseY;

void ms_update() {
	SDL_GetMouseState(&mouseX, &mouseY);
}