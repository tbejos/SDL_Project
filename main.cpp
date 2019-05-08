#include <string>
#include <cstdio>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "include/res_path.h"
#include "include/cleanup.h"

// Screen attributes
const int SCREEN_WIDTH = 244 * 3;
const int SCREEN_HEIGHT = 288 * 3;
// Using Square Tiles 40x40
const int TILE_SIZE = 40;

// Speed Attributes
const int xVel = 10;
const int yVel = 10;

/**
* Quit all of the SDL libraries loaded
*/
void quitAll() {
    Mix_Quit();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return;
}

/**
* Print an error to STDOUT
* @param function The function that caused the error
* @param error The error message from SDL libraries
*       function Error: [Error_Message]
*/
void printError(const char *function, const char *error) {
    printf("%s Error: %s\n", function, error);
    return;
}

/**
* Loads a BMP image into a texture on the rendering device
* @param file The BMP image file to load
* @param ren The renderer to load the texture onto
* @return the loaded texture, or nullptr if something went wrong.
*/
SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren) {
   SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
   if (texture == nullptr) {
       printError("LoadTexture", SDL_GetError());
   }
   return texture;
}

/**
* Loads a WAV/OGG/AIFF file into a music pointer
* @param file The WAV audio file to load
* @return the loaded song, or nullptr if something went wrong.
*/
Mix_Music* loadMusic(const std::string &file) {
    Mix_Music *song = Mix_LoadMUS(file.c_str());
    if (song == nullptr) {
        printError("Mix_LoadMUS", Mix_GetError());
    }
    return song;
}

