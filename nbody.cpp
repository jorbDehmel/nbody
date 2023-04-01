#include "nbody.hpp"

// Returns true if B should go in front of A
bool __collision_order::operator()(const Collision &A, const Collision &B) const
{
    return B.time < A.time;
}

// If ToMonitor is dynamically allocated, that's the user's problem
CollisionHandler::CollisionHandler(Body **ToMonitor, const int &N, void (*OnCollision)(Collision *))
{
    // Localize vars
    bodies = ToMonitor;
    numBodies = N;
    onCollision = OnCollision;

    // Initialize pq: O(n^2)
    for (int i = 0; i < numBodies; i++)
    {
        for (int j = i + 1; j < numBodies; j++)
        {
            // Predict the time of collision
            double predicted = predictCollisionTime(bodies[i], bodies[j]);

            // Construct collision event at the calculated time
            Collision toAdd{bodies[i],
                            bodies[j],
                            bodies[i]->collisions,
                            bodies[j]->collisions,
                            predicted};

            // Add collision to event pq
            events.push(toAdd);
        }
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
    } while (event.time < 0 || event.collisionsA != event.A->collisions || event.collisionsB != event.B->collisions);

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
    event.A->collisions++;
    event.B->collisions++;

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
            // Predict the time of collision
            double predicted = predictCollisionTime(event.A, bodies[i]);

            // Construct collision event at the calculated time
            Collision toAdd{event.A,
                            bodies[i],
                            event.A->collisions,
                            bodies[i]->collisions,
                            predicted};

            // Add collision to event pq
            events.push(toAdd);
        }
    }

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
            // Predict the time of collision
            double predicted = predictCollisionTime(event.B, bodies[i]);

            // If value is garbage, continue
            // NOTE: Infinity is denoted by -1 in this case, since time will only be positive
            if (predicted < 0)
            {
                continue;
            }

            // Construct collision event at the calculated time
            Collision toAdd{event.B,
                            bodies[i],
                            event.B->collisions,
                            bodies[i]->collisions,
                            predicted};

            // Add collision to event pq
            events.push(toAdd);
        }
    }

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
double CollisionHandler::predictCollisionTime(const Body *A, const Body *B)
{
    // Much easier equations than I was expecting
    double tx = (B->x - A->x) / (A->vx - B->vx);
    double ty = (B->y - A->y) / (A->vy - B->vy);

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
