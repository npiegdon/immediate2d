#pragma once

///////////////////////////////////////////////////////////////////////////////
//
// Immediate2D - Created by Nicholas Piegdon
// https://github.com/npiegdon/immediate2d
//
// A drawing framework for Windows that makes simple graphics programming as
// much fun as the days when computers booted directly to a BASIC prompt.
//
// This is free and unencumbered software released into the public domain.
// See the full notice at the end of this file.
// 
// CHANGELOG (see CHANGES.md for more detail)
// 2023-01, v2: API cleanup, single file, image loading/drawing, new examples
// 2017-11, v1: Initial release
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
//   7. Copy the following files from Immediate2D into your project's folder:
//      - immediate2d.h
//      - example1_helloWorld.cpp
//
//   8. Rename your example1_helloWorld.cpp to something like MyProject.cpp
//   9. Drag both of your new files from the folder to the "Source Files"
//      entry in Visual Studio's "Solution Explorer" panel.
//
//   You should be ready to go!  Click the green play arrow (labeled "Local
//   Windows Debugger") in the tool bar and after a few seconds you should see
//   a dot in the middle of a black window.  Congratulations!
//
//   Double-click your MyProject.cpp file (or whatever you named it) in the
//   "Source Files" list in the Solution Explorer panel and start writing code!
//
// Getting started without a template:
//
//   If you don't want to start from the example file, all you need to do is
//   add these two lines to the top of exactly one .cpp file in your project:
//
//   #define IMM2D_IMPLEMENTATION
//   #include "immediate2d.h"
//
//   void run()
//   {
//       DrawPixel(10, 10, White);
//   }
//


// By default, Immediate2D's virtual window is 160 pixels wide and 120 pixels
// tall, where each virtual pixel is scaled by 5x before being drawn.
//
// You can check these values at runtime by using the Width, Height, and
// PixelScale global variables.
//
// To change any of those, use additional #define lines, in the same file
// where you plan to #define IMM2D_IMPLEMENTATION, like this example:
//
// #define IMM2D_WIDTH 640
// #define IMM2D_HEIGHT 480
// #define IMM2D_SCALE 2
//
// #define IMM2D_IMPLEMENTATION
// #include "immediate2d.h"
//
// That would give you a window that was 640x480 where each virtual pixel is
// scaled 2x before being drawn to the screen.  All three can be set
// independently.  For example, you'd only need one line if you only wanted
// to change the scale.
//



///////////////////////////////////////////////////////////////////////////////
// Color
///////////////////////////////////////////////////////////////////////////////


// This line makes "Color" a synonym for "unsigned int", a positive number.
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

// MakeColorHSB is like MakeColor but instead of using red, green, and blue
// values, you get to specify the hue.  This is ideal for making rainbows!
//
// hue is an angle (in degrees) between 0 and 360.
// (0 is red, 30 is orange, 60 is yellow, 240 is blue...)
//
// saturation and brightness should be between 0 and 255.
// 
// saturation=0 makes everything grayscale.
// saturation=255 will be the most vivid version of that color.
//
// brightness=0 is always black.
// brightness=255 is the brightest version of that color.
//
Color MakeColorHSB(int hue, int saturation, int brightness);




///////////////////////////////////////////////////////////////////////////////
// Math Helpers
///////////////////////////////////////////////////////////////////////////////


// The number of radians in a complete circle (equal to 2*pi).
static constexpr double Tau = 6.283185307179586476925286766559;

// To find out how many radians there are in 45 degrees, call Radians(45).
static constexpr double Radians(double degrees) { return degrees * Tau / 360.0; }

// To find out how many degrees there are in half a circle, call Degrees(Tau / 2).
static constexpr double Degrees(double radians) { return radians * 360.0 / Tau; }




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

// Draws a rectangle with upper-left corner at (x, y) with the given width
// and height. The fill color will cause the inside of the rectangle to filled
// with that color.  The stroke color will be used to draw a single pixel
// border.  To skip drawing either the border or filling the inside, set that
// color to Transparent.
void DrawRectangle(int x, int y, int width, int height, Color fill, Color stroke = Transparent);

// Draws a circle centered at (x, y) with a given radius (in pixels).  Specify
// both the fill (inside) and stroke (single pixel border) colors.  To skip
// drawing either the stroke or fill, set its color to Transparent.
void DrawCircle(int x, int y, int radius, Color fill, Color stroke = Transparent);

// Draws a piece of a circle centered at (x, y) with the given stroke thickness.
// The piece of the circle you get is determined by the start and end radians.
// Zero radians is at the right-hand side of the circle and they increase upward.
// 0 for startRadians and Tau/2 for endRadians would draw the top half of a circle.
void DrawArc(int x, int y, float radius, float thickness, Color stroke, float startRadians, float endRadians);

// Prints text at (x, y) using the given font and size (in points).  It is normally
// left-justified but can be horizontally centered on x by passing true for centered.
void DrawString(int x, int y, const char *text, const char *fontName, int fontSizePt, const Color c, bool centered = false);


// Clears the screen to the given color (or Black if no color passed in).
void Clear(Color c = Black);

// Retrieves the color at the given location.  Test against specific colors:
//     if (ReadPixel(playerX, playerY) == Black) ...
//
Color ReadPixel(int x, int y);


// This makes "Image" a synonym for "int", a positive or negative number.
using Image = int;

const static Image InvalidImage = -1;

