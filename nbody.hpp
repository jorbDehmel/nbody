/*
Jordan "Jorb" Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef NBODY
#define NBODY

#include <queue>
#include <iostream>
using namespace std;

struct Body
{
    double x, y, vx, vy;
    int collisions = 0;
};

struct Collision
{
    Body *A, *B;
    int collisionsA, collisionsB;
    double time;
};

class __collision_order
{
public:
    // Returns true if B should go in front of A
    bool operator()(const Collision &A, const Collision &B) const;
};

extern double minX, maxX, minY, maxY;
double __predictCollisionTime(const Body *const A, const Body *const B);

class CollisionHandler
{
public:
    CollisionHandler(Body **ToMonitor, const int &N, void (*OnCollision)(Collision *), double (*PredictCollision)(const Body *const A, const Body *const B) = __predictCollisionTime);

    void next();
    bool empty() const;

    void setTime(const double &To);
    double getTime() const;

protected:
    double t;

    Body **bodies;
    int numBodies;

    void (*onCollision)(Collision *);
    double (*predictCollision)(const Body *const A, const Body *const B);

    priority_queue<Collision, vector<Collision>, __collision_order> events;

    void addCollision(Body *A, Body *B);
};

#endif
