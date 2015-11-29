
#include <stdio.h>
#include <math.h>

#include "sdlcommon.h"
#include "kbd.h"

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600
#define WINDOW_TITLE    "Pong Clone"
#define SCORE_FONT_SIZE 16
#define PADDLE_WIDTH    25
#define PADDLE_HEIGHT   75
#define PADDLE_SPEED    500.f // TODO: Make this change based on difficulty if playing AI
#define PADDING         5
#define PUCK_SIZE       10
#define PUCK_SPEED      250.f

struct paddle_s {
	int score;
	float posY;
};

struct game_s {
	int run;
	SDL_Window   *window;
	SDL_Renderer *renderer;
	struct paddle_s leftPaddle;
	struct paddle_s rightPaddle;

	TTF_Font *scoreFont;

	// TODO: this is going to store the top left co-ordinate, but it is important
	//       to remember that the collision code should calculate from the centre
	//       point, not the top left!
	float puckPosX;
	float puckPosY;
	float puckVelX;
	float puckVelY;

} game;

struct sprite_s {
	int w;
	int h;
	SDL_Texture *texture;
};

void update(float elapsedTime);
void draw();
int load_assets();
void unload_assets();
static void draw_scores();
static void draw_paddles();
static void draw_puck();
static void reset_game();
static void reset_puck(float velX);

int *create_text_texture(SDL_Renderer *renderer, const char *s, TTF_Font *font, SDL_Color fgColor, struct sprite_s *sprite);

int main(int argc, char *argv[]) {

	// initialise the various SDL libraries
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "error: Unable to initialise SDL: %s\n", SDL_GetError());
		return 1;
	}

	if (TTF_Init() != 0) {
		fprintf(stderr, "error: Unable to initialise TTF support: %s\n", TTF_GetError());
		return 1;
	}

	game.window = SDL_CreateWindow(
		WINDOW_TITLE,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		0);
	if (game.window == NULL) {
		fprintf(stderr, "error: Unable to create window: %s\n", SDL_GetError());
		return 1;
	}

	game.renderer = SDL_CreateRenderer(
		game.window,
		-1,
		SDL_RENDERER_ACCELERATED);
	if (game.renderer == NULL) {
		fprintf(stderr, "error: Unable to create renderer: %s\n", SDL_GetError());
		return 1;
	}

	// initialise keyboard code
	kbd_init();

	// load game assets (fonts, images, sounds)
	load_assets();

	float deltaTime;
	Uint32 currentTime;
	Uint32 lastTime;

	currentTime = lastTime = 0;
	game.run = 1;
	reset_game();

	while (game.run != 0) {

		currentTime = SDL_GetTicks();
		deltaTime = (float)(currentTime - lastTime) / 1000;
		lastTime = currentTime;

		update(deltaTime);
		draw();

		// try and target 60fps
		//SDL_Delay(33);
	}

	unload_assets();
	kbd_quit();
	TTF_Quit();
	SDL_Quit();

	return 0;
}

void update(float elapsedTime) {

	// left paddle is w,s
	if (kbd_key_is_down(SDL_SCANCODE_W))
		game.leftPaddle.posY -= PADDLE_SPEED * elapsedTime;
	if (kbd_key_is_down(SDL_SCANCODE_S))
		game.leftPaddle.posY += PADDLE_SPEED * elapsedTime;

	// right paddle is up,down
	if (kbd_key_is_down(SDL_SCANCODE_UP))
		game.rightPaddle.posY -= PADDLE_SPEED * elapsedTime;
	if (kbd_key_is_down(SDL_SCANCODE_DOWN))
		game.rightPaddle.posY += PADDLE_SPEED * elapsedTime;

	// move the puck
	game.puckPosX += game.puckVelX * elapsedTime;
	game.puckPosY += game.puckVelY * elapsedTime;

	// ensure paddles aren't off screen
	if (game.leftPaddle.posY < 0)
		game.leftPaddle.posY = 0;
	else if (game.leftPaddle.posY > WINDOW_HEIGHT - PADDLE_HEIGHT)
		game.leftPaddle.posY = WINDOW_HEIGHT - PADDLE_HEIGHT;

	if (game.rightPaddle.posY < 0)
		game.rightPaddle.posY = 0;
	else if (game.rightPaddle.posY > WINDOW_HEIGHT - PADDLE_HEIGHT)
		game.rightPaddle.posY = WINDOW_HEIGHT - PADDLE_HEIGHT;
	
	// scoring
	if (game.puckPosX + PUCK_SIZE >= WINDOW_WIDTH) {
		// ball touched right edge of screen, left scores
		game.leftPaddle.score++;
		// reset the puck, but have it heading towards the right
		reset_puck(abs(game.puckVelX) * 1.0f);
	}

	if (game.puckPosX <= 0) {
		// ball touched left edge of screen, right scores
		game.rightPaddle.score++;
		// reset the puck, but have it heading towards the left
		reset_puck(abs(game.puckVelX) * -1.0f);
	}

	// TODO: consider changing this to avoid getting the puck stuck "zig-zagging"
	// bounce puck from top and bottom of screen
	if (game.puckPosY < 0) {
		// the puck hit the top of the screen - reverse it's Y velocity
		game.puckVelY = abs(game.puckVelY) * 1.0f;
	}
	if (game.puckPosY > WINDOW_HEIGHT) { // todo: add half of the puck_size
		// the puck hit the bottom of the screen - reverse it's Y velocity
		game.puckVelY = abs(game.puckVelY) * -1.0f;
	}

	// paddle/puck collision
	if (game.puckPosX <= PADDING + PADDLE_WIDTH && (game.puckPosY > game.leftPaddle.posY && game.puckPosY < game.leftPaddle.posY + PADDLE_HEIGHT)) {
		// collision with left paddle
		game.puckVelX = abs(game.puckVelX) * 1.0f;
	}

	if (game.puckPosX >= WINDOW_WIDTH - PADDING - PADDLE_WIDTH && 
		 (game.puckPosY > game.rightPaddle.posY && game.puckPosY < game.rightPaddle.posY + PADDLE_HEIGHT)) {
		// collision with right paddle
		game.puckVelX = abs(game.puckVelX) * -1.0f;
	}
	
	SDL_Event evt;
	while (SDL_PollEvent(&evt)) {
		if (evt.type == SDL_QUIT)
			game.run = 0;
	}

}

