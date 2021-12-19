#include "drawing.h"
#include <string>

//
// Example 6 - Text
//
// A font rendering library in two functions
//
// Exercises:
// 1. Technically a char can contain negative values, so we need array bounds checking before each "Font[c]" lookup
// 2. Create a new DrawCenteredString(...) function that draws a string centered horizontally on the passed-in x coordinate
// 3. Center the title text using your new function
// 4. Test for the Tab key and advance 15 pixels at once
// 5. Test for the Enter key and start a new line
// 6. Once you Enter past the bottom of the screen, clear the screen and start back at the top
//
// Extra credit challenge: Test for Backspace and make it behave the way you'd expect!
//

// This block of numbers encodes a monochrome, 5-pixel-tall font for the first 127 ASCII characters!
//
// Bits are shifted out one at a time as each row is drawn (top to bottom).  Because each glyph fits
// inside an at-most 5x5 box, we can store all 5x5 = 25-bits fit inside a 32-bit unsigned int with
// room to spare.  That extra space is used to store that glyph's width in the most significant nibble.
//
// The first 32 entries are unprintable characters, so each is totally blank with a width of 0
//
static const unsigned int Font[128] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0x10000000, 0x10000017, 0x30000C03, 0x50AFABEA, 0x509AFEB2, 0x30004C99, 0x400A26AA, 0x10000003, 0x2000022E, 0x200001D1, 0x30001445, 0x300011C4, 0x10000018, 0x30001084, 0x10000010, 0x30000C98,
   0x30003A2E, 0x300043F2, 0x30004AB9, 0x30006EB1, 0x30007C87, 0x300026B7, 0x300076BF, 0x30007C21, 0x30006EBB, 0x30007EB7, 0x1000000A, 0x1000001A, 0x30004544, 0x4005294A, 0x30001151, 0x30000AA1,
   0x506ADE2E, 0x300078BE, 0x30002ABF, 0x3000462E, 0x30003A3F, 0x300046BF, 0x300004BF, 0x3000662E, 0x30007C9F, 0x1000001F, 0x30003E08, 0x30006C9F, 0x3000421F, 0x51F1105F, 0x51F4105F, 0x4007462E,
   0x300008BF, 0x400F662E, 0x300068BF, 0x300026B2, 0x300007E1, 0x30007E1F, 0x30003E0F, 0x50F8320F, 0x30006C9B, 0x30000F83, 0x30004EB9, 0x2000023F, 0x30006083, 0x200003F1, 0x30000822, 0x30004210,
   0x20000041, 0x300078BE, 0x30002ABF, 0x3000462E, 0x30003A3F, 0x300046BF, 0x300004BF, 0x3000662E, 0x30007C9F, 0x1000001F, 0x30003E08, 0x30006C9F, 0x3000421F, 0x51F1105F, 0x51F4105F, 0x4007462E,
   0x300008BF, 0x400F662E, 0x300068BF, 0x300026B2, 0x300007E1, 0x30007E1F, 0x30003E0F, 0x50F8320F, 0x30006C9B, 0x30000F83, 0x30004EB9, 0x30004764, 0x1000001F, 0x30001371, 0x50441044, 0x00000000,
};

// Returns the width (in pixels) that the given string will require when drawn
int MeasureString(const std::string &s)
{
    // We're going to sum the width of each character in the string
    int result = 0;

    for (char c : s)
    {
        // Grab the glyph for that character from our font
        unsigned int glyph = Font[c];

        // Retrieve the width (stored in the top nibble)
        int width = glyph >> 28;

        // Skip invisible characters
        if (width == 0) continue;

        // The +1 is for the space between letters
        result += width + 1;
    }

    // Trim the trailing space that we added on the last letter
    // (as long as there was at least one printable character)
    if (result > 0) result -= 1;

    return result;
}

// Draws a single character to the screen
//
// Returns the width of the printed character in pixels
//
int DrawCharacter(int left, int top, char c, Color color)
{
    unsigned int glyph = Font[c];
    int width = glyph >> 28;

    // Loop over the bounding box of the glyph
    for (int x = left; x < left + width; x++)
    {
        for (int y = top; y < top + 5; y++)
        {
            // If the current (LSB) bit is 1, we draw this pixel
            if ((glyph & 1) == 1) DrawPixel(x, y, color);

            // Shift out the next pixel from our packed glyph
            glyph = glyph >> 1;
        }
    }

    return width;
}

void DrawString(int x, int y, const std::string &s, const Color color)
{
    for (char c : s)
    {
        // The +1 is for the space between letters
        x += DrawCharacter(x, y, c, color) + 1;
    }
}

void run()
{
    DrawString(1, 1, "Simple Text Editor", LightGray);

    int cursorX = 1;
    int cursorY = 7;

    while (true)
    {
        // Adding a short wait between "frames" is a good idea so the CPU doesn't max out at 100%
        Wait(1);

        // Only proceed if the user pressed a key
        char key = LastKey();
        if (key == 0) continue;

        // Erase the old cursor
        DrawRectangle(cursorX, cursorY, 3, 5, Black);

        // Draw the character for the pressed key (keeping track of how wide it was)
        int width = DrawCharacter(cursorX, cursorY, key, LightGreen);

        // As long as it was a printable character, advance the cursor
        if (width > 0) cursorX += width + 1;

        // Redraw the cursor in the (potentially) new location
        DrawRectangle(cursorX, cursorY, 3, 5, Green);
    }
}
