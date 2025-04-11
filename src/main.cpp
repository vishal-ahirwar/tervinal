// Enhanced SDL Terminal with scrolling, command history, and autocomplete
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <algorithm>
#include <boost/process.hpp>
#include<tervinalconfig.h>
constexpr int WIDTH{800};
constexpr int HEIGHT{600};
constexpr int RADIUS{16};

void drawRoundedBackground(SDL_Renderer* renderer, int x, int y, int w, int h, int radius, SDL_Color color) {
    // Set color
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

    // 1. Draw the central rectangle (minus the corners)
    SDL_Rect centerRect = {x + radius, y, w - 2 * radius, h};
    SDL_RenderFillRect(renderer, &centerRect);

    SDL_Rect leftRect = {x, y + radius, radius, h - 2 * radius};
    SDL_Rect rightRect = {x + w - radius, y + radius, radius, h - 2 * radius};
    SDL_RenderFillRect(renderer, &leftRect);
    SDL_RenderFillRect(renderer, &rightRect);

    // 2. Draw the 4 quarter circles manually
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy <= radius * radius) {
                // Top-left
                SDL_RenderDrawPoint(renderer, x + radius + dx, y + radius + dy);
                // Top-right
                SDL_RenderDrawPoint(renderer, x + w - radius + dx, y + radius + dy);
                // Bottom-left
                SDL_RenderDrawPoint(renderer, x + radius + dx, y + h - radius + dy);
                // Bottom-right
                SDL_RenderDrawPoint(renderer, x + w - radius + dx, y + h - radius + dy);
            }
        }
    }
}

SDL_Texture *renderText(const std::string &text, TTF_Font *font, SDL_Color color, SDL_Renderer *renderer, SDL_Rect &outRect)
{
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surf)
        return nullptr;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    outRect = {0, 0, surf->w, surf->h};
    SDL_FreeSurface(surf);
    return tex;
}

std::vector<std::string> splitLines(const std::string &input)
{
    std::vector<std::string> lines;
    std::stringstream ss(input);
    std::string line;
    while (std::getline(ss, line, '\n'))
        lines.push_back(line);
    return lines;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("SDL Terminal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    TTF_Font *font = TTF_OpenFont("res/fonts/FiraCode-Regular.ttf", 18);
    SDL_Color textColor = {0, 0, 0}, bgColor = {255, 255, 255};

    std::string inputBuffer;
    std::deque<std::string> commandHistory;
    int historyIndex = -1;
    std::vector<std::string> lines;
    int scrollOffset = 0;
    const int lineSpacing = 28;

    std::vector<std::string> commands = {"help", "exit", "version", "clear"};
    std::string currentSuggestion;

    Uint32 lastBlink = SDL_GetTicks();
    bool showCursor = true, running = true;
    SDL_Event e;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;
            if (e.type == SDL_KEYDOWN)
            {
                SDL_Keycode key = e.key.keysym.sym;
                if (key == SDLK_ESCAPE)
                    running = false;
                else if (key == SDLK_BACKSPACE && !inputBuffer.empty())
                    inputBuffer.pop_back();
                else if (key == SDLK_RETURN)
                {
                    lines.push_back(">> " + inputBuffer);
                    commandHistory.push_front(inputBuffer);
                    historyIndex = -1;
                    // Handle command execution
                    if (inputBuffer == "clear")
                        lines.clear();
                    else if (inputBuffer == "help")
                        lines.push_back("Available: help, exit, version, clear");
                    else if (inputBuffer == "version")
                        lines.push_back("Tervinal v"+std::string(Project::VERSION_STRING));
                    else if (inputBuffer == "exit")
                        running = false;
                    inputBuffer.clear();
                    currentSuggestion.clear();
                }
                else if (key == SDLK_UP && !commandHistory.empty())
                {
                    historyIndex = std::min((int)commandHistory.size() - 1, historyIndex + 1);
                    inputBuffer = commandHistory[historyIndex];
                }
                else if (key == SDLK_DOWN && historyIndex >= 0)
                {
                    historyIndex--;
                    inputBuffer = historyIndex >= 0 ? commandHistory[historyIndex] : "";
                }
                else if (key >= 32 && key <= 126)
                {
                    inputBuffer += static_cast<char>(key);
                }

                if (!inputBuffer.empty())
                {
                    auto it = std::find_if(commands.begin(), commands.end(), [&](const std::string &cmd)
                                           { return cmd.rfind(inputBuffer, 0) == 0; });
                    currentSuggestion = (it != commands.end() && *it != inputBuffer) ? *it : "";
                }
                else
                    currentSuggestion.clear();
            }
            else if (e.type == SDL_MOUSEWHEEL)
            {
                scrollOffset -= e.wheel.y * lineSpacing;
                scrollOffset = std::max(0, scrollOffset);
            }
        }

        if (SDL_GetTicks() - lastBlink > 500)
        {
            showCursor = !showCursor;
            lastBlink = SDL_GetTicks();
        }

        drawRoundedBackground(renderer, 0, 0, WIDTH, HEIGHT, RADIUS, bgColor);

        int y = 10 - scrollOffset;
        for (const auto &line : lines)
        {
            SDL_Rect r;
            SDL_Texture *t = renderText(line, font, textColor, renderer, r);
            r.x = 10;
            r.y = y;
            SDL_RenderCopy(renderer, t, nullptr, &r);
            SDL_DestroyTexture(t);
            y += lineSpacing;
        }

        std::string activeLine = ">> " + inputBuffer + (showCursor ? "|" : "");
        SDL_Rect inputRect;
        SDL_Texture *inputTex = renderText(activeLine, font, textColor, renderer, inputRect);
        inputRect.x = 10;
        inputRect.y = y;
        SDL_RenderCopy(renderer, inputTex, nullptr, &inputRect);
        SDL_DestroyTexture(inputTex);

        if (!currentSuggestion.empty())
        {
            SDL_Rect sugRect;
            SDL_Texture *sugTex = renderText(currentSuggestion, font, {128, 128, 128}, renderer, sugRect);
            sugRect.x = 20 + inputRect.w;
            sugRect.y = y;
            SDL_RenderCopy(renderer, sugTex, nullptr, &sugRect);
            SDL_DestroyTexture(sugTex);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
