
///////////////////////////////////////////////////////////////////////////////
//
// Immediate2D - Created by Nicholas Piegdon, released in the public domain
//
// A drawing framework for Windows that makes simple graphics programming as
// fun and easy as the days when computers booted directly to a BASIC prompt
//
///////////////////////////////////////////////////////////////////////////////

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


//
// Version 1.0 - Initial release
//


#include "drawing.h"

#include <map>
#include <mutex>
#include <deque>
#include <chrono>
#include <string>
#include <atomic>
#include <memory>
#include <random>
#include <thread>
#include <numeric>
#include <algorithm>
#include <functional>





///////////////////////////////////////////////////////////////////////////////
//
//           YOU SHOULD BE READING "drawing.h" INSTEAD OF THIS FILE!
//
///////////////////////////////////////////////////////////////////////////////

//
// There aren't really any user-serviceable parts in here. If you're a complete
// beginner in C++ (which makes you the intended audience), trying to read this
// file might be a little overwhelming.
//
// If you're interested in what's going on under the hood, the short version is
// that there is a lot of Windows-specific code here that creates a window and
// relays your drawing commands to the screen using the GDI+ framework.
//
// Your main() function is actually launched in a separate thread so the Windows
// code can keep running at the same time as your own.  This means that each of
// the drawing functions have to be written to safely transfer your request to
// the window's thread.  That "thread-safety" code may also look pretty strange.
//








//
// Read on at your own peril!  :)
//








#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <Shlobj.h>

// We have to go through a few gyrations to avoid the built-in Windows min/max macros (which GDI+ uses)
namespace Gdiplus { using std::min; using std::max; }
#include <GdiPlus.h>

// This instructs Visual Studio to add these to the list of libraries we link against
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

using namespace std;
using namespace std::chrono;

static_assert(Width > 0, "Width must be greater than 0.");
static_assert(Height > 0, "Height must be greater than 0.");
static_assert(PixelScale > 0, "PixelScale must be greater than 0.");

static bool dirty{ true };
static bool doubleBuffered{ false };

static atomic<char> key{ 0 };
static atomic<bool> quitting{ false };
static atomic<bool> musicRunning{ true };
static atomic<bool> mouseDown[3]{ false, false, false };
static atomic<int> mouseX{ -1 }, mouseY{ -1 };

static mutex bitmapLock;
static unique_ptr<Gdiplus::Bitmap> bitmap, bitmapOther;
static unique_ptr<Gdiplus::Graphics> graphics, graphicsOther;
static map<pair<string, int>, unique_ptr<Gdiplus::Font>> fonts;

static mutex musicLock;
static unique_ptr<thread> musicThread;

struct MusicNote { uint8_t noteId; milliseconds duration; };
static deque<MusicNote> musicQueue;

static mutex inputLock;
static deque<char> inputBuffer;

extern void main();

void Present()
{
    lock_guard<mutex> lock(bitmapLock);

    if (doubleBuffered)
    {
        // This is more "offscreen composition" than double-buffering.  We could probably add a
        // an option to avoid this copy if the user really knew that they were going to redraw the
        // entire screen each frame.  But for now we assume immediate-mode drawing and make sure
        // things are consistent instead of flickering each frame if you've got different sets of
        // things drawn on each buffer
        graphicsOther->DrawImage(bitmap.get(), 0, 0);
    }

    std::swap(graphics, graphicsOther);
    std::swap(bitmap, bitmapOther);
    dirty = true;
}

void CloseWindow() { quitting = true; }
char LastKey() { return key.exchange(0); }
void Wait(int milliseconds) { this_thread::sleep_for(std::chrono::milliseconds(milliseconds)); }
void UseDoubleBuffering(bool enabled)
{
    lock_guard<mutex> lock(bitmapLock);
    doubleBuffered = enabled;
    dirty = true;
}

int MouseX() { return mouseX; }
int MouseY() { return mouseY; }
bool LeftMousePressed() { return mouseDown[0]; }
bool RightMousePressed() { return mouseDown[1]; }
bool MiddleMousePressed() { return mouseDown[2]; }

static void SetDirty()
{
    if (bitmapLock.try_lock()) throw std::runtime_error("SetDirty must be called while bitmapLock is held.");
    if (!doubleBuffered) dirty = true;
}