// Attempts to load and return an Image handle that can be used
// with DrawImage.  Many image file extensions are supported.
//
// To find the image you requested, the following locations are
// searched, in this order:
//   - Base64-encoded image data (any file format) passed in directly.
//   - Resources embedded inside the app's .exe file.
//   - Files in the current working directory.
// 
// If the image wasn't found or there was some other problem loading it,
// this function will return InvalidImage.
//
// Try to call this only once per file you want to load.  Keep the returned Image
// value around instead of calling this again and again.  Images remain loaded in
// memory until your program ends, so calling this repeatedly for the same image
// (say, inside a loop) will continue using more memory until your app crashes.
Image LoadImage(const char *name);

// Draws an image (obtained using LoadImage) with its top-left corner at the
// provided (x, y) coordinates at 100% scale.
//
// Image formats with animation frames (like .gif) cycle through their animation
// automatically as time goes on.  You just need to call DrawImage periodically
// so no frames are missed.
void DrawImage(int x, int y, Image i);

// Retrieves the width and height of an image (obtained using LoadImage).
int ImageWidth(Image i);
int ImageHeight(Image i);


// OPTIONAL!  Anti-aliasing is a graphics technique to make your lines and
// circles appear with smooth/soft edges.  These can be called at any time
// to change the way the other drawing functions behave.
void UseAntiAliasing();
void StopAntiAliasing();


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


// Generates a random true/false value (like a coin toss).
//
// This is useful when you need to make a decision:
//     if (RandomBool()) goToTheStore();
//     else stayHome();
//
bool RandomBool();

// Generates a random integer in the interval [low, high).
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
// Keyboard Input
///////////////////////////////////////////////////////////////////////////////


// Returns the most recently pressed keyboard character.  For special, non-
// printable characters, test against the values in "Keys", below.
//
// NOTE: To avoid reporting the same key press forever, this function only
//       returns the value ONCE and will subsequently report a value of zero
//       until the next key is pressed.  If you need to use this value more
//       than once, store the result in a variable first!
//
char LastKey();

// Used to check the results of LastKey() against non-printable keys:
//
//     // Save the value in a variable before testing it.
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


// Sometimes immediate input (where only the most-recent keypress is available) isn't
// the best fit.  If you don't want to miss any input, use this instead of LastKey.
// This is useful when you want to get typing input or fast arrow keys.
//
// Each call will return the next keypress in the stored list and then remove that
// entry from the list.  Calling this when no more input is available will return 0.
//
// Up to one hundred key presses can be recorded internally without calling this before
// they'll begin to be lost the same way LastKey loses all but the most recent key.
//
// NOTE: This works completely independently from LastKey.  The same input will be
//       reported by both functions separately.
//
char LastBufferedKey();

// If you haven't called LastBufferedKey in a while and would like to ignore any input
// that has come in since the last call (or since your program started running), call
// this to wipe out the internal list of recorded input.
void ClearInputBuffer();




///////////////////////////////////////////////////////////////////////////////
// Mouse Input
///////////////////////////////////////////////////////////////////////////////

// In order to make good use of mouse input, you'll want to set up your
// program so that input is checked and drawing is performed repeatedly,
// in a loop.  Game programmers usually call this the "game loop".


// Is the mouse button currently being held down?
bool LeftMousePressed();
bool RightMousePressed();
bool MiddleMousePressed();

// Retrieves the current mouse position (relative to the top-left corner of
// the drawing surface), or -1 if the mouse cursor isn't inside our window.
int MouseX();
int MouseY();




///////////////////////////////////////////////////////////////////////////////
// Music!
///////////////////////////////////////////////////////////////////////////////


// Play a music note (or rest) in the background.  Notes (or rests) are played
// one by one, back-to-back as they're queued by this function.  So you may
// send all the notes for a song and they'll be played in turn with the right
// timing automatically while your code continues to run normally.
//
// noteId 60 is middle C.
// noteId 61 is the C# just above that.
// noteId 62 is the D just above that.
//
// Use noteId 0 for a rest (no sound; just a pause in the music).
//
// noteId 21 is the lowest key on an 88 key piano.
// noteId 108 is the highest key on an 88 key piano.
//
// There are 1000 milliseconds in one second.
//
void PlayMusic(int noteId, int milliseconds);

// Clears all queued notes/rests.  This is useful if you need to play
// something more important before your current song has ended.
void ResetMusic();





























//
// Everything below this point is Immediate2D's implementation,
// which may be more challenging for beginners to read and follow.
//

extern const int Width;
extern const int Height;
extern const int PixelScale;



#ifdef IMM2D_IMPLEMENTATION

#include <map>
#include <mutex>
#include <deque>
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <numeric>
#include <algorithm>

#ifndef IMM2D_WIDTH
#define IMM2D_WIDTH 160
#endif

#ifndef IMM2D_HEIGHT
#define IMM2D_HEIGHT 120
#endif

#ifndef IMM2D_SCALE
#define IMM2D_SCALE 5
#endif

#ifndef IMM2D_WINDOW_TITLE
#define IMM2D_WINDOW_TITLE "Immediate2D"
#endif

const int Width = IMM2D_WIDTH;
const int Height = IMM2D_HEIGHT;
const int PixelScale = IMM2D_SCALE;

// The standard library's min/max algorithms conflict with Microsoft's
// min/max macros, but if you define NOMINMAX, the Windows header omits them.
#define NOMINMAX

