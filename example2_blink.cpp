#include "drawing.h"

//
// Example 2 - Blink
//
// Animates a blinking circle in the middle of the screen.
//
// Exercises:
// 1. Tinker with the four parameters to achieve different effects
// 2. Draw a rectangle instead of a circle
//

void run()
{
    int x = 80;
    int y = 60;
    int radius = 20;
    int delay = 1000;

    // Draw shapes with smooth edges
    UseAntiAliasing();

    // Loop forever
    while (true)
    {
        DrawCircle(x, y, radius, Yellow, Brown);
        Wait(delay);

        Clear();
        Wait(delay);
    }
}
