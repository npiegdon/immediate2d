#include "drawing.h"

#include <vector>
#include <algorithm>
using namespace std;

//
// Example 8 - Smoke
//
// It is sometimes a bit miraculous what you're able to accomplish in only a
// couple hundred lines, without any dependencies on monolithic libraries.
//
// NOTE: To run anywhere near real-time speed, be sure to change the drop-down
//       at the top of the Visual Studio window from "Debug" to "Release"!
//                                                               ^^^^^^^
// Usage:                                                        ^^^^^^^
// - Right-mouse-drag to add smoke
// - Left-mouse-drag to induce an air current
// - (Holding both buttons at once is fun!)
//
// - Space toggles between viewing the smoke density vs. air current velocity
// - 's' will save the current image
// - 'c' will reset the simulation
// - Esc will quit
//
// Exercises:
// 1. Have fun playing with the simulation!
//


//
// This simulation was adapted (rather heavily) from Joe Stam's 2003 paper
// "Real-Time Fluid Dynamics for Games" and its included source code, which
// can be found at his website:
//
// http://www.dgp.toronto.edu/~stam/reality/Research/pub.html
//
// Most of the adaptation was a migration from the rather idiomatic use of C
// coding style and away from GLUT to the Immediate2D framework.  The rest
// was changing from a square vector field with sides N, to a rectangle field
// the same size as our window.  (And then a little extra to make things show
// up nice and colorfully.)
//


// The +2 in each dimension is for the boundary walls that are just off-screen
static constexpr int Size = (Width + 2)*(Height + 2);
static constexpr size_t id(int i, int j) { return (Width + 2)*j + i; }

void setBoundary(int b, vector<float> &x)
{
    for (int j = 1; j <= Height; j++)
    {
        x[id(0,         j)] = b == 1 ? -x[id(1,     j)] : x[id(1,     j)];
        x[id(Width + 1, j)] = b == 1 ? -x[id(Width, j)] : x[id(Width, j)];
    }

    for (int i = 1; i <= Width; i++)
    {
        x[id(i,          0)] = b == 2 ? -x[id(i,      1)] : x[id(i,      1)];
        x[id(i, Height + 1)] = b == 2 ? -x[id(i, Height)] : x[id(i, Height)];
    }

    x[id(0,                  0)] = 0.5f*(x[id(1,              0)] + x[id(0,              1)]);
    x[id(0,         Height + 1)] = 0.5f*(x[id(1,     Height + 1)] + x[id(0,         Height)]);
    x[id(Width + 1,          0)] = 0.5f*(x[id(Width,          0)] + x[id(Width + 1,      1)]);
    x[id(Width + 1, Height + 1)] = 0.5f*(x[id(Width, Height + 1)] + x[id(Width + 1, Height)]);
}

void linearSolve(int b, vector<float> &x, vector<float> &x0, float a, float c)
{
    for (int k = 0; k < 20; k++)
    {
        for (int j = 1; j <= Height; j++)
            for (int i = 1; i <= Width; i++)
                x[id(i, j)] = (x0[id(i, j)] + a*(x[id(i-1, j)] + x[id(i+1, j)] + x[id(i, j-1)] + x[id(i, j+1)])) / c;

        setBoundary(b, x);
    }
}

void diffuse(int b, vector<float> &x, vector<float> &x0, float diffusion, float dt)
{
    const float a = dt * diffusion * Width * Height;
    linearSolve(b, x, x0, a, 1 + 4 * a);
}

void advect(int b, vector<float> &d, vector<float> &d0, vector<float> &u, vector<float> &v, float dt)
{
    const float dt0 = dt*Height;
    for (int j = 1; j <= Height; j++)
    {
        for (int i = 1; i <= Width; i++)
        {
            float x = i - dt0*u[id(i, j)];
            float y = j - dt0*v[id(i, j)];

            if (x < 0.5f) x = 0.5f;
            if (x > Width + 0.5f) x = Width + 0.5f;
            int i0 = (int)x, i1 = i0 + 1;

            if (y < 0.5f) y = 0.5f;
            if (y > Height + 0.5f) y = Height + 0.5f;
            int j0 = (int)y, j1 = j0 + 1;

            float s1 = x - i0, s0 = 1 - s1;
            float t1 = y - j0, t0 = 1 - t1;
            d[id(i, j)] = s0*(t0*d0[id(i0, j0)] + t1*d0[id(i0, j1)]) + s1*(t0*d0[id(i1, j0)] + t1*d0[id(i1, j1)]);
        }
    }

    setBoundary(b, d);
}

