#include "drawing.h"

//
// Example 5 - Graphing
//
// A simple, single-variable graphing calculator.
//
// Exercises:
// 1. Instead of a single pixel, use line segments between adjacent points (so "tan(x)" ends up a solid curve)
// 2. Pull the input & output transforms into separate functions
// 3. Add a second math function "g(x)" and plot it on the same set of axes (using your new transform functions)
// 4. Parameterize your transforms so you can adjust the window/zoom of your graph
//

// We're going to need some interesting math functions for this one!
#include <cmath>

double f(double x)
{
    return sin(x);
}

void main()
{
    // Draw our axes
    DrawLine(0, Height / 2, Width, Height / 2, 1, DarkGray);
    DrawLine(Width / 2, 0, Width / 2, Height, 1, DarkGray);

    for (int p = 0; p < Width; ++p)
    {
        // We'll need to transform from our pixel coordinate system to the usual Cartesian coordinates
        double x = p;

        // First, shift x=0 to the center of the screen (instead of the left edge)
        x -= Width / 2;

        // Scale x down, effectively setting our graph's "Window" to something like w=(-8, 8), h=(-6, 6)
        x /= 10.0;

        // Run the function!
        double y = f(x);

        // Now we have to scale our result back up
        y *= 10;

        // In computer graphics, y increases downward
        // In Cartesian coordinates, y increases upward
        y = -y;

        // Finally, shift it to the center of the screen (instead of the top edge)
        y += Height / 2;

        SetPixel(p, (int)y, LightRed);
    }
}