static const wstring ToWide(const std::string &utf8)
{
    const int wlen = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    auto buffer = make_unique<wchar_t[]>(wlen);

    const int success = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, buffer.get(), wlen);
    return success ? wstring(buffer.get()) : wstring();
}

char LastBufferedKey()
{
    lock_guard<mutex> lock(inputLock);
    if (inputBuffer.empty()) return 0;

    char k = inputBuffer.front();
    inputBuffer.pop_front();
    return k;
}

void ClearInputBuffer()
{
    lock_guard<mutex> lock(inputLock);
    inputBuffer.clear();
}

static void AddBufferedKey(char c)
{
    lock_guard<mutex> lock(inputLock);

    // If we're not using this feature, keep list a reasonable size
    inputBuffer.push_back(c);
    while (inputBuffer.size() > 100) inputBuffer.pop_front();
}

// Nice, fast, reasonably high-quality public-domain PRNG from http://xoroshiro.di.unimi.it/xoroshiro128plus.c
//
// For random coordinates/colors in a tight loop, this outperforms std::mt19937 by a mile
//
uint64_t xoroshiro128plus(void) {
    static uint64_t s[2] = { 1, random_device()() };
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
    return (xoroshiro128plus() % (int64_t(high) - int64_t(low))) + low;
}

double RandomDouble()
{
    union U { uint64_t i; double d; };
    return U{ UINT64_C(0x3FF) << 52 | xoroshiro128plus() >> 12 }.d - 1.0;
}

// GDI+ makes us work a little harder before we can save as a particular image type
static CLSID GetEncoderClsid(const wstring &format)
{
    using namespace Gdiplus;

    UINT count, bytes;
    GetImageEncodersSize(&count, &bytes);

    // Something weird is going on here.  The returned size isn't just count*sizeof(ImageCodecInfo).
    auto codecs = make_unique<uint8_t[]>(bytes);
    if (!codecs) return CLSID{};

    GetImageEncoders(count, bytes, reinterpret_cast<ImageCodecInfo*>(codecs.get()));

    for (UINT i = 0; i < count; ++i)
    {
        const auto &codec = reinterpret_cast<ImageCodecInfo*>(codecs.get())[i];
        if (wcscmp(codec.MimeType, format.c_str()) != 0) continue;
        return codec.Clsid;
    }

    return CLSID{};
}

void SaveImage(unsigned int suffix)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    static wstring desktop;
    if (desktop.empty())
    {
        wchar_t *desktopRaw = nullptr;
        if (SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopRaw) != S_OK) return;
        desktop = desktopRaw;
        CoTaskMemFree(desktopRaw);
    }

    wstring path = desktop + wstring(L"\\image");
    if (suffix > 0) path += L"_" + to_wstring(suffix);
    path += L".png";

    static const CLSID png = GetEncoderClsid(L"image/png");
    bitmap->Save(path.c_str(), &png, NULL);
}



//
// If you've made it this far and you're still reading, here is a reward: secret functions!
//
// To avoid cluttering drawing.h (the whole point is to keep things as simple as possible and
// avoid overwhelming beginners), these extra-optional features were omitted from the header
// in the name of simplicity!  But, you can still use them just by copying the following
// declarations near the top of your main.cpp file:
//


// Creates a color using a (hue, saturation, value) triple instead of (red, green, blue).
//
// - S and V are in the usual [0, 256) range, but H is in degrees: [0, 360)
//
Color MakeColorHSV(int h, int s, int v);

// If your program is going to be writing every pixel every frame, this is a simple optimization
// over issuing Width*Height calls to SetPixel.  Instead of spending time on the thread-safety
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

// Sometimes immediate input (where the most-recent keypress overwrites all others) isn't the
// best fit.  If you don't want to miss any input, use this instead of LastKey.  Up to one
// hundred or so of the most recent keypresses are stored in a queue and can be retrieved
// one-by-one by calling this until it returns 0 (which indicates the input queue is empty).
//
// NOTE: This operates completely independently from LastKey.  The same input will be reported
//       by both functions separately.
//
char LastBufferedKey();