/**
* Draw an SDL_Texture to an SDL_Renderer at some destination rect
* taking a clip of the texture if desired
* @param tex The source texture we want to draw
* @param ren The renderer we want to draw to
* @param dst The destination rectangle to render the texture to
* @param clip The sub-section of the texture to draw (clipping rect)
*       default of nullptr draws the entire texture
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst,
    SDL_Rect *clip = nullptr) {

    SDL_RenderCopy(ren, tex, clip, &dst);
    return;
}

/**
* Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
* the texture's width and height and taking a clip of the texture if desired
* If a clip is passed, the clip's width and height will be used instead of
*   the texture's
* @param tex The source texture we want to draw
* @param ren The renderer we want to draw to
* @param x The x coordinate to draw to
* @param y The y coordinate to draw to
* @param clip The sub-section of the texture to draw (clipping rect)
*       default of nullptr draws the entire texture
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y,
    SDL_Rect *clip = nullptr) {

    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    if (clip != nullptr) {
        dest.w = clip->w;
        dest.h = clip->h;
    }
    else {
        SDL_QueryTexture(tex, NULL, NULL, &dest.w, &dest.h);
    }
    renderTexture(tex, ren, dest, clip);
    return;
}

/**
* Render the message we want to display to a texture for drawing
* @param message The message we want to display
* @param fontFile The font we want to use to render the text
* @param color The color we want the text to be
* @param fontSize The size we want the font to be
* @param renderer The renderer to load the texture in
* @return An SDL_Texture containing the rendered message, or nullptr if
*       something went wrong
*/
SDL_Texture* renderText(const std::string &message, TTF_Font *font,
    SDL_Color color, SDL_Renderer *renderer) {

    SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
    if (surf == nullptr) {
        printError("TTF_RenderText", TTF_GetError());
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (texture == nullptr) {
        printError("SDL_CreateTexture", SDL_GetError());
    }

    cleanup(surf);
    return texture;
}

/**
* Where the game is run
* @param argc The number of command line arguments
* @param argv The command line arguments
* @return 0 if successful, 1 if an error
*/
int main(int argc, char **argv) {
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printError("SDL_Init", SDL_GetError());
        return 1;
    }

    /* Initialize SDL_mixer */
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printError("Mix_OpenAudio", Mix_GetError());
        quitAll();
        return 1;
    }

    /* Initialize SDL_image */
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG){
        printError("IMG_Init", IMG_GetError());
        quitAll();
        return 1;
    }

    /* Initialize SDL_ttf */
    if (TTF_Init() != 0){
        printError("TTF_Init", TTF_GetError());
        quitAll();
        return 1;
    }

    /* Open Window */
    SDL_Window *window = SDL_CreateWindow("tbejos Game", 100, 100, SCREEN_WIDTH,
        SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        printError("SDL_CreateWindow", SDL_GetError());
        quitAll();
        return 1;
    }

    /* Create Renderer */
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        cleanup(window);
        printError("SDL_CreateRenderer", SDL_GetError());
        quitAll();
        return 1;
    }

    /* Load Fonts */
    const std::string fontPath = getResourcePath("font");
    TTF_Font *font = TTF_OpenFont((fontPath + "Inconsolata-LGC.ttf").c_str(), 48);
    if (font == nullptr) {
        cleanup(renderer, window);
        printError("TTF_OpenFont", TTF_GetError());
        quitAll();
        return 1;
    }
    SDL_Color color = {255, 255, 255, 255};
    SDL_Texture *text= renderText("TTF fonts are cool!", font, color, renderer);

    /* Load Image */
    const std::string imgPath = getResourcePath("img");
    SDL_Texture *image = loadTexture(imgPath + "image.png", renderer);
    if (image == nullptr || text == nullptr) {
        cleanup(font, image, text, renderer, window);
        quitAll();
        return 1;
    }

    /* Load Audio */
    const std::string audioPath = getResourcePath("audio");
    Mix_Music *song = loadMusic(audioPath + "(c)song.ogg");
    if (song == nullptr) {
        cleanup(song, font, image, text, renderer, window);
        quitAll();
        return 1;
    }

    // iW and iH are the clip width and height
    // We'll be drawing only clips so get center position for the w/h of a clip
    int iW = 100, iH = 100;
    int iX = SCREEN_WIDTH / 2 - iW / 2;
    int iY = SCREEN_HEIGHT / 2 - iH / 2;

    // Setup the clips for our image
    SDL_Rect clips[4];
    // Since our clips our uniform in size we can generate a list of their
    // positions using some math
    for (int i = 0; i < 4; ++i){
        clips[i].x = i / 2 * iW;
        clips[i].y = i % 2 * iH;
        clips[i].w = iW;
        clips[i].h = iH;
    }
    // Specify a default clip to start with
    int useClip = 0;

    // Text width and height
    int tW, tH;
    SDL_QueryTexture(text, NULL, NULL, &tW, &tH);
    int tX = SCREEN_WIDTH / 2 - tW / 2;
    int tY = SCREEN_HEIGHT / 2 - tH / 2;

    // Start Music
    Mix_PlayMusic(song, -1);

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    /* Spritesheet */
                    case SDLK_1:
                    case SDLK_KP_1:
                        useClip = 0;
                        break;
                    case SDLK_2:
                    case SDLK_KP_2:
                        useClip = 1;
                        break;
                    case SDLK_3:
                    case SDLK_KP_3:
                        useClip = 2;
                        break;
                    case SDLK_4:
                    case SDLK_KP_4:
                        useClip = 3;
                        break;
                    /* Movement */
                    case SDLK_UP:
                    case SDLK_w:
                        if (iY >= 0)
                            iY -= yVel;
                        if (iY < 0)
                            iY = 0;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        if (iY + iH <= SCREEN_HEIGHT)
                            iY += yVel;
                        if (iY + iH > SCREEN_HEIGHT)
                            iY = SCREEN_HEIGHT - iH;
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        if (iX >= 0)
                            iX -= xVel;
                        if (iX < 0)
                            iX = 0;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        if (iX + iW <= SCREEN_WIDTH)
                            iX += xVel;
                        if (iX + iW > SCREEN_WIDTH)
                            iX = SCREEN_WIDTH - iW;
                        break;
                    /* Audio Control */
                    case SDLK_m:
                        if(Mix_PlayingMusic() == 0) {
                            Mix_PlayMusic(song, -1);
                        } else {
                            if(Mix_PausedMusic() == 1) {
                                Mix_ResumeMusic();
                            } else {
                                Mix_PauseMusic();
                            }
                        }
                        break;
                    /* Quit */
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    default:
                        break;
                }
            }
        }
        // Render the scene
        SDL_RenderClear(renderer);
        renderTexture(image, renderer, iX, iY, &clips[useClip]);
        renderTexture(text, renderer, tX, tY);
        SDL_RenderPresent(renderer);
    }

    Mix_HaltMusic();
    cleanup(song, font, image, renderer, window);
    quitAll();
    return 0;
}