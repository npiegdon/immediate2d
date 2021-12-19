#pragma once

///////////////////////////////////////////////////////////////////////////////
//
// Immediate2D - Created by Nicholas Piegdon, released in the public domain
// https://github.com/npiegdon/immediate2d
//
// A drawing framework for Windows that makes simple graphics programming as
// much fun as the days when computers booted directly to a BASIC prompt.
//
///////////////////////////////////////////////////////////////////////////////


//
// Getting started:
//
//   You'll need a recent Visual Studio with the "Desktop development with C++"
//   workload selected during the install process.
//
//   1. Launch Visual Studio and choose "Create a new project".
//   2. For the project template, choose "Windows Desktop Wizard" from the list.
//   3. Name the project whatever you like and save it wherever you like.
//
//   4. Choose "Desktop Application (.exe)" from the "Application type" box.
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



// Tinker with these to change the size of the window.
//
// (All of the examples assume a Width of 160 and Height of 120.)
//
static constexpr int Width = 160;
static constexpr int Height = 120;
static constexpr int PixelScale = 5;



// A useful constant (2*pi) to help with trigonometry or circles.
static constexpr double Tau = 6.283185307179586476925286766559;



///////////////////////////////////////////////////////////////////////////////
// Color
///////////////////////////////////////////////////////////////////////////////


// This makes "Color" a synonym for "unsigned int" (a positive number).
using Color = unsigned int;

// MakeColor returns a Color that can be used with the other drawing functions.
// The red, green, and blue parameters are color intensities between 0 and 255.
constexpr Color MakeColor(int red, int green, int blue);

// Here are some colors to get you started.
static const Color Transparent =  0U;
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
// In computer graphics, y starts at the top of the screen and goes downward
// as it increases. (This is upside-down from the way it's usually done in math
// classes and can be a little tricky to get used to!)


// Draws a single dot at (x, y) in the given color.
void DrawPixel(int x, int y, Color c);

// Draws a line from (x1, y1) to (x2, y2) with a given stroke thickness
// (in pixels) in the given color.
void DrawLine(int x1, int y1, int x2, int y2, int thickness, Color c);

// Draws a circle centered at (x, y) with a given radius (in pixels).  The
// filled parameter controls whether to draw a single pixel outline or to
// completely fill the circle with the passed-in color.
void DrawCircle(int x, int y, int radius, Color c, bool filled);

// Draws a rectangle with upper-left corner at (x, y) with the given width
// and height.  The filled parameter controls whether to draw a single pixel
// outline or to completely fill the rectangle with the passed-in color.
void DrawRectangle(int x, int y, int width, int height, Color c, bool filled);


// Clears the screen to the given color (or Black if no color passed in).
void Clear(Color c = Black);

// Retrieves the color at the given location.  Test against specific colors:
//     if (ReadPixel(playerX, playerY) == Black) ...
//
Color ReadPixel(int x, int y);




// OPTIONAL!  Anti-aliasing is a graphics technique to make your lines and
// circles appear with smooth/soft edges.
void UseAntiAliasing(bool enabled);

// OPTIONAL!  Things are normally drawn to the screen just as your drawing
// code is run.  This works fine until you have very intricate scenes that
// require many drawing commands or when you want to animate something.  At
// that point you may not want to see the intermediate drawing steps anymore.
//
// When you enable "double buffering", your drawing will take place on a
// different, hidden surface called the "back-buffer".  You can draw as much
// as you like to the hidden back-buffer without any visible changes to the
// screen.  Then, after you've composed your entire scene or updated your
// animation, you can "present" the scene all at once.
//
void UseDoubleBuffering(bool enabled);

// OPTIONAL!  This isn't normally needed.  But, if you've enabled double
// buffering, you MUST Present() whenever you would like to show the results
// of all your drawing code since the last time you called Present().
//
// Tip: If called at regular intervals, you can achieve an animation effect.
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
//       effect, your run() function will be terminated immediately.
//
void CloseWindow();

// Saves your current drawing to a PNG image file on your desktop.
//
// Calling SaveImage() will give you a file called "image.png".  If you include
// a number like SaveImage(26) then you'll get a file called "image_26.png".
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

// Generates a random "real" number between zero and one.
//
// This is useful whenever you need something that isn't a whole number.  You
// could pick a random angle (in radians) around a circle like this:
//     double theta = RandomDouble() * Tau;
//
// Or a random angle (in degrees):
//     double theta = RandomDouble() * 360.0;
//
double RandomDouble();


// Delays your code for the given number of milliseconds.  By drawing, then
// waiting, then drawing something else, you can make things animate.
//
// There are 1000 milliseconds in one second.
//
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
    Left = 17,
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

// Retrieves the current mouse position (relative to the top-left corner of
// the drawing surface), or -1 if the mouse cursor isn't inside our window.
int MouseX();
int MouseY();