// This trims the Windows.h header down to improve compile times
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Shlobj.h>
#include <Shlwapi.h>

// This is all we need that is removed by WIN32_LEAN_AND_MEAN
#include <mmeapi.h>
#include <timeapi.h>

// GDI+ uses the min/max macros, so we have to work around disabling them.
namespace Gdiplus { using std::min; using std::max; }
#include <GdiPlus.h>

// This instructs Visual Studio to add these to the list of libraries we link against
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

// The primary user-supplied function that we call on its own thread
extern void run();



//
// Immediate2D internal state
//

static bool imm2d_dirty{ true };
static bool imm2d_doubleBuffered{ false };

static std::atomic<char> imm2d_key{ 0 };
static std::atomic<bool> imm2d_quitting{ false };
static std::atomic<bool> imm2d_musicRunning{ true };
static std::atomic<bool> imm2d_mouseDown[3]{ false, false, false };
static std::atomic<int> imm2d_mouseX{ -1 }, imm2d_mouseY{ -1 };
static std::atomic<ULONGLONG> imm2d_runDuration{ 0 };

static std::mutex imm2d_bitmapLock;
static std::unique_ptr<Gdiplus::Bitmap> imm2d_bitmap, imm2d_bitmapOther;
static std::unique_ptr<Gdiplus::Graphics> imm2d_graphics, imm2d_graphicsOther;
static std::map<std::pair<std::string, int>, std::unique_ptr<Gdiplus::Font>> imm2d_fonts;

static std::mutex imm2d_mediaLock;
static std::vector<std::unique_ptr<Gdiplus::Bitmap>> imm2d_images;
static std::vector<std::pair<int, int>> imm2d_imageSizes;
static std::vector<uint32_t> imm2d_imageFrameCount;
static std::vector<size_t> imm2d_imageFrameStart;
static std::vector<uint32_t> imm2d_imageFrameCumulativeCentiSeconds;
static std::vector<uint32_t> imm2d_imageFrameSumMs;

static std::mutex imm2d_musicLock;

struct Imm2dMusicNote { uint8_t noteId; uint32_t duration; };
static std::deque<Imm2dMusicNote> imm2d_musicQueue;

static std::mutex imm2d_inputLock;
static std::deque<char> imm2d_inputBuffer;

void Present()
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);

    if (imm2d_doubleBuffered)
    {
        // This is more "offscreen composition" than double-buffering.  We could probably add a
        // an option to avoid this copy if the user really knew that they were going to redraw the
        // entire screen each frame.  But for now we assume immediate-mode drawing and make sure
        // things are consistent instead of flickering each frame if you've got different sets of
        // things drawn on each buffer
        imm2d_graphicsOther->DrawImage(imm2d_bitmap.get(), 0, 0);
    }

    std::swap(imm2d_graphics, imm2d_graphicsOther);
    std::swap(imm2d_bitmap, imm2d_bitmapOther);
    imm2d_dirty = true;
}

void CloseWindow() { imm2d_quitting = true; }
char LastKey() { return imm2d_key.exchange(0); }
void Wait(int milliseconds) { ::Sleep(milliseconds); }
void UseDoubleBuffering(bool enabled)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    imm2d_doubleBuffered = enabled;
    imm2d_dirty = true;
}

int MouseX() { return imm2d_mouseX; }
int MouseY() { return imm2d_mouseY; }
bool LeftMousePressed() { return imm2d_mouseDown[0]; }
bool RightMousePressed() { return imm2d_mouseDown[1]; }
bool MiddleMousePressed() { return imm2d_mouseDown[2]; }

static void imm2d_SetDirty()
{
    if (imm2d_bitmapLock.try_lock()) throw std::runtime_error("SetDirty must be called while bitmapLock is held.");
    if (!imm2d_doubleBuffered) imm2d_dirty = true;
}

static const std::wstring imm2d_ToWide(const char *utf8)
{
    const int wlen = ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    auto buffer = std::make_unique<wchar_t[]>(wlen);

    const int success = ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, buffer.get(), wlen);
    return success ? std::wstring(buffer.get()) : std::wstring();
}

char LastBufferedKey()
{
    std::lock_guard<std::mutex> lock(imm2d_inputLock);
    if (imm2d_inputBuffer.empty()) return 0;

    char k = imm2d_inputBuffer.front();
    imm2d_inputBuffer.pop_front();
    return k;
}

void ClearInputBuffer()
{
    std::lock_guard<std::mutex> lock(imm2d_inputLock);
    imm2d_inputBuffer.clear();
}

static void imm2d_AddBufferedKey(char c)
{
    std::lock_guard<std::mutex> lock(imm2d_inputLock);

    // If we're not using this feature, keep list a reasonable size
    imm2d_inputBuffer.push_back(c);
    while (imm2d_inputBuffer.size() > 100) imm2d_inputBuffer.pop_front();
}

// Nice, fast, reasonably high-quality public-domain PRNG from http://xoroshiro.di.unimi.it/xoroshiro128plus.c
//
// For random coordinates/colors in a tight loop, this outperforms std::mt19937 by a mile
//
uint64_t imm2d_xoroshiro128plus(void) {
    static uint64_t s[2] = { 1, static_cast<uint64_t>(time(NULL)) };
    auto rotl = [](const uint64_t x, int k) { return (x << k) | (x >> (64 - k)); };

    const uint64_t s0 = s[0];
    uint64_t s1 = s[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);
    s[1] = rotl(s1, 36);

    return result;
}

