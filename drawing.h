#pragma once

///////////////////////////////////////////////////////////////////////////////
//
// Immediate2D - Created by Nicholas Piegdon, released in the public domain
// https://github.com/npiegdon/immediate2d
//
// A drawing framework for Windows that makes simple graphics programming as
// fun and easy as the days when computers booted directly to a BASIC prompt
//
///////////////////////////////////////////////////////////////////////////////


//
// Getting started:
//
//   You'll need a recent Visual Studio with the "Desktop development with C++"
//   workload selected during the install process.
//
//   1. Launch Visual Studio and choose File --> New --> Project.
//   2. Type in whatever project name and choose whichever location you like.
//   3. From the "Visual C++" --> "Windows Desktop" category, choose "Windows
//      Desktop Wizard", then click OK.
//
//   4. Choose "Windows Application (.exe)" from the "Application type" box.
//   5. Add a check mark to the "Empty Project" box!
//   6. Click OK.
//
//   7. Copy the following 3 files from Immediate2D into your project's folder:
//      - drawing.cpp
//      - drawing.h
//      - example1_helloWorld.cpp
//
//   8. Rename your example1_helloWorld.cpp to something like MyProject.cpp
//   9. Drag all three of your new files from the folder to the "Source Files"
//      entry in Visual Studio's "Solution Explorer" panel.
//
//   You should be ready to go!  Click the green play arrow (labeled "Local
//   Windows Debugger") in the tool bar and after a few seconds you should see
//   a dot in the middle of a black window.  Congratulations!
//
//   Double-click your MyProject.cpp file (or whatever you named it) in the
//   "Source Files" list in the Solution Explorer panel and start writing code!
//



// Tinker with these to change the size of the window
//
// (All of the examples assume a Width of 160 and Height of 120)
//
static constexpr int Width = 160;
static constexpr int Height = 120;
static constexpr int PixelScale = 5;



// A useful constant (equal to two pi) to help with trigonometry or circles
static constexpr double Tau = 6.283185307179586476925286766559;



///////////////////////////////////////////////////////////////////////////////
// Color
///////////////////////////////////////////////////////////////////////////////


// This declares a synonym: whenever you see "Color", it means "unsigned int"
using Color = unsigned int;

// Creates a color that can be used with the other drawing functions.
// The r, g, and b parameters are color intensities between 0 and 255.
Color MakeColor(int r, int g, int b);

// Here are some colors (from the old, 16-color EGA palette) to get you
// started.  You can make your own using the same MakeColor function.
static const Color Black =        MakeColor(  0,   0,   0);
static const Color Blue =         MakeColor(  0,   0, 170);
static const Color Green =        MakeColor(  0, 170,   0);
static const Color Cyan =         MakeColor(  0, 170, 170);
static const Color Red =          MakeColor(170,   0,   0);
static const Color Magenta =      MakeColor(170,   0, 170);
static const Color Brown =        MakeColor(170,  85,   0);
static const Color LightGray =    MakeColor(170, 170, 170);
static const Color DarkGray =     MakeColor( 85,  85,  85);
static const Color LightBlue =    MakeColor( 85,  85, 170);
static const Color LightGreen =   MakeColor( 85, 255,  85);
static const Color LightCyan =    MakeColor( 85, 255, 255);
static const Color LightRed =     MakeColor(255,  85,  85);
static const Color LightMagenta = MakeColor(255,  85, 255);
static const Color Yellow =       MakeColor(255, 255,  85);
static const Color White =        MakeColor(255, 255, 255);




///////////////////////////////////////////////////////////////////////////////
// Drawing
///////////////////////////////////////////////////////////////////////////////


// For all of the following drawing functions, x is between 0 and Width;
// y is between 0 and Height.
//
// In computer graphics, y=0 is the top of the screen and it increases
// downward (contrary to the typical mathematics convention).


// Draws a single dot at (x, y) in the given color
void SetPixel(int x, int y, Color c);

// Draws a line from (x1, y1) to (x2, y2) with a given stroke thickness
// (in units of pixels) in the given color
void DrawLine(int x1, int y1, int x2, int y2, int thickness, Color c);

// Draws a circle centered at (x, y) with a given radius (in pixels).  The
// filled parameter controls whether to draw a single pixel outline or to
// completely fill the circle with the passed-in color.
void DrawCircle(int x, int y, int radius, Color c, bool filled);

