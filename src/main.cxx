#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdbool.h>
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Surface *screen = NULL;
SDL_Surface *image = NULL;

static bool is_running = false;

const int BUTTON_WIDTH = 300;
const int BUTTON_HEIGHT = 200;
const int TOTAL_BUTTONS = 4;

enum ButtonSprite
{
    BUTTON_SPRITE_MOUSE_OUT = 0,
    BUTTON_SPRITE_MOUSE_OVER_MOTION = 1,
    BUTTON_SPRITE_MOUSE_DOWN = 2,
    BUTTON_SPRITE_MOUSE_UP = 3,
    BUTTON_SPRITE_TOTAL = 4
};
bool init_sdl() {
    bool success = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL init ERR: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);
        if (SDL_MUSTLOCK(screen))
            SDL_LockSurface(screen);
        if (screen == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        if (SDL_MUSTLOCK(screen))
            SDL_UnlockSurface(screen);

        SDL_Flip(screen); // swap screen buffer
    }
    return success;
}

bool load_media()
{
    bool success = true;
    int flags = IMG_INIT_PNG;
    if (!(IMG_Init(flags) & flags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }

    image = IMG_Load("src/smile.png");
    if (image == NULL)
    {
        printf("Unable to load image %s! SDL Error: %s\n", "smile.png", IMG_GetError());
        success = false;
    }
    return success;
}

void close_sdl()
{
    SDL_FreeSurface(image);
    SDL_FreeSurface(screen);
    SDL_Quit();
}

static void mainloop(void)
{
    SDL_Event e;
    if (!is_running)
    {
        // clean up
        emscripten_cancel_main_loop();
        close_sdl();
    }

    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            is_running = false;
        }
    }
    // run it
}

int main(int argc, char *argv[])
{
    EM_ASM({ console.log('Hello from C!'); }); // remove me
    // SDL stuff

    if (!init_sdl())
    {
        printf("Failed to initialize!\n");
    }
    else
    {
        // Load media
        if (!load_media())
        {
            printf("Failed to load media!\n");
        }
        else
        {
            // Apply the image
            SDL_BlitSurface(image, NULL, screen, NULL);
        }
    }
    close_sdl();
    return 0;
}