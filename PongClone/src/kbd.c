
#include "sdlcommon.h"

static int numkeys;
static const Uint8 *current_state;

void kbd_init() {
	current_state = SDL_GetKeyboardState(NULL);
}

void kbd_quit() {
}

// call this once per frame
void kbd_update() {
	// then update the current state
	current_state = SDL_GetKeyboardState(NULL);
}

int kbd_key_is_down(int key) {
	return current_state[key] == 1;
}

int kbd_key_is_up(int key) {
	return current_state[key] == 0;
}
