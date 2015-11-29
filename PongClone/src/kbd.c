
#include "sdlcommon.h"
#include "kbd.h"
#include <memory.h>

static int numkeys;
static Uint8 *previous_state;
static const Uint8 *current_state;


// TODO: Remember to free previous_state 
void kbd_init() {
	current_state = SDL_GetKeyboardState(&numkeys);

	previous_state = malloc(sizeof(Uint8) * numkeys);
}

void kbd_quit() {
	if (previous_state != NULL)
		free(previous_state);
}

// call this once per frame
void kbd_update() {

	// copy the current state into the previous state
	memcpy(previous_state, current_state, sizeof(Uint8) * numkeys);

	// then update the current state
	current_state = SDL_GetKeyboardState(NULL);

}


static int kbd_key_was_down(int key) {
	return previous_state[key] == 1;
}

static int kbd_key_was_up(int key) {
	return previous_state[key] == 0;
}

int kbd_key_is_down(int key) {
	return current_state[key] == 1;
}

int kbd_key_is_up(int key) {
	return current_state[key] == 0;
}

// NOTE: This doesn't work! Fix it.
int kbd_key_was_pressed(int key) {
	return kbd_key_was_up(key) && kbd_key_is_down(key);
}

int kbd_key_was_released(int key) {
	return kbd_key_was_down(key) && kbd_key_is_up(key);
}