int RandomInt(int low, int high)
{
    if (high <= low) return low;
    return (imm2d_xoroshiro128plus() % (int64_t(high) - int64_t(low))) + low;
}

bool RandomBool()
{
    return RandomInt(0, 1) == 1;
}

double RandomDouble()
{
    union U { uint64_t i; double d; };
    return U{ UINT64_C(0x3FF) << 52 | imm2d_xoroshiro128plus() >> 12 }.d - 1.0;
}

// GDI+ makes us work a little harder before we can save as a particular image type
static CLSID imm2d_GetEncoderClsid(const std::wstring &format)
{
    UINT count{}, bytes{};
    Gdiplus::GetImageEncodersSize(&count, &bytes);

    // Something weird is going on here.  The returned size isn't just count*sizeof(ImageCodecInfo).
    auto codecs = std::make_unique<uint8_t[]>(bytes);
    if (!codecs) return CLSID{};

    Gdiplus::GetImageEncoders(count, bytes, reinterpret_cast<Gdiplus::ImageCodecInfo *>(codecs.get()));

    for (UINT i = 0; i < count; ++i)
    {
        const auto &codec = reinterpret_cast<Gdiplus::ImageCodecInfo *>(codecs.get())[i];
        if (wcscmp(codec.MimeType, format.c_str()) != 0) continue;
        return codec.Clsid;
    }

    return CLSID{};
}

void SaveImage(unsigned int suffix)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    static std::wstring desktop;
    if (desktop.empty())
    {
        wchar_t *desktopRaw = nullptr;
        if (SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopRaw) != S_OK) return;
        desktop = desktopRaw;
        CoTaskMemFree(desktopRaw);
    }

    std::wstring path = desktop + std::wstring(L"\\image");
    if (suffix > 0) path += L"_" + std::to_wstring(suffix);
    path += L".png";

    static const CLSID png = imm2d_GetEncoderClsid(L"image/png");
    imm2d_bitmap->Save(path.c_str(), &png, NULL);
}




// If your program is going to be writing every pixel every frame, this is a simple optimization
// over issuing Width*Height calls to DrawPixel.  Instead of spending time on the thread-safety
// overhead Width*Height times, the entire operation is performed in bulk behind a single lock.
//
// The first element in "screen" is the top-left (0, 0) pixel.  The next is (1, 0) and so on,
// wrapping automatically to the next row once you reach (Width - 1, 0) to (0, 1).  The passed-
// in screen must have example Width*Height elements!
//
// To create a screen that is the right size (and fill it with a default color):
//    std::vector<Color> screen(Width * Height, Black);
//
// To get the array index of a particular pixel:
//    int index = y*Width + x;
//
// Which can then be used directly to set a color:
//    screen[index] = LightBlue;
//
// And once you've placed every color value:
//    Present(screen);
//
void Present(const std::vector<Color> &screen);

Color MakeColor(int r, int g, int b, int a)
{
    return ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF) << 0);
}

constexpr Color MakeColor(int r, int g, int b)
{
    return 0xFF000000 | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF) << 0);
}

Color MakeColorHSB(int hue, int sat, int val)
{
    float h = std::min(1.0f, std::max(0.0f, (hue % 360) / 360.0f));
    float s = std::min(1.0f, std::max(0.0f, sat / 255.0f));
    float v = std::min(1.0f, std::max(0.0f, val / 255.0f));

    if (s == 0)
    {
        int gray = int(v * 255);
        return MakeColor(gray, gray, gray);
    }

    float var_h = h * 6;
    int var_i = int(var_h);
    float var_1 = v * (1 - s);
    float var_2 = v * (1 - s * (var_h - var_i));
    float var_3 = v * (1 - s * (1 - (var_h - var_i)));

    float var_r;
    float var_g;
    float var_b;

    switch (var_i)
    {
    case 0:  var_r = v;     var_g = var_3; var_b = var_1; break;
    case 1:  var_r = var_2; var_g = v;     var_b = var_1; break;
    case 2:  var_r = var_1; var_g = v;     var_b = var_3; break;
    case 3:  var_r = var_1; var_g = var_2; var_b = v;     break;
    case 4:  var_r = var_3; var_g = var_1; var_b = v;     break;
    default: var_r = v;     var_g = var_1; var_b = var_2; break;
    }

    return MakeColor(int(var_r * 255), int(var_g * 255), int(var_b * 255));
}

void imm2d_setAntiAliasing(bool enabled)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    imm2d_graphics->SetSmoothingMode(enabled ? Gdiplus::SmoothingModeAntiAlias : Gdiplus::SmoothingModeNone);
    imm2d_graphicsOther->SetSmoothingMode(enabled ? Gdiplus::SmoothingModeAntiAlias : Gdiplus::SmoothingModeNone);
}

void UseAntiAliasing() { imm2d_setAntiAliasing(true); }
void StopAntiAliasing() { imm2d_setAntiAliasing(false); }


void DrawPixel(int x, int y, Color c)
{
    if (x < 0 || x >= Width || y < 0 || y >= Height) return;

    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    Gdiplus::BitmapData d;

    Gdiplus::Rect r(x, y, 1, 1);
    imm2d_bitmap->LockBits(&r, Gdiplus::ImageLockModeWrite, imm2d_bitmap->GetPixelFormat(), &d);
    *reinterpret_cast<uint32_t *>(d.Scan0) = c;
    imm2d_bitmap->UnlockBits(&d);

    imm2d_SetDirty();
}

