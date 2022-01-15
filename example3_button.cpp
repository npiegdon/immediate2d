
#define IMM2D_IMPLEMENTATION
#include "immediate2d.h"

//
// Example 3 - Button
//
// Draws two buttons.  Clicking either will fill most of the screen with that button's color.
//
// Exercises:
// 1. Create a Button struct that contains a position and color
// 2. Migrate "blueButtonX", etc. over to use the new struct: "Button button1 = { 10, 90, Blue };"
// 3. Consolidate the repeated drawing code (before the loop) into a separate "void DrawButton(Button b)" function
// 4. Consolidate the repeated hit-testing code (inside the loop) into a separate "bool TestButton(Button b, int x, int y)" function
// 5. Add another button using your new functions
//

const int ButtonSize = 20;

void run()
{
    int blueButtonX = 10;
    int blueButtonY = 90;

    int greenButtonX = 40;
    int greenButtonY = 90;

    // Draw our buttons with a nice bevel effect
    DrawRectangle(blueButtonX,     blueButtonY,     ButtonSize,     ButtonSize,     DarkGray);
    DrawRectangle(blueButtonX,     blueButtonY,     ButtonSize - 1, ButtonSize - 1, White);
    DrawRectangle(blueButtonX + 1, blueButtonY + 1, ButtonSize - 2, ButtonSize - 2, Blue);

    DrawRectangle(greenButtonX,     greenButtonY,     ButtonSize,     ButtonSize,     DarkGray);
    DrawRectangle(greenButtonX,     greenButtonY,     ButtonSize - 1, ButtonSize - 1, White);
    DrawRectangle(greenButtonX + 1, greenButtonY + 1, ButtonSize - 2, ButtonSize - 2, Green);

    // Loop forever
    while (true)
    {
        // Adding a short wait between "frames" is a good idea so the CPU doesn't max out at 100%
        Wait(1);

        // Don't do anything unless the mouse button is pressed
        if (!LeftMousePressed()) continue;

        int x = MouseX();
        int y = MouseY();

        // Check if we're clicking the blue button
        if (x >= blueButtonX && x < blueButtonX + ButtonSize
         && y >= blueButtonY && y < blueButtonY + ButtonSize)
        {
            DrawRectangle(0, 0, Width, 80, Blue);
        }

        // Check if we're clicking the green button
        if (x >= greenButtonX && x < greenButtonX + ButtonSize
         && y >= greenButtonY && y < greenButtonY + ButtonSize)
        {
            DrawRectangle(0, 0, Width, 80, Green);
        }
    }
}
