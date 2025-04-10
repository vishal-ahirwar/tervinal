#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

SDL_Texture* renderText(const std::string& text, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer, SDL_Rect& outRect) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), color); // Better quality
    if (!surf) return nullptr;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    outRect = { 0, 0, surf->w, surf->h };
    SDL_FreeSurface(surf);
    return tex;
}

std::vector<std::string> splitLines(const std::string& input) {
    std::vector<std::string> lines;
    std::stringstream ss(input);
    std::string line;

    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }

    return lines;
}

int main(int arc,char**argv) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("SDL Terminal",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("res/fonts/FiraCode-Regular.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << "\n";
        return 1;
    }

    SDL_Color black = { 0, 0, 0 };
    SDL_Color white = { 255, 255, 255 };

    SDL_Surface* welcomeSurf = TTF_RenderText_Blended(font, "Tervinal (C)2025", black);
    SDL_Texture* welcomeTex = SDL_CreateTextureFromSurface(renderer, welcomeSurf);
    SDL_Rect welcomeRect = { 250, 10, welcomeSurf->w, welcomeSurf->h };
    SDL_FreeSurface(welcomeSurf);

    std::string inputBuffer;
    bool running = true;
    SDL_Event e;

    Uint32 lastBlinkTime = SDL_GetTicks();
    bool showCursor = true;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode key = e.key.keysym.sym;

                if (key == SDLK_BACKSPACE && !inputBuffer.empty()) {
                    inputBuffer.pop_back();
                } else if (key == SDLK_RETURN) {
                    inputBuffer += '\n';
                } else if (key == SDLK_ESCAPE) {
                    running = false;
                } else if (key >= 32 && key <= 126) {
                    inputBuffer += static_cast<char>(key);
                }
            }
        }

        // Handle cursor blink every 500ms
        if (SDL_GetTicks() - lastBlinkTime > 500) {
            showCursor = !showCursor;
            lastBlinkTime = SDL_GetTicks();
        }

        // Clear screen with white background
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Draw welcome text
        SDL_RenderCopy(renderer, welcomeTex, nullptr, &welcomeRect);

        // Split lines and render each
        std::vector<std::string> lines = splitLines(inputBuffer);
        int y = welcomeRect.y + welcomeRect.h + 10;
        for (size_t i = 0; i < lines.size(); ++i) {
            SDL_Rect rect;
            std::string displayLine = ">>" + lines[i];
            SDL_Texture* textTex = renderText(displayLine, font, black, renderer, rect);
            rect.x = 10;
            rect.y = y;
            SDL_RenderCopy(renderer, textTex, nullptr, &rect);
            SDL_DestroyTexture(textTex);
            y += rect.h;
        }

        // Render blinking cursor
        if (showCursor) {
            std::string lastLine = (lines.empty() ? "" : lines.back());
            SDL_Rect cursorRect;
            std::string displayCursor = ">>" + lastLine + "|";
            SDL_Texture* cursorTex = renderText(displayCursor, font, black, renderer, cursorRect);
            cursorRect.x = 10;
            cursorRect.y = y - (lines.empty() ? 0 : cursorRect.h);
            SDL_RenderCopy(renderer, cursorTex, nullptr, &cursorRect);
            SDL_DestroyTexture(cursorTex);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyTexture(welcomeTex);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