// Use in conjunction with LastBufferedKey.  If you haven't called LastBufferedKey in a while
// and would like to ignore any input that has come in since the last call (or since your
// program started running), call this to wipe out the contents of the input queue.
void ClearInputBuffer();

// This is the same DrawString from the Text example, available here so other examples (or you)
// can use it without a lot of distracting copy-paste.
void DrawString(int x, int y, const std::string &s, const Color c, bool centered = false);

// This DrawString uses the system's built-in font rendering and can handle UTF-8 strings to
// print Unicode characters in any size using any font installed on the system.
void DrawString(int x, int y, const std::string &s, const std::string &fontName, int fontPtSize, const Color c, bool centered = false);

// This is very similar to the other drawing calls, but for portions of a circle
void DrawArc(int x, int y, float radius, float thickness, Color c, float startRadians, float endRadians);

// Lets you draw using the raw GDI+ Graphics object if that's something you're interested in
void Draw(std::function<void(Gdiplus::Graphics &g)> f);

// This is kind of out of the scope of a drawing framework, but it's too much fun not to include!
//
// Uses the built-in Microsoft MIDI synth to play music notes in the background.  Only one note
// is ever played at once (monophonic) and notes are played back-to-back as they're queued by
// this function.  (Use a noteId of 0 to indicate a rest of a certain duration.)
//
// The note ID for middle C is 60 and they proceed chromatically.  So 61 is the C# above middle C
//
void PlayMidiNote(int noteId, int milliseconds);

// Because MIDI notes are queued and played sequentially in the background, you may need to
// interrupt whatever is currently playing in order to play something more important.
void ResetMusic();



Color MakeColor(int r, int g, int b, int a)
{
    return ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF) << 0);
}

constexpr Color MakeColor(int r, int g, int b)
{
    return (0xFF << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF) << 0);
}

Color MakeColorHSV(int hue, int sat, int val)
{
    float h = fmod(hue / 360.0f, 1.0f);
    float s = min(1.0f, max(0.0f, sat / 255.0f));
    float v = min(1.0f, max(0.0f, val / 255.0f));

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

void UseAntiAliasing(bool enabled)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    graphics->SetSmoothingMode(enabled ? Gdiplus::SmoothingModeAntiAlias : Gdiplus::SmoothingModeNone);
    graphicsOther->SetSmoothingMode(enabled ? Gdiplus::SmoothingModeAntiAlias : Gdiplus::SmoothingModeNone);
}

void SetPixel(int x, int y, Color c)
{
    if (x < 0 || x >= Width || y < 0 || y >= Height) return;

    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    Gdiplus::BitmapData d;

    Gdiplus::Rect r(x, y, 1, 1);
    bitmap->LockBits(&r, Gdiplus::ImageLockModeWrite, bitmap->GetPixelFormat(), &d);
    *reinterpret_cast<uint32_t*>(d.Scan0) = c;
    bitmap->UnlockBits(&d);

    SetDirty();
}

void Present(const std::vector<Color> &screen)
{
    if (screen.size() != Width * Height) return;

    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    Gdiplus::BitmapData d;
    Gdiplus::Rect r(0, 0, Width, Height);

    auto &b = doubleBuffered ? bitmapOther : bitmap;
    b->LockBits(&r, Gdiplus::ImageLockModeWrite, b->GetPixelFormat(), &d);

    auto dstLine = reinterpret_cast<uint32_t*>(d.Scan0);
    for (int y = 0; y < Height; ++y)
    {
        auto srcLine = &screen[Width * y];
        memcpy(dstLine, srcLine, Width * sizeof(uint32_t));
        dstLine += d.Stride / 4;
    }

    b->UnlockBits(&d);
    dirty = true;
}

Color GetPixel(int x, int y)
{
    if (x < 0 || x >= Width || y < 0 || y >= Height) return Black;

    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return Black;

    Gdiplus::Color c;
    bitmap->GetPixel(x, y, &c);
    return c.GetValue();
}

void DrawLine(int x1, int y1, int x2, int y2, int thickness, Color c)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    Gdiplus::Color color(c);
    Gdiplus::Pen p(c, (float)thickness);
    p.SetStartCap(Gdiplus::LineCapRound);
    p.SetEndCap(Gdiplus::LineCapRound);

    graphics->DrawLine(&p, x1, y1, x2, y2);

    SetDirty();
}