void draw() {

	// clear to black
	SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 0);
	SDL_RenderClear(game.renderer);

	draw_scores();
	draw_paddles();
	draw_puck();

	// draw everything to screen
	SDL_RenderPresent(game.renderer);
}

int load_assets() {

	game.scoreFont = TTF_OpenFont("..\\assets\\fonts\\LiberationMono-Regular.ttf", SCORE_FONT_SIZE);
	if (game.scoreFont == NULL)
	{
		fprintf(stderr, "error: unable to load scoreboard font: %s\n", TTF_GetError());
		return 1;
	}

	return 0;
}

void unload_assets() {
	if (game.scoreFont)
		TTF_CloseFont(game.scoreFont);
}

int *create_text_texture(SDL_Renderer *renderer, const char *s, TTF_Font *font, SDL_Color fgColor, struct sprite_s *sprite) {

	SDL_Surface *surface = TTF_RenderText_Blended(
		font,
		s,
		fgColor);

	sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	TTF_SizeText(
		font,
		s,
		&sprite->w,
		&sprite->h);

	return 0;
}

static void draw_scores() {

	// render the score to a texture and draw to screen
	SDL_Color white = { 0xFF, 0xFF, 0xFF };
	char scoreText[32];
	struct sprite_s scoreSprite;

	sprintf(scoreText, "%d : %d", game.leftPaddle.score, game.rightPaddle.score);
	create_text_texture(game.renderer, scoreText, game.scoreFont, white, &scoreSprite);

	SDL_Rect scoreRect = {
		(WINDOW_WIDTH / 2) - (scoreSprite.w / 2),
		PADDING,
		scoreSprite.w,
		scoreSprite.h
	};

	// draw score to screen
	SDL_RenderCopy(game.renderer, scoreSprite.texture, NULL, &scoreRect);

}

static void draw_paddles() {

	SDL_Rect leftPaddleRect = {
		PADDING, // offset from left edge of screen
		(int)game.leftPaddle.posY,
		PADDLE_WIDTH,
		PADDLE_HEIGHT
	};

	SDL_Rect rightPaddleRect = {
		WINDOW_WIDTH - PADDLE_WIDTH - PADDING, // offset from right edge
		(int)game.rightPaddle.posY,
		PADDLE_WIDTH,
		PADDLE_HEIGHT
	};

	SDL_SetRenderDrawColor(game.renderer, 0xFF, 0xFF, 0xFF, 0);
	SDL_RenderFillRect(game.renderer, &leftPaddleRect);
	SDL_RenderFillRect(game.renderer, &rightPaddleRect);

}

static void draw_puck() {

	SDL_Rect puckRect = {
		(int)game.puckPosX,
		(int)game.puckPosY,
		PUCK_SIZE,
		PUCK_SIZE
	};

	SDL_SetRenderDrawColor(game.renderer, 0xFF, 0xFF, 0xFF, 0);
	SDL_RenderFillRect(game.renderer, &puckRect);

}

static void reset_game() {

	// TODO: Reset paddles to start positions (centre of screen)

	game.leftPaddle.score = game.rightPaddle.score = 0;
	reset_puck(PUCK_SPEED);

}

static void reset_puck(float velX) {
	game.puckPosX = WINDOW_WIDTH / 2 - PUCK_SIZE / 2;
	game.puckPosY = WINDOW_HEIGHT / 2 - PUCK_SIZE / 2;

	// TODO: The initial direction of the puck should depend on who scored last
	game.puckVelX = velX;
	game.puckVelY = PUCK_SPEED;

}