// Draws a rectangle with upper-left corner at (x, y) with the given width
// and height.  The filled parameter controls whether to draw a single pixel
// outline or to completely fill the rectangle with the passed-in color.
void DrawRectangle(int x, int y, int width, int height, Color c, bool filled);


// Clears the screen to the given color (or Black if no color passed in)
void Clear(Color c = Black);

// Retrieves the color at the given location.  Test against specific colors:
//     if (GetPixel(playerX, playerY) == Black) ...
//
Color GetPixel(int x, int y);




// OPTIONAL!  When anti-aliasing is enabled, all drawing (except single pixels)
// is done with soft edges.  Can be toggled at any time.  Disabled by default.
void UseAntiAliasing(bool enabled);

// OPTIONAL!  When double buffering is enabled, instead of drawing directly to
// the screen, you'll be drawing to a "back buffer".  This lets you compose
// entire scenes or erase and redraw objects without any visible flicker.
//
// Once enabled, you must call the Present function (see below) in order to
// see anything!
//
// Disabled by default
//
void UseDoubleBuffering(bool enabled);

// OPTIONAL!  Only necessary when you've enabled double buffering.  Call when
// you would like to simultaneously show all new drawing performed since your
// previous Present().
//
// Tip: If called at regular intervals, you can achieve an animation effect.
//
// If double buffering is enabled, this MUST be called after you're finished
// drawing in order to see anything!
//
void Present();




///////////////////////////////////////////////////////////////////////////////
// Utility
///////////////////////////////////////////////////////////////////////////////


// When you are ready to end your program, you may call this to automatically
// close the window, too.  If you don't, the window will simply show whatever
// was drawn most recently, forever.
//
// NOTE: If you're NOT ready to end your program, don't call this!  As a side-
//       effect, your main function will be terminated immediately.
//
void CloseWindow();

// Saves your current drawing to a PNG image file on your desktop
//
// Calling SaveImage() will give you a file called "image.png".  If you provide
// a number, say, SaveImage(26), you'll get a file called "image_26.png".
//
void SaveImage(unsigned int suffix = 0);


// Generates a random integer in the interval [low, high)
//
// This is useful for placing things on the screen:
//     int x = RandomInt(0, Width);
//     int y = RandomInt(0, Height);
//
// Or for generating random colors:
//     Color c = MakeColor(RandomInt(0, 256), RandomInt(0, 256), RandomInt(0, 256));
//
int RandomInt(int low, int high);

// Generates a random double in the unit interval [0, 1]
//
// This is useful for lots of things like picking a random angle:
//     double theta = RandomDouble() * Tau;
//
double RandomDouble();


// Delays for the given number of milliseconds.  Useful for animation.
void Wait(int milliseconds);




///////////////////////////////////////////////////////////////////////////////
// Interactivity (Mouse & Keyboard)
///////////////////////////////////////////////////////////////////////////////


// In order to make good use of mouse and keyboard input, you'll want to set up
// your program so that input is checked and drawing is performed repeatedly,
// in a loop.  Game programmers usually call this the "game loop".


// Returns the most recently pressed keyboard character.  For special, non-
// printable characters, test against the values in "Keys", below.
//
// NOTE: To avoid reporting the same key press forever, this function only
//       returns the value ONCE and will subsequently report a value of zero
//       until the next key is pressed.  If you need to use this value more
//       than once, store it in a variable first!
//
char LastKey();

// Used to check the results of LastKey() against non-printable keys:
//
//     // Save the value in a variable before checking it
//     const char key = LastKey();
//     if (key == Enter) ...
//     if (key == Left) ...
//     if (key == Esc) ...
//     if (key == '&') ...
//     if (key == 'r') ...
//     if (key == ' ') ...
//
enum Keys
{
    Left = 0x11,
    Up,
    Right,
    Down,

    Backspace = 8,
    Enter = 13,
    Esc = 27,
    Tab = 9,
};


// Is the mouse button currently being held down?
bool LeftMousePressed();
bool RightMousePressed();
bool MiddleMousePressed();

// Retrieves the current mouse coordinates, or (-1, -1) if the mouse isn't
// currently hovering over our window
int MouseX();
int MouseY();
