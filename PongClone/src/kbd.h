
#pragma once

void kbd_init();
void kbd_quit();
void kbd_update();

// is the key currently pressed
int kbd_key_is_down(int key);

// is the key currently not pressed
int kbd_key_is_up(int key);

// was the key pressed (it wasn't down and it now is) since the last frame
int kbd_key_was_pressed(int key);

// was the key released since the last frame
int kbd_key_was_released(int key);