void DrawLine(float x1, float y1, float x2, float y2, int thickness, Color c)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    Gdiplus::Color color(c);
    Gdiplus::Pen p(c, (float)thickness);
    p.SetStartCap(Gdiplus::LineCapRound);
    p.SetEndCap(Gdiplus::LineCapRound);

    graphics->DrawLine(&p, x1, y1, x2, y2);

    SetDirty();
}

void DrawCircle(int x, int y, int radius, Color c, bool filled)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    Gdiplus::Rect r(x - radius, y - radius, radius * 2, radius * 2);
    Gdiplus::Color color(c);

    if (filled)
    {
        Gdiplus::SolidBrush brush(c);
        graphics->FillEllipse(&brush, r);
    }
    else
    {
        Gdiplus::Pen p(c);
        graphics->DrawEllipse(&p, r);
    }

    SetDirty();
}

void DrawCircle(float x, float y, float radius, Color c, bool filled)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    Gdiplus::RectF r(x - radius, y - radius, radius * 2, radius * 2);
    Gdiplus::Color color(c);

    if (filled)
    {
        Gdiplus::SolidBrush brush(c);
        graphics->FillEllipse(&brush, r);
    }
    else
    {
        Gdiplus::Pen p(c);
        graphics->DrawEllipse(&p, r);
    }

    SetDirty();
}

void DrawArc(int x, int y, float radius, float thickness, Color c, float startRadians, float endRadians)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    Gdiplus::Color color(c);
    Gdiplus::Pen p(c, thickness);
    p.SetStartCap(Gdiplus::LineCapRound);
    p.SetEndCap(Gdiplus::LineCapRound);

    const auto s = static_cast<Gdiplus::REAL>(startRadians * 360.0 / Tau);
    const auto e = static_cast<Gdiplus::REAL>(endRadians * 360.0 / Tau);
    graphics->DrawArc(&p, x - radius, y - radius, radius * 2, radius * 2, s, e - s);
    SetDirty();
}

void Draw(std::function<void(Gdiplus::Graphics &g)> f)
{
    if (!f) return;

    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    f(*graphics.get());

    SetDirty();
}

void DrawRectangle(int x, int y, int width, int height, Color c, bool filled)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    // GDI+'s DrawRectangle and FillRectangle behave a little differently: One
    // of them treats the end coordinates as inclusive and the other exclusive
    const int adjustment = filled ? 0 : -1;

    Gdiplus::Rect r(x, y, width + adjustment, height + adjustment);
    Gdiplus::Color color(c);

    if (filled)
    {
        Gdiplus::SolidBrush brush(c);
        graphics->FillRectangle(&brush, r);
    }
    else
    {
        Gdiplus::Pen p(c);
        graphics->DrawRectangle(&p, r);
    }

    SetDirty();
}