void Present(const std::vector<Color> &screen)
{
    if (screen.size() != Width * Height) return;

    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    Gdiplus::BitmapData d;
    Gdiplus::Rect r(0, 0, Width, Height);

    auto &b = imm2d_doubleBuffered ? imm2d_bitmapOther : imm2d_bitmap;
    b->LockBits(&r, Gdiplus::ImageLockModeWrite, b->GetPixelFormat(), &d);

    auto dstLine = reinterpret_cast<uint32_t *>(d.Scan0);
    for (int y = 0; y < Height; ++y)
    {
        auto srcLine = &screen[Width * y];
        memcpy(dstLine, srcLine, Width * sizeof(uint32_t));
        dstLine += d.Stride / 4;
    }

    b->UnlockBits(&d);
    imm2d_dirty = true;
}

Color ReadPixel(int x, int y)
{
    if (x < 0 || x >= Width || y < 0 || y >= Height) return Black;

    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return Black;

    Gdiplus::Color c;
    imm2d_bitmap->GetPixel(x, y, &c);
    return c.GetValue();
}

void DrawLine(int x1, int y1, int x2, int y2, int thickness, Color c)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    Gdiplus::Color color(c);
    Gdiplus::Pen p(c, (float)thickness);
    p.SetStartCap(Gdiplus::LineCapRound);
    p.SetEndCap(Gdiplus::LineCapRound);

    imm2d_graphics->DrawLine(&p, x1, y1, x2, y2);

    imm2d_SetDirty();
}

void DrawLine(float x1, float y1, float x2, float y2, int thickness, Color c)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    Gdiplus::Color color(c);
    Gdiplus::Pen p(c, (float)thickness);
    p.SetStartCap(Gdiplus::LineCapRound);
    p.SetEndCap(Gdiplus::LineCapRound);

    imm2d_graphics->DrawLine(&p, x1, y1, x2, y2);

    imm2d_SetDirty();
}

void DrawCircle(int x, int y, int radius, Color fill, Color stroke)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    Gdiplus::Rect r(x - radius, y - radius, radius * 2, radius * 2);

    if (fill != Transparent)
    {
        Gdiplus::SolidBrush brush{ Gdiplus::Color(fill) };
        imm2d_graphics->FillEllipse(&brush, r);
    }

    if (stroke != Transparent)
    {
        Gdiplus::Pen pen{ Gdiplus::Color(stroke) };
        imm2d_graphics->DrawEllipse(&pen, r);
    }

    imm2d_SetDirty();
}

void DrawCircle(float x, float y, float radius, Color fill, Color stroke)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    Gdiplus::RectF r(x - radius, y - radius, radius * 2, radius * 2);

    if (fill != Transparent)
    {
        Gdiplus::SolidBrush brush{ Gdiplus::Color(fill) };
        imm2d_graphics->FillEllipse(&brush, r);
    }

    if (stroke != Transparent)
    {
        Gdiplus::Pen pen{ Gdiplus::Color(stroke) };
        imm2d_graphics->DrawEllipse(&pen, r);
    }

    imm2d_SetDirty();
}

void DrawArc(int x, int y, float radius, float thickness, Color c, float startRadians, float endRadians)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    Gdiplus::Color color(c);
    Gdiplus::Pen p(c, thickness);
    p.SetStartCap(Gdiplus::LineCapRound);
    p.SetEndCap(Gdiplus::LineCapRound);

    const auto s = static_cast<Gdiplus::REAL>(startRadians * 360.0 / Tau);
    const auto e = static_cast<Gdiplus::REAL>(endRadians * 360.0 / Tau);
    imm2d_graphics->DrawArc(&p, x - radius, y - radius, radius * 2, radius * 2, s, e - s);
    imm2d_SetDirty();
}

void DrawRectangle(int x, int y, int width, int height, Color fill, Color stroke)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    // GDI+'s DrawRectangle and FillRectangle behave a little differently: One
    // of them treats the end coordinates as inclusive and the other exclusive
    const int adjustment = fill != Transparent ? 0 : -1;

    Gdiplus::Rect r(x, y, width + adjustment, height + adjustment);

    if (fill != Transparent)
    {
        Gdiplus::SolidBrush brush{ Gdiplus::Color(fill) };
        imm2d_graphics->FillRectangle(&brush, r);
    }

    if (stroke != Transparent)
    {
        Gdiplus::Pen pen{ Gdiplus::Color(stroke) };
        imm2d_graphics->DrawRectangle(&pen, r);
    }

    imm2d_SetDirty();
}

