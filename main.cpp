/*
Jordan "Jorb" Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include <SDL2/SDL.h>
#include <iostream>
#include "nbody.hpp"
using namespace std;

void onCollision(Collision *What)
{
    if (What->A != nullptr)
    {
        What->A->vx *= -0.5;
        What->A->vy *= -0.5;
    }
    else
    {
        cout << "BOUNCE\n";
    }

    if (What->B != nullptr)
    {
        What->B->vx *= -0.5;
        What->B->vy *= -0.5;
    }
    else
    {
        cout << "BOUNCE\n";
    }

    return;
}

int main(const int argc, const char *argv[])
{
    srand(time(NULL));

    const int numBodies = 20;
    Body bodies[numBodies];

    for (int i = 0; i < numBodies; i++)
    {
        bodies[i].x = (rand() % 101) - 50;
        bodies[i].y = (rand() % 101) - 50;

        bodies[i].vx = (rand() % 11) - 5;
        bodies[i].vy = (rand() % 11) - 5;
    }

    Body *refs[numBodies];
    for (int i = 0; i < numBodies; i++)
    {
        refs[i] = &bodies[i];
    }

    CollisionHandler handler((Body **)refs, numBodies, onCollision);

    // SDL2 stuff for visualizing it
    SDL_Renderer *rend;
    SDL_Window *wind;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(512, 512, SDL_WINDOW_OPENGL, &wind, &rend);

    bool isRunning = true;
    for (double t = 0; isRunning; t += 0.1)
    {
        handler.setTime(t);

        // Clear renderer
        SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
        SDL_RenderClear(rend);

        // Render bodies
        for (int i = 0; i < numBodies; i++)
        {
            SDL_SetRenderDrawColor(rend, (i * 16) % 255, 255, 255, 255);

            SDL_FRect rect{0, 0, 0, 0};
            rect.x = 256 + bodies[i].x;
            rect.y = 256 + bodies[i].y;
            rect.w = max(bodies[i].vx, 1.0);
            rect.h = max(bodies[i].vy, 1.0);

            SDL_RenderFillRectF(rend, &rect);
        }

        SDL_RenderPresent(rend);
        SDL_Delay(1);

        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case 27:
                    isRunning = false;
                    break;
                }
            }
        }
    }

    SDL_DestroyWindow(wind);
    SDL_DestroyRenderer(rend);
    SDL_Quit();

    for (int i = 0; i < numBodies; i++)
    {
        cout << "i=" << i << ": " << bodies[i].collisions << '\n';
    }

    return 0;
}