// For a line-by-line breakdown of this single-function text rendering library, see the Text example.  (This version has
// been compacted a bit and made a little more generic so we can draw directly into the density field in the smoke example.)
void DrawString(int x, int y, const string &s, const Color c, bool centered, function<void(int x, int y, Color c)> customDraw)
{
    static constexpr uint32_t Font[128 - 32] = {
        0x10000000, 0x10000017, 0x30000C03, 0x50AFABEA, 0x509AFEB2, 0x30004C99, 0x400A26AA, 0x10000003, 0x2000022E, 0x200001D1, 0x30001445, 0x300011C4, 0x10000018, 0x30001084, 0x10000010, 0x30000C98,
        0x30003A2E, 0x300043F2, 0x30004AB9, 0x30006EB1, 0x30007C87, 0x300026B7, 0x300076BF, 0x30007C21, 0x30006EBB, 0x30007EB7, 0x1000000A, 0x1000001A, 0x30004544, 0x4005294A, 0x30001151, 0x30000AA1,
        0x506ADE2E, 0x300078BE, 0x30002ABF, 0x3000462E, 0x30003A3F, 0x300046BF, 0x300004BF, 0x3000662E, 0x30007C9F, 0x1000001F, 0x30003E08, 0x30006C9F, 0x3000421F, 0x51F1105F, 0x51F4105F, 0x4007462E,
        0x300008BF, 0x400F662E, 0x300068BF, 0x300026B2, 0x300007E1, 0x30007E1F, 0x30003E0F, 0x50F8320F, 0x30006C9B, 0x30000F83, 0x30004EB9, 0x2000023F, 0x30006083, 0x200003F1, 0x30000822, 0x30004210,
        0x20000041, 0x300078BE, 0x30002ABF, 0x3000462E, 0x30003A3F, 0x300046BF, 0x300004BF, 0x3000662E, 0x30007C9F, 0x1000001F, 0x30003E08, 0x30006C9F, 0x3000421F, 0x51F1105F, 0x51F4105F, 0x4007462E,
        0x300008BF, 0x400F662E, 0x300068BF, 0x300026B2, 0x300007E1, 0x30007E1F, 0x30003E0F, 0x50F8320F, 0x30006C9B, 0x30000F83, 0x30004EB9, 0x30004764, 0x1000001F, 0x30001371, 0x50441044, 0x00000000,
    };

    if (centered) x -= accumulate(s.begin(), s.end(), 0, [](int a, char b) { return a + (b < 32 ? 0 : (Font[b - 32] >> 28) + 1); }) / 2;
    for (auto i : s)
    {
        if (i < 32 || i > 127) continue;
        uint32_t glyph = Font[i - 32];
        const int width = glyph >> 28;
        for (int u = x; u < x + width; ++u) for (int v = y; v < y + 5; ++v, glyph = glyph >> 1) if ((glyph & 1) == 1) customDraw(u, v, c);
        if (width > 0) x += width + 1;
    }
}

// Windows has their own SetPixel in global scope, so we need the static_cast to disambiguate from ours
void DrawString(int x, int y, const string &s, const Color c, bool centered)
{
    DrawString(x, y, s, c, centered, static_cast<void(*)(int, int, Color)>(SetPixel));
}

void DrawString(int x, int y, const string &s, const string &fontName, int fontPtSize, const Color c, bool centered)
{
    if (fontPtSize < 1 || s.empty()) return;

    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    static auto getFont = [] (string name, int size) {
        const pair<string, int> key{ name, size };

        const auto found = fonts.find(key);
        if (found != fonts.end()) return &found->second;

        fonts[key] = make_unique<Gdiplus::Font>(ToWide(name).c_str(), static_cast<Gdiplus::REAL>(size));
        return &fonts[key];
    };

    const auto wide = ToWide(s);
    const auto font = getFont(fontName, fontPtSize);
    const Gdiplus::SolidBrush brush(c);
    const Gdiplus::PointF origin{ static_cast<Gdiplus::REAL>(x), static_cast<Gdiplus::REAL>(y) };
    Gdiplus::StringFormat format;
    format.SetAlignment(centered ? Gdiplus::StringAlignmentCenter : Gdiplus::StringAlignmentNear);

    bool aa = graphics->GetSmoothingMode() == Gdiplus::SmoothingModeAntiAlias;
    graphics->SetTextRenderingHint(aa ? Gdiplus::TextRenderingHintAntiAlias : Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
    graphics->DrawString(wide.c_str(), static_cast<INT>(wide.length()), font->get(), origin, &format, &brush);
    SetDirty();
}

void Clear(Color c)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    graphics->Clear(Gdiplus::Color(c));
    SetDirty();
}

