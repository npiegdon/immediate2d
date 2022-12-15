
//
// Example A - Snowy Landscape
//
// I used to leave the title screen of the 1993 PC/DOS game "Jetpack Christmas
// Special!" running for an hour to watch the snow pile up on the credits.
//
// Here's the original effect: https://youtu.be/aQGgKvJPuZA
//
// This example demonstrates embedding Base64 encoded images directly in your
// code, which makes the result a completely standalone, single-file app.
//
// Controls:
//   R: resets the snow with a new, blank random scene.
//   T: toggles whether the text should be shown (and also resets).
//   +/-: Adjusts the speed of the snow animation.
//   Esc: Closes the window.

#define IMM2D_IMPLEMENTATION
#include "immediate2d.h"

#include <vector>
using namespace std;

// If your images are small, you can avoid the usual problems of copying the
// files around with your program (or dealing with compiling and linking
// Visual Studio's .RC resource format) by simply embedding the image data
// directly in your code.
//
// Encoding the file as a Base64 string (search for "Base64 Encoder" online),
// we can pass the string directly to Immediate2D's image loading function and
// it will figure things out on its own, automatically.
//
// These PNGs were only a couple hundred bytes, so it makes sense and isn't
// too messy here.  You can shrink PNGs substantially without any loss by using
// tools like pngout, which you can find here: http://advsys.net/ken/utils.htm

static constexpr const char HousePng[] = "iVBORw0KGgoAAAANSUhEUgAAACEAAAAeCAMAAACVFoclAAAAM1BMVEX/"
    "//////+yJwigPADrplWfMBdnNRBoOhf/r1C5fzrz4d/R0dFVMBWinJezs7OVlZXAurXSwoknAAAAAXRSTlMAQObYZgAAA"
    "LdJREFUeF6V0FkOgzAMBFCPnQ3odv/TtpmIglGtqCPxYfHiCcg1kEkAzMCNJDwMlBISVSUgiQRQe24FiEQZIYlETCjYER"
    "ZBNbMiKgJydqJeCFBzRq3nHkeAPhcXR/xLX7RXuLiieAN7SELAHhahxuEW2bZ7C7KkJC8ReUgKsrQmT5HP04J0wRzCmF/"
    "CTFXNdrGP/R4jyWzV1SwN8R3nO5zgoUNw/O8ee4Zw/4MBJl8LDAKcBNAFwRuJgw4oY9H/hQAAAABJRU5ErkJggg==";

static constexpr const char TreePng[] = "iVBORw0KGgoAAAANSUhEUgAAAA4AAAAcBAMAAACjeJNyAAAAElBMVEX//"
    "/8AjQChxaGgPAANfw3///8mLBhYAAAAAXRSTlMAQObYZgAAAGRJREFUeF6NzLENwzAMBVGyUJ8T5AEYZINMoEADSIX3Xy"
    "X2F6tUIUC85uPsumL7PF0pvz7SQxRCOj1F8mTKIb31LSiwCAXgE+ddoQ0kMM2pcQ9XffUah9G6Bg4ZsPf1f1jOafYF+LM"
    "K5nbjm7QAAAAASUVORK5CYII=";

struct Flake
{
    Color c;
    int x, y, speed;

    void Draw() const { DrawPixel(x, y, c); }
};

// We generate snow flake colors in more than one place, so
// this is pulled out as a separate function for consistency.
Color RandomGray()
{
    int albedo = RandomInt(205, 255);
    return MakeColor(albedo, albedo, albedo);
}

void run()
{
    UseDoubleBuffering(true);
    const Image house = LoadImage(HousePng);
    const Image tree = LoadImage(TreePng);

    constexpr int MaxFlakes = 300;

    vector<Flake> flakes;
    flakes.reserve(MaxFlakes);

    bool showText = true;
    int delay = 30;
    while (true)
    {
        flakes.clear();

        // Set up our initial winter scene
        Clear(Black);
        DrawLine(0, Height, Width, Height, 6, RandomGray());
        DrawImage(Width / 2 - RandomInt(0, ImageWidth(house)), Height - ImageHeight(house) - RandomInt(1, 3), house);
        DrawImage(1 * Width / 4 + RandomInt(-30, 0), Height - ImageHeight(tree) - RandomInt(1, 4), tree);
        DrawImage(3 * Width / 4 + RandomInt(-5, 20), Height - ImageHeight(tree) - RandomInt(1, 4), tree);

        if (showText) DrawString(Width / 2, 20, "Happy\nHolidays!", "Arial", 18, LightBlue, true);

        // We use solid white pixels in our images to denote snow (because the
        // image data compresses better with a single color) but the demo looks
        // better if those solid white pixels are randomized to the same gray
        // scale as the rest of our snow flakes.
        for (int y = 0; y < Height - 5; ++y)
            for (int x = 0; x < Width; ++x)
                if (ReadPixel(x, y) == White) DrawPixel(x, y, RandomGray());

        bool running = true;
        while (running)
        {
            Present();

            switch (LastKey())
            {
            case '+': delay += 25; break;
            case '-': delay -= 25; break;

            case 't':
            case 'T':
                showText = !showText;
                [[fallthrough]];

            case 'r':
            case 'R':
                running = false;
                break;

            case Esc:
                CloseWindow();
                return;
            }

            delay = clamp(delay, 5, 200);
            Wait(delay);

            constexpr int CutoffY = 15;

            // Reset if the snow has built up to the top of the screen.
            int litPixels = 0;
            for (int i = 0; i < Width; ++i)
                if (ReadPixel(i, CutoffY) == Black) break;
                else ++litPixels;

            if (litPixels == Width) break;

            // Spawn new flakes randomly, with more chances to
            // spawn, the fewer active flakes we currently have.
            for (int i = 0, chances = int(MaxFlakes - flakes.size()); i < chances; ++i)
            {
                constexpr double SpawnChance = 0.01;
                if (RandomDouble() > SpawnChance) continue;

                int x = RandomInt(0, Width);
                int speed = RandomInt(1, 4);
                flakes.push_back(Flake{ RandomGray(), x, 0, speed });
            }

            for (int i = 0; i < flakes.size(); ++i)
            {
                auto &f = flakes[i];
                DrawPixel(f.x, f.y, Black);

                // Step it down one tick for each "speed".
                bool landed = false;
                for (int s = 0; s < f.speed; ++s)
                {
                    bool left =  ReadPixel(f.x - 1, f.y + 1) == Black && f.x > 0;
                    bool down =  ReadPixel(f.x,     f.y + 1) == Black;
                    bool right = ReadPixel(f.x + 1, f.y + 1) == Black && f.x < Width - 1;

                    // If there's anywhere to go, we always move down a pixel.
                    if (down || left || right) ++f.y;
                    else
                    {
                        landed = true;
                        break;
                    }

                    // If we can't move straight down, we have to decide
                    // which direction to roll (using a simple coin toss).
                    if (!down)
                    {
                        if (left && right) f.x += RandomBool() ? 1 : -1;
                        else if (left) --f.x;
                        else if (right) ++f.x;
                    }
                }
                f.Draw();

                if (landed)
                {
                    // We need to remove this flake from the list.  But, to avoid
                    // shimmying all the elements in the list after this one up one
                    // to fill in the hole, we just grab the flake at the end of the
                    // list to fill it, then shrink the list by one.  (This works
                    // because our flakes don't have any sort of ordering or inter-
                    // dependence constraints.)
                    //
                    // The i-- steps our loop back one so we don't forget to process
                    // the flake that just took this one's place.
                    flakes[i--] = flakes.back();
                    flakes.pop_back();
                }
            }

        }
    }
}
