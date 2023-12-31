#include "Graphics.h"
#include "Physics.h"
#include "Game.h"
#include <stdio.h>
#include <queue>

using hlslpp::float2;
using hlslpp::float3;

bool run;

float deltaTime = 1.0f;
float time;

bool leftHeld = false;
bool rightHeld = false;
bool upHeld = false;
bool spaceHeld = false;

int scoreDisplay = 0;
int scoreRendered = -1;
int finalScore = 0;

unsigned int gameTick = 0;

bool nuke = false;

SDL_Window* window;

TTF_Font* font;

void handleInput(SDL_Event event)
{
	if (event.type == SDL_KEYDOWN)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_UP:
			upHeld = true;
			break;
			
		case SDLK_SPACE:
			spaceHeld = true;
			break;

		case SDLK_LEFT:
			leftHeld = true;
			break;

		case SDLK_RIGHT:
			rightHeld = true;
			break;

		default:
			break;
		}

	}
	if (event.type == SDL_KEYUP)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_UP:
			upHeld = false;
			break;

		case SDLK_SPACE:
			spaceHeld = false;
			break;

		case SDLK_LEFT:
			leftHeld = false;
			break;

		case SDLK_RIGHT:
			rightHeld = false;
			break;

		default:
			break;
		}

	}


}

bool init()
{
	bool ok = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		ok = false;
	}

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags) && ok)
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		ok = false;
	}

	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		ok = false;
	}

	font = TTF_OpenFont("font.ttf", 28);
	if (font == NULL)
	{
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		ok = false;
	}

	if (ok)
	{
		window = SDL_CreateWindow("Space War", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WindowFlags::SDL_WINDOW_ALLOW_HIGHDPI);

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		ok = renderer;
	}

	return ok;
}

int main(int argumentCount, char * arguments[])
{
	if (!init())
	{
		printf("init failed :c\n");
		return EXIT_FAILURE;
	}

	if (!LoadSpriteAtlas("testatlas.png"))
	{
		return EXIT_FAILURE;
	}

	// most objects stack allocated here
	PlayerShip player;

	std::vector<Enemy*> enemies;
	float enemyTimer = 0.0f;
	float enemyCountdown = 1000.0f;

	const float tickSpeed = 200.0f;
	float tickTimer = tickSpeed;

	Text txt;	// text render object
	txt.Set(float2(0.0f, 0.0f));
	
	SDL_Event e;
	run = true;
	while (run)
	{
		deltaTime = SDL_GetTicks() - time;
		if (deltaTime > 250) deltaTime = 250;
		time = SDL_GetTicks();

		// increment tick counter if last tick was longer ago than tickspeed and reset timer
		if (tickTimer < 0.0f)
		{
			gameTick++;
			tickTimer = tickSpeed;
		}
		else
		{
			tickTimer = tickTimer - deltaTime;
		}

		// handle inputs and events
		while (SDL_PollEvent(&e) != 0)
		{
			handleInput(e);
			if (e.type == SDL_QUIT)
			{
				run = false;
			}

		}
		if (leftHeld)
		{
			player.angle -= 0.0005 * deltaTime;
		}
		if (rightHeld)
		{
			player.angle += 0.0005 * deltaTime;
		}
		if (upHeld)
		{
			player.Thruster();
		}
		else
		{
			player.sprite.Set(0);
		}
		if (spaceHeld)
		{
			player.Fire();
		}

		//prerender section

		if (enemyTimer < 0.0f)	// spawning of new enemies on timer
		{
			enemies.push_back(new Enemy());
			enemyTimer = enemyCountdown;
			if (enemyCountdown > 250.0f)
			{
				enemyCountdown = enemyCountdown - 5.0f;
			}
		}
		else if (enemyTimer >= 0.0f)
		{
			enemyTimer = enemyTimer - deltaTime;
		}


		if (scoreDisplay != scoreRendered)	// only update score if onscreen is different from actual
		{
			int len = snprintf(NULL, 0, "SCORE: %06d", scoreDisplay);
			char* result = (char*)malloc(len + 1);
			snprintf(result, len + 1, "SCORE: %06d", scoreDisplay);
			txt.Set(result);
			scoreRendered = scoreDisplay;
			free(result);
		}

		Collider::UpdateColliders();
		
		std::vector<int> killIdxs;
		player.Update(deltaTime);
		for (int i = 0; i < enemies.size(); i++) // update enemies, enemy returns true if it wants to get deleted
		{
			if (enemies[i]->Update(deltaTime))
			{
				killIdxs.push_back(i);
			}
		}

		for (int i = killIdxs.size() - 1; i >= 0; i--) // kill enemies that are on the kill list
		{
			delete(enemies[killIdxs[i]]);
			enemies.erase(enemies.begin() + killIdxs[i]);
		}


		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
		SDL_RenderClear(renderer);

		//render section
		for (RenderObject* s : RenderObjects)	// draw all objects
		{
			s->Render();
		}

		player.sprite.Render(); // hack to always render player on top

		SDL_RenderPresent(renderer);

		if (nuke)	// if we want to kill all enemies (used when player dies)
		{
			nuke = false;
			for (int i = 0; i < enemies.size(); i++)
			{
				enemies[i]->Destroy();
			}
		}

		
	}

	

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = NULL;
	window = NULL;

	IMG_Quit();
	SDL_Quit();

	return EXIT_SUCCESS;
}