void DrawString(int x, int y, const char *text, const char *fontName, int fontPtSize, const Color c, bool centered)
{
    if (!text || !fontName) return;
    if (fontPtSize < 1 || !text[0]) return;

    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    static auto getFont = [](std::string name, int size) {
        const std::pair<std::string, int> key{ name, size };

        const auto found = imm2d_fonts.find(key);
        if (found != imm2d_fonts.end()) return &found->second;

        imm2d_fonts[key] = std::make_unique<Gdiplus::Font>(imm2d_ToWide(name.c_str()).c_str(), static_cast<Gdiplus::REAL>(size));
        return &imm2d_fonts[key];
    };

    const auto wide = imm2d_ToWide(text);
    const auto font = getFont(fontName, fontPtSize);
    const Gdiplus::SolidBrush brush(c);
    const Gdiplus::PointF origin{ static_cast<Gdiplus::REAL>(x), static_cast<Gdiplus::REAL>(y) };
    Gdiplus::StringFormat format;
    format.SetAlignment(centered ? Gdiplus::StringAlignmentCenter : Gdiplus::StringAlignmentNear);

    bool aa = imm2d_graphics->GetSmoothingMode() == Gdiplus::SmoothingModeAntiAlias;
    imm2d_graphics->SetTextRenderingHint(aa ? Gdiplus::TextRenderingHintAntiAlias : Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
    imm2d_graphics->DrawString(wide.c_str(), static_cast<INT>(wide.length()), font->get(), origin, &format, &brush);
    imm2d_SetDirty();
}

void Clear(Color c)
{
    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    imm2d_graphics->Clear(Gdiplus::Color(c));
    imm2d_SetDirty();
}

Gdiplus::Bitmap *imm2d_CheckedLoad(Gdiplus::Bitmap *b)
{
    // It's not enough to try and load a GDI+ bitmap.  It can return
    // a valid pointer even when it wasn't able to load anything. You
    // have to check the image status, too.  So we pass all of our
    // load calls through this double-checker.

    if (!b || b->GetLastStatus() == Gdiplus::Ok) return b;

    delete b;
    return nullptr;
}

Gdiplus::Bitmap *imm2d_LoadResourceImage(const char *resourceName)
{
    const HRSRC id = FindResourceA(nullptr, resourceName, "IMAGES");
    if (!id) return nullptr;

    const auto size = SizeofResource(nullptr, id);
    if (!size) return nullptr;

    const auto resource = LoadResource(nullptr, id);
    if (!resource) return nullptr;

    void *bytes = LockResource(resource);
    if (!bytes) { FreeResource(resource); return nullptr; }

    IStream *stream = SHCreateMemStream(static_cast<BYTE*>(bytes), size);
    if (!stream) { FreeResource(resource); return nullptr; }

    auto *result = imm2d_CheckedLoad(Gdiplus::Bitmap::FromStream(stream));
    stream->Release();
    FreeResource(resource);

    if (result->GetLastStatus() != Gdiplus::Ok) result = nullptr;
    return result;
}

Gdiplus::Bitmap *imm2d_LoadBase64Image(const char *base64)
{
    // Base64 decoding snippet adapted from https://stackoverflow.com/a/34571089

    static const std::vector<int> T = []{
        std::vector<int> lookup(256, -1);
        for (int i = 0; i < 64; i++) lookup["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
        return lookup;
    }();

    std::string decoded;
    unsigned int val = 0;
    int valb = -8;
    while (*base64)
    {
        const char c = *(base64++);
        const int lookup = T[c];
        if (lookup == -1)
        {
            // Unless this was padding, invalid data means this wasn't Base64 to begin with
            if (c != '=') decoded.clear();
            break;
        }

        val = (val << 6) + lookup, valb += 6;
        if (valb >= 0) { decoded.push_back(char((val >> valb) & 0xFF)); valb -= 8; }
    }

    if (decoded.empty()) return nullptr;

    IStream *stream = SHCreateMemStream((const BYTE*)decoded.data(), (UINT)decoded.size());
    if (!stream) return nullptr;

    auto *result = imm2d_CheckedLoad(Gdiplus::Bitmap::FromStream(stream));
    stream->Release();

    return result;
}

// Are we being a bad neighbor?  What's the likelihood that the user wants to use the
// Win32 API version of LoadImage in the same compilation unit as our implementation?
#ifdef LoadImage
#undef LoadImage
#endif

Image LoadImage(const char *name)
{
    if (!name) return InvalidImage;

    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return InvalidImage;

    // Try the Base64 case from the outset (which fails almost immediately
    // if the string being passed in isn't actually Base64 data)
    Gdiplus::Bitmap *result = imm2d_LoadBase64Image(name);

    // TODO: After 50 images, start storing a path-->Image map and checking it before re-loading.
    //       (This gets trickier with huge Base64-encoded inputs.  Maybe hash-->Image, instead?)

    if (!result) result = imm2d_LoadResourceImage(name);

    // TODO: Work much harder to find the file!
    if (!result) result = imm2d_CheckedLoad(Gdiplus::Bitmap::FromFile(imm2d_ToWide(name).c_str()));

    if (!result) return InvalidImage;
    const UINT frameCount = result->GetFrameCount(&Gdiplus::FrameDimensionTime);

    std::lock_guard<std::mutex> lock2(imm2d_mediaLock);
    imm2d_imageSizes.push_back(std::pair<int, int>(result->GetWidth(), result->GetHeight()));
    imm2d_images.push_back(std::unique_ptr<Gdiplus::Bitmap>(result));

    imm2d_imageFrameCount.push_back(frameCount);
    imm2d_imageFrameStart.push_back(imm2d_imageFrameCumulativeCentiSeconds.size());
    imm2d_imageFrameSumMs.push_back(0);

    if (frameCount > 0)
    {
        // GDI+ follows the usual Win32 convention of making you request the size of
        // the thing first, then having you provide a big enough buffer to fill it.
        // In this case, "it" is the list of "centi"-second delay between frames.
        const UINT bufferSize = result->GetPropertyItemSize(PropertyTagFrameDelay);
        auto itemBuffer = std::make_unique<uint8_t[]>(bufferSize);

        auto *item = reinterpret_cast<Gdiplus::PropertyItem *>(itemBuffer.get());
        result->GetPropertyItem(PropertyTagFrameDelay, bufferSize, item);
        const auto *frameCentiSeconds = reinterpret_cast<long*>(item->value);

        // TODO: What do "infinite"-length frames at the end of a GIF look like?
        // TODO: What does PropertyTagLoopCount look like?  Is that how we detect infinite loops?

        auto &sum = imm2d_imageFrameSumMs.back();
        for (size_t i = 0; i < frameCount; ++i)
        {
            const auto cSec = frameCentiSeconds[i];
            
            // TODO: If "infinite"-length frames are anywhere near LONG_MAX, we should check for overflows
            sum += cSec;
            imm2d_imageFrameCumulativeCentiSeconds.push_back(sum);
        }
        sum *= 10;
    }

    return static_cast<Image>(imm2d_images.size() - 1);
}

void DrawImage(int x, int y, Image i)
{
    if (i < 0) return;

    std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
    if (!imm2d_graphics) return;

    std::lock_guard<std::mutex> lock2(imm2d_mediaLock);
    if (imm2d_images.size() <= static_cast<size_t>(i)) return;

    auto *image = imm2d_images[i].get();
    const auto count = imm2d_imageFrameCount[i];
    if (count > 0)
    {
        const uint64_t now = imm2d_runDuration;
        const auto wrapped = now % std::max(1U, imm2d_imageFrameSumMs[i]);

        const auto begin = imm2d_imageFrameCumulativeCentiSeconds.cbegin() + imm2d_imageFrameStart[i];
        auto found = std::lower_bound(begin, begin + imm2d_imageFrameCount[i], wrapped / 10);

        uint32_t frameId = 0;
        if (found != imm2d_imageFrameCumulativeCentiSeconds.end()) frameId = static_cast<uint32_t>(std::distance(begin, found));

        image->SelectActiveFrame(&Gdiplus::FrameDimensionTime, frameId);
    }

    imm2d_graphics->DrawImage(image, x, y);
    imm2d_SetDirty();
}

int ImageWidth(Image i)
{
    if (i < 0) return 0;

    std::lock_guard<std::mutex> lock(imm2d_mediaLock);
    if (imm2d_imageSizes.size() <= static_cast<size_t>(i)) return 0;
    return imm2d_imageSizes[i].first;
}

int ImageHeight(Image i)
{
    if (i < 0) return 0;

    std::lock_guard<std::mutex> lock(imm2d_mediaLock);
    if (imm2d_imageSizes.size() <= static_cast<size_t>(i)) return 0;
    return imm2d_imageSizes[i].second;
}


static DWORD WINAPI imm2d_musicThreadProc(LPVOID)
{
    HMIDIOUT synth = nullptr;
    if (midiOutOpen(&synth, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) return 0;

    // We always use the "Lead 1 (Square)" instrument because it sounds like the PC speaker
    constexpr uint8_t Instrument = 80;
    midiOutShortMsg(synth, 0xC0 | (Instrument << 8));

    while (imm2d_musicRunning)
    {
        Imm2dMusicNote n;

        {
            std::unique_lock<std::mutex> lock(imm2d_musicLock);
            if (imm2d_musicQueue.empty())
            {
                lock.unlock();
                ::Sleep(1);
                continue;
            }

            n = imm2d_musicQueue.front();
            imm2d_musicQueue.pop_front();
        }

        if (n.noteId != 0) midiOutShortMsg(synth, 0x00700090 | (n.noteId << 8));
        ::Sleep(n.duration);
        if (n.noteId != 0) midiOutShortMsg(synth, 0x00000090 | (n.noteId << 8));
    }

    midiOutClose(synth);
    return 0;
}

void PlayMusic(int noteId, int ms)
{
    if (noteId < 0 || ms < 0) return;

    std::lock_guard<std::mutex> lock(imm2d_musicLock);
    if (!imm2d_musicRunning) return;

    imm2d_musicQueue.push_back(Imm2dMusicNote{ uint8_t(uint8_t(noteId) & 0x7F), static_cast<uint32_t>(ms) });
}

void ResetMusic()
{
    std::lock_guard<std::mutex> lock(imm2d_musicLock);
    imm2d_musicQueue.clear();
}


static LRESULT CALLBACK imm2d_WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
    static HDC bitmapDC{};
    static HBITMAP hbitmap{};

    switch (msg)
    {
    case WM_CREATE: return 0;
    case WM_ERASEBKGND: return 1;
    case WM_CLOSE: DestroyWindow(wnd); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(wnd, &ps);

        // A scaled GDI+ blit (even with nearest neighbor interpolation) is much slower than a plain
        // Win32 StretchBlt.  Even with the extra 1:1 copy to a GDI surface first, it's still faster.
        if (!hbitmap)
        {
            bitmapDC = CreateCompatibleDC(hdc);
            hbitmap = CreateCompatibleBitmap(hdc, Width, Height);
        }

        HANDLE old = SelectObject(bitmapDC, hbitmap);

        {
            std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
            Gdiplus::Graphics hdcG(bitmapDC);

            hdcG.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
            hdcG.DrawImage(imm2d_doubleBuffered ? imm2d_bitmapOther.get() : imm2d_bitmap.get(), 0, 0, 0, 0, Width, Height, Gdiplus::UnitPixel);
        }

        StretchBlt(hdc, 0, 0, Width * PixelScale, Height * PixelScale, bitmapDC, 0, 0, Width, Height, SRCCOPY);
        SelectObject(bitmapDC, old);

        EndPaint(wnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        imm2d_mouseDown[0] = (w & MK_LBUTTON) == MK_LBUTTON;
        imm2d_mouseDown[1] = (w & MK_RBUTTON) == MK_RBUTTON;
        imm2d_mouseDown[2] = (w & MK_MBUTTON) == MK_MBUTTON;
        imm2d_mouseX = ((int)(short)LOWORD(l)) / PixelScale;
        imm2d_mouseY = ((int)(short)HIWORD(l)) / PixelScale;
        break;

    case WM_MOUSELEAVE:
        imm2d_mouseX = -1;
        imm2d_mouseY = -1;
        break;

    case WM_CHAR:
        imm2d_AddBufferedKey(imm2d_key = (char)w);
        return 0;

    case WM_KEYDOWN:
    {
        auto thisKey = (char)MapVirtualKey((UINT)w, MAPVK_VK_TO_CHAR);
        if (w == VK_LEFT || w == VK_UP || w == VK_RIGHT || w == VK_DOWN) thisKey = char(w) - 0x14;
        if (thisKey < 32) imm2d_AddBufferedKey(imm2d_key = thisKey);
        return 0;
    }

    }

    return DefWindowProc(wnd, msg, w, l);
}

static DWORD WINAPI imm2d_threadProc(LPVOID) { run(); return 0; }

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int cmdShow)
{
    if constexpr (Width <= 0) { MessageBox(0, TEXT("IMM2D_WIDTH must be greater than 0."), TEXT("Bad Width"), MB_ICONERROR); return 1; }
    if constexpr (Height <= 0) { MessageBox(0, TEXT("IMM2D_HEIGHT must be greater than 0."), TEXT("Bad Height"), MB_ICONERROR); return 1; }
    if constexpr (PixelScale <= 0) { MessageBox(0, TEXT("IMM2D_SCALE must be greater than 0."), TEXT("Bad PixelScale"), MB_ICONERROR); return 1; }

    WNDCLASS wc{ CS_OWNDC, imm2d_WndProc, 0, 0, instance, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), nullptr, TEXT("Immediate2D") };
    if (!RegisterClass(&wc)) return 1;

    const DWORD style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;

    RECT r{ 0, 0, Width * PixelScale, Height * PixelScale };
    AdjustWindowRect(&r, style, FALSE);

    HWND wnd = CreateWindow(TEXT("Immediate2D"), TEXT(IMM2D_WINDOW_TITLE), style, CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, nullptr, nullptr, instance, nullptr);
    if (wnd == nullptr) return 1;

    // We want the best possible timer resolution for Sleep calls
    ::timeBeginPeriod(1);

    ULONG_PTR gdiPlusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiPlusToken, &gdiplusStartupInput, NULL);

    imm2d_bitmap = std::make_unique<Gdiplus::Bitmap>(Width, Height);
    imm2d_bitmapOther = std::make_unique<Gdiplus::Bitmap>(Width, Height);
    imm2d_graphics = std::make_unique<Gdiplus::Graphics>(imm2d_bitmap.get());
    imm2d_graphicsOther = std::make_unique<Gdiplus::Graphics>(imm2d_bitmapOther.get());
    StopAntiAliasing();
    Clear();

    ShowWindow(wnd, cmdShow);
    UpdateWindow(wnd);

    HANDLE musicThread = CreateThread(nullptr, 0, imm2d_musicThreadProc, nullptr, 0, nullptr);
    CreateThread(nullptr, 0, imm2d_threadProc, nullptr, 0, nullptr);

    const auto firstDraw = GetTickCount64();
    auto lastDraw = firstDraw;

    MSG message;
    while (true)
    {
        if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT) break;
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        if (imm2d_quitting.exchange(false)) PostQuitMessage(0);

        const auto now = GetTickCount64();
        imm2d_runDuration = now - firstDraw;

        if (now - lastDraw > 5)
        {
            std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
            if (imm2d_dirty) InvalidateRect(wnd, nullptr, FALSE);
            imm2d_dirty = false;

            lastDraw = now;
        }
        else ::Sleep(1);
    }

    {
        std::lock_guard<std::mutex> lock(imm2d_bitmapLock);
        imm2d_fonts.clear();
        imm2d_graphicsOther.reset();
        imm2d_graphics.reset();
        imm2d_bitmapOther.reset();
        imm2d_bitmap.reset();

        std::lock_guard<std::mutex> lock2(imm2d_mediaLock);
        imm2d_images.clear();

        Gdiplus::GdiplusShutdown(gdiPlusToken);

        std::lock_guard<std::mutex> lock3(imm2d_musicLock);
        imm2d_musicRunning = false;
        if (musicThread) WaitForSingleObject(musicThread, INFINITE);

        ::timeEndPeriod(1);

        // Without this, the main thread doesn't get killed fast enough to avoid
        // touching objects that have already been cleaned up after WinMain returns.
        ExitProcess((UINT)message.wParam);
    }

    return (UINT)message.wParam;
}

#endif





//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non - commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>
//