void PlayMidiNote(int noteId, int ms)
{
    if (noteId < 0 || ms < 0) return;

    lock_guard<mutex> lock(musicLock);
    if (!musicRunning) return;

    musicQueue.push_back(MusicNote{ uint8_t(uint8_t(noteId) & 0x7F), milliseconds(ms) });

    if (!musicThread) musicThread = make_unique<thread>([]()
    {
        HMIDIOUT synth = nullptr;
        if (midiOutOpen(&synth, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) return;

        // We always use the "Lead 1 (Square)" instrument because it sounds like the PC speaker
        constexpr uint8_t Instrument = 80;
        midiOutShortMsg(synth, 0xC0 | (Instrument << 8));

        while (musicRunning)
        {
            MusicNote n;

            {
                unique_lock<mutex> lock(musicLock);
                if (musicQueue.empty())
                {
                    lock.unlock();
                    this_thread::sleep_for(1ms);
                    continue;
                }

                n = musicQueue.front();
                musicQueue.pop_front();
            }

            if (n.noteId != 0) midiOutShortMsg(synth, 0x00700090 | (n.noteId << 8));
            this_thread::sleep_for(n.duration);
            if (n.noteId != 0) midiOutShortMsg(synth, 0x00000090 | (n.noteId << 8));
        }

        midiOutClose(synth);
    });
}

void ResetMusic()
{
    lock_guard<mutex> lock(musicLock);
    musicQueue.clear();
}


LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
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
            lock_guard<mutex> lock(bitmapLock);
            Gdiplus::Graphics hdcG(bitmapDC);

            hdcG.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
            hdcG.DrawImage(doubleBuffered ? bitmapOther.get() : bitmap.get(), 0, 0, 0, 0, Width, Height, Gdiplus::UnitPixel);
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
        mouseDown[0] = (w & MK_LBUTTON) == MK_LBUTTON;
        mouseDown[1] = (w & MK_RBUTTON) == MK_RBUTTON;
        mouseDown[2] = (w & MK_MBUTTON) == MK_MBUTTON;
        mouseX = GET_X_LPARAM(l) / PixelScale;
        mouseY = GET_Y_LPARAM(l) / PixelScale;
        break;

    case WM_MOUSELEAVE:
        mouseX = -1;
        mouseY = -1;
        break;

    case WM_CHAR:
        AddBufferedKey(key = (char)w);
        return 0;

    case WM_KEYDOWN:
    {
        char thisKey = MapVirtualKey((UINT)w, MAPVK_VK_TO_CHAR);
        if (w == VK_LEFT || w == VK_UP || w == VK_RIGHT || w == VK_DOWN) thisKey = char(w) - 0x14;
        if (thisKey < 32) AddBufferedKey(key = thisKey);
        return 0;
    }

    }

    return DefWindowProc(wnd, msg, w, l);
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int cmdShow)
{
    WNDCLASSW wc{ CS_OWNDC, WndProc, 0, 0, instance, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), nullptr, L"Immediate2D" };
    if (!RegisterClassW(&wc)) return 1;

    const DWORD style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;

    RECT r{ 0, 0, Width * PixelScale, Height * PixelScale };
    AdjustWindowRect(&r, style, FALSE);

    HWND wnd = CreateWindowW(L"Immediate2D", L"Immediate2D", style, CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, nullptr, nullptr, instance, nullptr);
    if (wnd == nullptr) return 1;

    ULONG_PTR gdiPlusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::Status startupResult = GdiplusStartup(&gdiPlusToken, &gdiplusStartupInput, NULL);

    bitmap = make_unique<Gdiplus::Bitmap>(Width, Height);
    bitmapOther = make_unique<Gdiplus::Bitmap>(Width, Height);
    graphics = make_unique<Gdiplus::Graphics>(bitmap.get());
    graphicsOther = make_unique<Gdiplus::Graphics>(bitmapOther.get());
    UseAntiAliasing(false);
    Clear();

    ShowWindow(wnd, cmdShow);
    UpdateWindow(wnd);

    thread(main).detach();

    auto lastDraw = high_resolution_clock::now();

    MSG message;
    while (true)
    {
        if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT) break;
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        if (quitting.exchange(false)) PostQuitMessage(0);

        const auto now = high_resolution_clock::now();
        if (now - lastDraw > 5ms)
        {
            lock_guard<mutex> lock(bitmapLock);
            if (dirty) InvalidateRect(wnd, nullptr, FALSE);
            dirty = false;

            lastDraw = now;
        }
        else this_thread::sleep_for(1ms);
    }

    {
        lock_guard<mutex> lock(bitmapLock);
        fonts.clear();
        graphicsOther.reset();
        graphics.reset();
        bitmapOther.reset();
        bitmap.reset();
        Gdiplus::GdiplusShutdown(gdiPlusToken);

        lock_guard<mutex> lock2(musicLock);
        musicRunning = false;
        if (musicThread) musicThread->join();

        // Without this, the main thread doesn't get killed fast enough to avoid
        // touching objects that have already been cleaned up after WinMain returns.
        ExitProcess((UINT)message.wParam);
    }

    return (UINT)message.wParam;
}
