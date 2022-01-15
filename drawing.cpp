
///////////////////////////////////////////////////////////////////////////////
//
// Immediate2D - Created by Nicholas Piegdon, released in the public domain
//
// A drawing framework for Windows that makes simple graphics programming as
// fun and easy as the days when computers booted directly to a BASIC prompt
//
// This is free and unencumbered software released into the public domain.
// See the full notice at the end of the file.
//
///////////////////////////////////////////////////////////////////////////////


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

// The standard library's min/max algorithms conflict with Microsoft's
// min/max macros, but if you define NOMINMAX, the Windows header omits them.
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <Shlobj.h>

// GDI+ uses the min/max macros, so we have to work around disabling them.
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

extern void run();

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

// Lets you draw using the raw GDI+ Graphics object if that's something you're interested in
void Draw(std::function<void(Gdiplus::Graphics &g)> f);


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
    float h = min(1.0f, max(0.0f, (hue % 360) / 360.0f));
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

void DrawPixel(int x, int y, Color c)
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

Color ReadPixel(int x, int y)
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

void DrawCircle(int x, int y, int radius, Color fill, Color stroke)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    Gdiplus::Rect r(x - radius, y - radius, radius * 2, radius * 2);

    if (fill != Transparent)
    {
        Gdiplus::SolidBrush brush{ Gdiplus::Color(fill) };
        graphics->FillEllipse(&brush, r);
    }

    if (stroke != Transparent)
    {
        Gdiplus::Pen pen{ Gdiplus::Color(stroke) };
        graphics->DrawEllipse(&pen, r);
    }

    SetDirty();
}

void DrawCircle(float x, float y, float radius, Color fill, Color stroke)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    Gdiplus::RectF r(x - radius, y - radius, radius * 2, radius * 2);

    if (fill != Transparent)
    {
        Gdiplus::SolidBrush brush{ Gdiplus::Color(fill) };
        graphics->FillEllipse(&brush, r);
    }

    if (stroke != Transparent)
    {
        Gdiplus::Pen pen{ Gdiplus::Color(stroke) };
        graphics->DrawEllipse(&pen, r);
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

void DrawRectangle(int x, int y, int width, int height, Color fill, Color stroke)
{
    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    // GDI+'s DrawRectangle and FillRectangle behave a little differently: One
    // of them treats the end coordinates as inclusive and the other exclusive
    const int adjustment = fill != Transparent ? 0 : -1;

    Gdiplus::Rect r(x, y, width + adjustment, height + adjustment);

    if (fill != Transparent)
    {
        Gdiplus::SolidBrush brush{ Gdiplus::Color(fill) };
        graphics->FillRectangle(&brush, r);
    }

    if (stroke != Transparent)
    {
        Gdiplus::Pen pen{ Gdiplus::Color(stroke) };
        graphics->DrawRectangle(&pen, r);
    }

    SetDirty();
}

void DrawString(int x, int y, const char *text, const char *fontName, int fontPtSize, const Color c, bool centered)
{
    if (!text || !fontName) return;
    if (fontPtSize < 1 || !text[0]) return;

    lock_guard<mutex> lock(bitmapLock);
    if (!graphics) return;

    static auto getFont = [] (string name, int size) {
        const pair<string, int> key{ name, size };

        const auto found = fonts.find(key);
        if (found != fonts.end()) return &found->second;

        fonts[key] = make_unique<Gdiplus::Font>(ToWide(name).c_str(), static_cast<Gdiplus::REAL>(size));
        return &fonts[key];
    };

    const auto wide = ToWide(text);
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

void PlayMusic(int noteId, int ms)
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

    thread(run).detach();

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
