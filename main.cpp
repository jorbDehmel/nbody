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
    cout << "A collision occurred!\n"
         << "X: " << What->A->x << '\n'
         << "Y: " << What->A->y << '\n'
         << "T: " << What->time << '\n';

    return;
}

int main(const int argc, const char *argv[])
{
    const int numBodies = 2;
    Body bodies[numBodies];

    bodies[0].x = -50;
    bodies[0].vx = 1;

    bodies[1].x = 50;
    bodies[1].vx = -1;

    bodies[0].y = bodies[1].y = 50;
    bodies[0].vy = bodies[1].vy = 1;

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

    for (double t = 0; t < 100; t += 0.1)
    {
        handler.setTime(t);

        // Clear renderer
        SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
        SDL_RenderClear(rend);

        // Render bodies
        for (int i = 0; i < numBodies; i++)
        {
            SDL_SetRenderDrawColor(rend, i * 255, 255, 255, 255);

            SDL_FRect rect{0, 0, 0, 0};
            rect.x = 256 + bodies[i].x;
            rect.y = 256 + bodies[i].y;
            rect.w = max(bodies[i].vx, 1.0);
            rect.h = max(bodies[i].vy, 1.0);

            SDL_RenderFillRectF(rend, &rect);
            SDL_Delay(1);
        }

        SDL_RenderPresent(rend);
    }

    SDL_DestroyWindow(wind);
    SDL_DestroyRenderer(rend);
    SDL_Quit();

    return 0;
}
