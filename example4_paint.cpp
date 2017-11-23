#include "drawing.h"

//
// Example 4 - Paint
//
// A simple drawing program.  Click a color in the palette
// to select it.  Left-click drag in the canvas area to draw.
//
// Exercises:
// 1. Add an 's' shortcut key that saves the image to the deskop (using the SaveImage() function)
// 2. Allow a right-click drag to "erase" (by always drawing in Black)
// 3. Extend your right-click interaction to allow for two currently-selected colors: a "foreground"
//    and "background" color, instead of right-click always being black
// 4. Ensure the spacebar shortcut also uses the current background color when it clears the canvas
//

void main()
{
    // Add the built-in colors to a list so we can retrieve them by array index
    Color colors[16] = { Black, Blue, Green, Cyan, Red, Magenta, Brown, LightGray, DarkGray, LightBlue, LightGreen, LightCyan, LightRed, LightMagenta, Yellow, White };

    const int PaletteEntryWidth = Width / 8;
    const int PaletteEntryHeight = 6;

    // The canvas begins at three "rows" of palette down: the palette really only contains two rows,
    // but the third is used to show the currently selected color.  The extra +2 is to leave a space
    // between the palette area and the canvas area.
    const int CanvasY = PaletteEntryHeight * 3 + 2;

    // Draw the palette at the top of the screen
    for (int i = 0; i < 16; i++)
    {
        // Halfway through our loop, wrap x back to the first column (using the "remainder" or "modulus" operator)
        int x = PaletteEntryWidth * (i % 8);
        int y = 0;

        // Halfway through our loop, start a second row of colors
        if (i >= 8) y += PaletteEntryHeight;

        DrawRectangle(x, y, PaletteEntryWidth, PaletteEntryHeight, colors[i], true);
    }

    // This holds the index to the currently selected color (from the "colors" list).
    // The default is White, which is the 15th (zero-based) entry in the list.
    int currentColor = 15;

    // These hold where we last saw the mouse on the canvas, or (-1, -1) if the last click was not on the canvas
    int previousX = -1;
    int previousY = -1;

    while (true)
    {
        // Adding a short wait between "frames" is a good idea so the CPU doesn't max out at 100%
        Wait(1);

        const char key = LastKey();

        // The spacebar clears the canvas
        if (key == ' ') DrawRectangle(0, CanvasY, Width, Height, Black, true);

        // Grab our current mouse coordinates
        const int x = MouseX();
        const int y = MouseY();

        const bool mouseOffscreen = x < 0 || y < 0;
        const bool mouseOnCanvas = !mouseOffscreen && y > CanvasY;

        // Unless the mouse is currently on the canvas with the button held, force a new line segment
        if (!mouseOnCanvas || !LeftMousePressed())
        {
            previousX = -1;
            previousY = -1;
        }

        // Wait until a mouse button is pressed before proceeding
        if (!LeftMousePressed()) continue;

        // Is this click in the palette area?
        if (y < PaletteEntryHeight * 2)
        {
            // Determine which color was clicked
            int row = 0;
            if (y > PaletteEntryHeight) row = 1;

            int col = x * 8 / Width;

            // Calculate the new index into our "colors" table
            currentColor = row * 8 + col;

            // Show the new color on the screen
            DrawRectangle(0, PaletteEntryHeight * 2 + 1, Width, PaletteEntryHeight, colors[currentColor], true);
        }

        // The rest of the loop is for drawing on the canvas, which we only
        // do if we're currently hovering over the canvas area of the screen
        if (!mouseOnCanvas) continue;

        // If the mouse hasn't moved yet, keep the coordinates
        // for later and just draw a single pixel for now
        if (previousX == -1 && previousY == -1)
        {
            SetPixel(x, y, colors[currentColor]);
            previousX = x;
            previousY = y;
            continue;
        }

        // Draw the line!
        DrawLine(previousX, previousY, x, y, 1, colors[currentColor]);

        // Keep track of our new end-point for next time
        previousX = x;
        previousY = y;
    }
}