void project(vector<float> &u, vector<float> &v, vector<float> &p, vector<float> &div)
{
    for (int j = 1; j <= Height; j++)
    {
        for (int i = 1; i <= Width; i++)
        {
            div[id(i, j)] = -0.5f*(u[id(i+1, j)] - u[id(i-1, j)] + v[id(i, j+1)] - v[id(i, j-1)]) / Height;
            p[id(i, j)] = 0;
        }
    }

    setBoundary(0, div);
    setBoundary(0, p);
    linearSolve(0, p, div, 1, 4);
    for (int j = 1; j <= Height; j++)
    {
        for (int i = 1; i <= Width; i++)
        {
            u[id(i, j)] -= 0.5f*Height*(p[id(i + 1, j)] - p[id(i - 1, j)]);
            v[id(i, j)] -= 0.5f*Height*(p[id(i, j + 1)] - p[id(i, j - 1)]);
        }
    }

    setBoundary(1, u);
    setBoundary(2, v);
}

void densityStep(vector<float> &x, vector<float> &x0, vector<float> &u, vector<float> &v, float diffusion, float dt)
{
    for (int i = 0; i < Size; i++) x[i] += dt * x0[i];
    diffuse(0, x0, x, diffusion, dt);
    advect(0, x, x0, u, v, dt);
}

void velocityStep(vector<float> &u, vector<float> &v, vector<float> &u0, vector<float> &v0, float viscosity, float dt)
{
    for (int i = 0; i < Size; i++) u[i] += dt * u0[i];
    for (int i = 0; i < Size; i++) v[i] += dt * v0[i];
    diffuse(1, u0, u, viscosity, dt);
    diffuse(2, v0, v, viscosity, dt);
    project(u0, v0, u, v);
    advect(1, u, u0, u0, v0, dt);
    advect(2, v, v0, u0, v0, dt);
    project(u, v, u0, v0);
}




// We use a secret function from drawing.cpp that lets us update every pixel at once
void Present(const vector<Color> &screen);

// Generate a color based on our preferred visualization
Color FluidColor(float u, float v, float density, bool showVelocity)
{
    // There's no real basis for these numbers; they were
    // simply hand-tweaked until things looked nice

    if (showVelocity)
    {
        int h = min(360, max(0, int(sqrt(u*u + v*v) * 1500.0)));
        int v = min(255, max(0, int(density * 500.0)));
        return MakeColorHSB(h, 255, v);
    }

    const int value = min(360, max(0, int(density * 100.0f)));
    return MakeColorHSB(max(0, value - 310), value / 2, value);
}

// Returns a list of points between the given coordinates
vector<pair<int, int>> Line(int x1, int y1, int x2, int y2)
{
    vector<pair<int, int>> result;
    const int dx = x2 - x1;
    const int dy = y2 - y1;
    if (dy == 0) { for (int x = min(x1, x2); x <= max(x1, x2); ++x) result.emplace_back(x, y1); return result; }
    if (dx == 0) { for (int y = min(y1, y2); y <= max(y1, y2); ++y) result.emplace_back(x1, y); return result; }

    if (abs(dx) > abs(dy)) // Is the line X or Y-major?
    {
        double slope = 1.0 * dy / abs(dx), y = y1;
        for (int i = 0, x = x1; i <= abs(dx); ++i, x += x1 > x2 ? -1 : 1, y += slope) result.emplace_back(x, int(y));
        return result;
    }

    double slope = 1.0 * dx / abs(dy), x = x1;
    for (int j = 0, y = y1; j <= abs(dy); ++j, y += y1 > y2 ? -1 : 1, x += slope) result.emplace_back(int(x), y);
    return result;
}

void MouseDrag(vector<float> &field, int x1, int y1, int x2, int y2, float value)
{
    // The mouse pointer usually moves faster than one pixel per frame, so instead of dotting up our
    // velocity and density vector fields with single points, it makes the interaction more fun if
    // we scratch out an entire line segment between the previous and current mouse coordinates
    const auto points = Line(x1, y1, x2, y2);
    for (const auto &p : points) field[id(p.first, p.second)] = value / points.size();
}

