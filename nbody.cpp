/*
Jordan "Jorb" Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "nbody.hpp"

// Returns true if B should go in front of A
bool __collision_order::operator()(const Collision &A, const Collision &B) const
{
    return B.time < A.time;
}

// If ToMonitor is dynamically allocated, that's the user's problem
CollisionHandler::CollisionHandler(Body **ToMonitor, const int &N, void (*OnCollision)(Collision *), double (*PredictCollision)(const Body *const A, const Body *const B))
{
    // Localize vars
    bodies = ToMonitor;
    numBodies = N;
    onCollision = OnCollision;
    predictCollision = PredictCollision;

    // Initialize pq: O(n^2)
    for (int i = 0; i < numBodies; i++)
    {
        for (int j = i + 1; j < numBodies; j++)
        {
            addCollision(bodies[i], bodies[j]);
        }

        // Wall collisions
        addCollision(bodies[i], nullptr);
        addCollision(nullptr, bodies[i]);
    }

    return;
}

void CollisionHandler::next()
{
    // Get the next non-invalidated event
    Collision event;
    do
    {
        if (events.empty())
        {
            return;
        }

        event = events.top();
        events.pop();
    } while (event.time < 0 || (event.A != nullptr && event.collisionsA != event.A->collisions) || (event.B != nullptr && event.collisionsB != event.B->collisions));

    // Update time
    double deltaT = event.time - t;
    t = event.time;

    // Update all body's position values
    for (int i = 0; i < numBodies; i++)
    {
        bodies[i]->x += bodies[i]->vx * deltaT;
        bodies[i]->y += bodies[i]->vy * deltaT;
    }

    // Call the appropriate collision handling function
    onCollision(&event);

    // Increment body collision counts
    if (event.A != nullptr)
    {
        event.A->collisions++;
    }

    if (event.B != nullptr)
    {
        event.B->collisions++;
    }

    // Recalculate necessary collisions: O(2n)
    // For body A:
    for (int i = 0; i < numBodies; i++)
    {
        // Object cannot collide with itself (otherwise we have bigger concerns)
        if (bodies[i] == event.A)
        {
            continue;
        }

        // Otherwise, add collision
        else
        {
            addCollision(event.A, bodies[i]);
        }
    }

    // Wall collisions
    addCollision(event.A, nullptr);
    addCollision(nullptr, event.A);

    // For body B:
    for (int i = 0; i < numBodies; i++)
    {
        // Object cannot collide with itself (otherwise we have bigger concerns)
        if (bodies[i] == event.B)
        {
            continue;
        }

        // Otherwise, add collision
        else
        {
            addCollision(event.B, bodies[i]);
        }
    }

    // Wall collisions
    addCollision(event.B, nullptr);
    addCollision(nullptr, event.B);

    return;
}

bool CollisionHandler::empty() const
{
    return events.empty();
}

void CollisionHandler::setTime(const double &To)
{
    // Handle any collisions that will happen in the meantime
    while (!events.empty() && events.top().time < To)
    {
        next();
    }

    // Update positions
    double deltaT = To - t;
    for (int i = 0; i < numBodies; i++)
    {
        bodies[i]->x += bodies[i]->vx * deltaT;
        bodies[i]->y += bodies[i]->vy * deltaT;
    }

    // Update time
    t = To;

    return;
}

double CollisionHandler::getTime() const
{
    return t;
}

/*
If A and B will collide, there will be some t such that the following system holds.

A_{x_0} + A_{v_x} t = B_{x_0} + B_{v_x} t
A_{y_0} + A_{v_y} t = B_{y_0} + B_{v_y} t

Algebraically for the first equation:
A_{v_x} t - B_{v_x} t = B_{x_0} - A_{x_0}
t (A_{v_x} - B_{v_x}) = B_{x_0} - A_{x_0}

t_x = (B_{x_0} - A_{x_0}) / (A_{v_x} - B_{v_x})

Similarly,

t_y = (B_{y_0} - A_{y_0}) / (A_{v_y} - B_{v_y})

If t_y == t_x, the answer is either of these. Otherwise, it's -1.

*/

// Using the current velocities and positions, predict the collision time of two bodies
// NOTE: Denotes no collision as -1
double minX = -100, maxX = 100, minY = -100, maxY = 100;
double __predictCollisionTime(const Body *const A, const Body *const B)
{
    double tx, ty;

    // Neither are real bodies
    if (A == nullptr && B == nullptr)
    {
        // Wall colliding with wall, failure case
        return -2;
    }

    // Vertical wall: Only x matters
    else if (B == nullptr)
    {
        if (A->vx > 0)
        {
            // Right wall
            return (maxX - A->x) / A->vx;
        }
        else if (A->vx < 0)
        {
            // Left wall
            return (minX - A->x) / A->vx;
        }
        else
        {
            return -1;
        }
    }

    // Horizontal wall: Only y matters
    else if (A == nullptr)
    {
        if (B->vx > 0)
        {
            // Roof
            return (B->y - minY) / -B->vy;
        }
        else if (B->vx < 0)
        {
            // Floor
            return (B->y - maxY) / -B->vy;
        }
        else
        {
            return -1;
        }
    }

    // Both real bodies
    else
    {
        // Much easier equations than I was expecting
        tx = (B->x - A->x) / (A->vx - B->vx);
        ty = (B->y - A->y) / (A->vy - B->vy);
    }

    // cout << "tx: " << tx << '\n'
    //      << "ty: " << ty << '\n';

    if (tx != tx && ty != ty)
    {
        // Both numbers are NaN: No collision
        return -1;
    }
    else if (tx != tx)
    {
        // tx is NaN: Only ty matters
        return ty;
    }
    else if (ty != ty)
    {
        // ty is NaN: Only tx matters
        return tx;
    }
    else if (tx == ty)
    {
        return tx;
    }
    else
    {
        return -1;
    }
}

void CollisionHandler::addCollision(Body *A, Body *B)
{
    // Predict the time of collision
    double predicted = predictCollision(A, B);

    // If value is garbage, continue
    // NOTE: Infinity is denoted by -1 in this case, since time will only be positive
    if (predicted < 0)
    {
        return;
    }

    // Allow for nullptr entries
    int collisionsA, collisionsB;
    collisionsA = (A == nullptr) ? 0 : A->collisions;
    collisionsB = (B == nullptr) ? 0 : B->collisions;

    // Construct collision event at the calculated time
    Collision toAdd{A,
                    B,
                    collisionsA,
                    collisionsB,
                    predicted};

    // Add collision to event pq
    events.push(toAdd);

    return;
}