// For a line-by-line breakdown of this single-function text rendering library,
// see the Text example.  (This version has been compacted a bit and made a
// little more generic so we can draw directly into the density field in this smoke example.)
void DrawString(vector<float> &density, int y, const char *s)
{
    static constexpr uint32_t Font[128 - 32] = {
        0x10000000, 0x10000017, 0x30000C03, 0x50AFABEA, 0x509AFEB2, 0x30004C99, 0x400A26AA, 0x10000003, 0x2000022E, 0x200001D1, 0x30001445, 0x300011C4, 0x10000018, 0x30001084, 0x10000010, 0x30000C98,
        0x30003A2E, 0x300043F2, 0x30004AB9, 0x30006EB1, 0x30007C87, 0x300026B7, 0x300076BF, 0x30007C21, 0x30006EBB, 0x30007EB7, 0x1000000A, 0x1000001A, 0x30004544, 0x4005294A, 0x30001151, 0x30000AA1,
        0x506ADE2E, 0x300078BE, 0x30002ABF, 0x3000462E, 0x30003A3F, 0x300046BF, 0x300004BF, 0x3000662E, 0x30007C9F, 0x1000001F, 0x30003E08, 0x30006C9F, 0x3000421F, 0x51F1105F, 0x51F4105F, 0x4007462E,
        0x300008BF, 0x400F662E, 0x300068BF, 0x300026B2, 0x300007E1, 0x30007E1F, 0x30003E0F, 0x50F8320F, 0x30006C9B, 0x30000F83, 0x30004EB9, 0x2000023F, 0x30006083, 0x200003F1, 0x30000822, 0x30004210,
        0x20000041, 0x300078BE, 0x30002ABF, 0x3000462E, 0x30003A3F, 0x300046BF, 0x300004BF, 0x3000662E, 0x30007C9F, 0x1000001F, 0x30003E08, 0x30006C9F, 0x3000421F, 0x51F1105F, 0x51F4105F, 0x4007462E,
        0x300008BF, 0x400F662E, 0x300068BF, 0x300026B2, 0x300007E1, 0x30007E1F, 0x30003E0F, 0x50F8320F, 0x30006C9B, 0x30000F83, 0x30004EB9, 0x30004764, 0x1000001F, 0x30001371, 0x50441044, 0x00000000,
    };

    // Center the line
    int textWidth = 0;
    for (const char *i = s; *i; ++i) textWidth += (*i < 32 ? 0 : (Font[*i - 32] >> 28) + 1);
    int x = (Width - textWidth) / 2;

    for (const char *i = s; *i; ++i)
    {
        if (*i < 32 || *i > 127) continue;
        uint32_t glyph = Font[*i - 32];
        const int width = glyph >> 28;
        for (int u = x; u < x + width; ++u) for (int v = y; v < y + 5; ++v, glyph = glyph >> 1) if ((glyph & 1) == 1) density[id(u, v)] = 3.0f;
        if (width > 0) x += width + 1;
    }
}

void run()
{
    const float dt = 0.1f;
    const float diffusion = 0.0f;
    const float viscosity = 0.0f;
    vector<float> u(Size), v(Size), uPrev(Size), vPrev(Size), density(Size), densityPrev(Size);
    vector<Color> screen(Width * Height);

    DrawString(density, 1 * Height / 7, "Left mouse drag to move air");
    DrawString(density, 2 * Height / 7, "Right mouse drag to add smoke");
    DrawString(density, 3 * Height / 7, "Holding both is the most fun!");
    DrawString(density, 5 * Height / 7, "Use spacebar to toggle velocity view");
    DrawString(density, 6 * Height / 7, "(Press 'C' to clear the screen)");

    bool showVelocity = false;
    bool mouseWasDown = false;
    int downX = -1, downY = -1;

    while (true)
    {
        // Adding a short wait between "frames" is a good idea so the CPU doesn't max out at 100%
        Wait(1);

        for (int i = 0; i < Size; i++) uPrev[i] = vPrev[i] = densityPrev[i] = 0.0f;

        const char key = LastKey();
        if (key == 'c') for (int i = 0; i < Size; i++) u[i] = v[i] = uPrev[i] = vPrev[i] = density[i] = densityPrev[i] = 0.0f;
        if (key == ' ') showVelocity = !showVelocity;
        if (key == Esc) CloseWindow();

        const int mX = MouseX() + 1;
        const int mY = MouseY() + 1;

        // Remember where the mouse was when a button was first clicked
        const bool mouseIsDown = LeftMousePressed() || RightMousePressed();
        if (mouseIsDown && !mouseWasDown) { downX = mX; downY = mY; }
        mouseWasDown = mouseIsDown;

        if (mouseIsDown && mX >= 1 && mX <= Width && mY >= 1 && mY <= Height)
        {
            if (LeftMousePressed())
            {
                MouseDrag(uPrev, downX, downY, mX, mY, 10.0f * (mX - downX));
                MouseDrag(vPrev, downX, downY, mX, mY, 10.0f * (mY - downY));
            }

            if (RightMousePressed()) MouseDrag(densityPrev, downX, downY, mX, mY, 100.0f);

            downX = mX;
            downY = mY;
        }

        velocityStep(u, v, uPrev, vPrev, viscosity, dt);
        densityStep(density, densityPrev, u, v, diffusion, dt);

        for (int j = 0; j < Height; j++)
            for (int i = 0; i < Width; i++)
                screen[j*Width + i] = FluidColor(u[id(i+1, j+1)], v[id(i+1, j+1)], density[id(i+1, j+1)], showVelocity);

        Present(screen);
    }
}
