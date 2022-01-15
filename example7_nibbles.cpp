
#define IMM2D_IMPLEMENTATION
#include "immediate2d.h"

#include <numeric>
#include <string>
#include <deque>
using namespace std;

//
// Example 7 - Nibbles
//
// A snake game modeled after the original NIBBLES.BAS included with Microsoft QBasic
//
// Exercises:
// 1. Add a pre-game menu to choose the color of your snake (from the EGA palette)
// 2. Add an option that increases the speed of the game after every apple
// 3. Design a new level or two
//

enum class Direction { Up, Down, Left, Right };
struct Point { int x, y; };

struct Snake
{
    deque<Point> body;
    Direction direction;

    int lives, score;
    Color color;
};

// The snakes (and the pixels on the screen) are our only real global state
Snake snakes[2];


// We draw the play field at double-size to reach the 80x50 size of the original
void SetBigPixel(int x, int y, Color c)
{
    // We have a little extra vertical space left over, so leave some room for a header
    DrawPixel(x * 2,     y * 2 + 10, c);
    DrawPixel(x * 2 + 1, y * 2 + 10, c);
    DrawPixel(x * 2,     y * 2 + 11, c);
    DrawPixel(x * 2 + 1, y * 2 + 11, c);
}

Color GetBigPixel(int x, int y)
{
    // We assume all four pixels are the same color.  This routine just exists
    // to do the x2 and header padding calculation for us.
    return ReadPixel(x * 2, y * 2 + 10);
}

// For a line-by-line breakdown of this single-function text rendering library,
// see the Text example.  (This version has been compacted and is a bit terse.)
void DrawString(int x, int y, const string &s, Color c, bool centered = false)
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
        for (int u = x; u < x + width; ++u)
            for (int v = y; v < y + 5; ++v, glyph = glyph >> 1)
                if ((glyph & 1) == 1)
                    DrawPixel(u, v, c);

        if (width > 0) x += width + 1;
    }
}


const Color Walls = LightRed;
const Color Background = Blue;
const Color Text = White;
const Color Apple = White;

void InitLevel(int level)
{
    Clear(Background);
    for (Snake &s : snakes) s.body.clear();

    // No matter the level, we always need a full perimeter box
    for (int i = 0; i < 80; ++i)
    {
        SetBigPixel(i,  0, Walls);
        SetBigPixel(i, 49, Walls);
    }
    for (int j = 0; j < 50; ++j)
    {
        SetBigPixel(0,  j, Walls);
        SetBigPixel(79, j, Walls);
    }

    switch (level)
    {
    case 1:
        snakes[0].body.push_front({ 50, 25 });
        snakes[1].body.push_front({ 30, 25 });
        snakes[0].direction = Direction::Right;
        snakes[1].direction = Direction::Left;
        break;

    case 2:
        for (int x = 20; x < 60; ++x) SetBigPixel(x, 25, Walls);
        snakes[0].body.push_front({ 60,  7 });
        snakes[1].body.push_front({ 20, 43 });
        snakes[0].direction = Direction::Left;
        snakes[1].direction = Direction::Right;
        break;

    case 3:
        for (int y = 10; y < 40; ++y)
        {
            SetBigPixel(20, y, Walls);
            SetBigPixel(60, y, Walls);
        }
        snakes[0].body.push_front({ 50, 25 });
        snakes[1].body.push_front({ 30, 25 });
        snakes[0].direction = Direction::Up;
        snakes[1].direction = Direction::Down;
        break;

    case 4:
        for (int y = 0; y < 30; ++y)
        {
            SetBigPixel(20,      y, Walls);
            SetBigPixel(60, 49 - y, Walls);
        }
        for (int x = 0; x < 40; ++x)
        {
            SetBigPixel(     x, 36, Walls);
            SetBigPixel(81 - x, 13, Walls);
        }
        snakes[0].body.push_front({ 60,  7 });
        snakes[1].body.push_front({ 20, 43 });
        snakes[0].direction = Direction::Left;
        snakes[1].direction = Direction::Right;
        break;

    case 5:
        for (int y = 11; y < 37; ++y)
        {
            SetBigPixel(21, y, Walls);
            SetBigPixel(58, y, Walls);
        }
        for (int x = 23; x < 57; ++x)
        {
            SetBigPixel(x,  9, Walls);
            SetBigPixel(x, 38, Walls);
        }
        snakes[0].body.push_front({ 50, 25 });
        snakes[1].body.push_front({ 30, 25 });
        snakes[0].direction = Direction::Up;
        snakes[1].direction = Direction::Down;
        break;

    case 6:
        for (int y = 0; y < 49; ++y)
        {
            if (y >= 19 && y <= 30) continue;
            for (int x = 10; x <= 70; x += 10) SetBigPixel(x, y, Walls);
        }
        snakes[0].body.push_front({ 65,  7 });
        snakes[1].body.push_front({ 15, 43 });
        snakes[0].direction = Direction::Down;
        snakes[1].direction = Direction::Up;
        break;

    case 7:
        for (int y = 1; y < 49; y += 2) SetBigPixel(40, y, Walls);
        snakes[0].body.push_front({ 65,  7 });
        snakes[1].body.push_front({ 15, 43 });
        snakes[0].direction = Direction::Down;
        snakes[1].direction = Direction::Up;
        break;

    case 8:
        for (int y = 0; y < 40; ++y)
            for (int x = 10; x <= 70; x += 10)
                SetBigPixel(x, x % 20 == 0 ? 49 - y : y, Walls);

        snakes[0].body.push_front({ 65,  7 });
        snakes[1].body.push_front({ 15, 43 });
        snakes[0].direction = Direction::Down;
        snakes[1].direction = Direction::Up;
        break;

    case 9:
        for (int i = 3; i < 47; ++i)
        {
            SetBigPixel(i +  2, i, Walls);
            SetBigPixel(i + 28, i, Walls);
        }
        snakes[0].body.push_front({ 75, 40 });
        snakes[1].body.push_front({  5, 15 });
        snakes[0].direction = Direction::Up;
        snakes[1].direction = Direction::Down;
        break;

    default:
        // Repeat the final level forever
        for (int y = 1; y < 49; y += 2)
            for (int j = 0; j < 7; ++j)
                SetBigPixel(j * 10 + 10, y + (j % 2 == 0 ? 0 : 1), Walls);

        snakes[0].body.push_front({ 65,  7 });
        snakes[1].body.push_front({ 15, 43 });
        snakes[0].direction = Direction::Down;
        snakes[1].direction = Direction::Up;
        break;
    }

    // Duplicate the head of each snake so we're not just a
    // single-pixel head running around at the start of the level
    for (Snake &s : snakes) s.body.push_back(s.body.back());
}

void EraseSnakes()
{
    // Erase using a neat segmented effect
    for (size_t i = 0; i < 8; ++i)
    {
        for (Snake &s : snakes)
            for (size_t j = i; j < s.body.size(); j += 8)
                SetBigPixel(s.body[j].x, s.body[j].y, Background);

        Wait(35);
    }
}

void Pause(const string &message)
{
    // Discard any pre-existing input
    LastKey();

    DrawString(80, 113, message, White, true);
    while (LastKey() != ' ') Wait(1);
    DrawString(80, 113, message, Background, true);

    // Wipe out any other keyboard input that happened during the pause
    ClearInputBuffer();
}

void GenerateApple()
{
    while (true)
    {
        int x = RandomInt(1, 79);
        int y = RandomInt(1, 49);
        if (GetBigPixel(x, y) != Background) continue;

        SetBigPixel(x, y, Apple);
        return;
    }
}

bool ReadInputAndDelay()
{
    Wait(80);

    // We use buffered input so we don't miss any keypresses from either player.
    // LastKey() is usually fine for intermittent input around 60 fps, but this
    // game relies on fast, sequential, low-latency inputs for multiple players.
    Direction desired[2] = { snakes[0].direction, snakes[1].direction };
    while (char k = LastBufferedKey())
    {
        switch (k)
        {
        case 'p':
        case 'P':   Pause("Paused! Press Space"); break;

        case 'w':
        case 'W':   if (snakes[1].direction != Direction::Down)  desired[1] = Direction::Up;    break;
        case 'a':
        case 'A':   if (snakes[1].direction != Direction::Right) desired[1] = Direction::Left;  break;
        case 's':
        case 'S':   if (snakes[1].direction != Direction::Up)    desired[1] = Direction::Down;  break;
        case 'd':
        case 'D':   if (snakes[1].direction != Direction::Left)  desired[1] = Direction::Right; break;

        case Up:    if (snakes[0].direction != Direction::Down)  desired[0] = Direction::Up;    break;
        case Left:  if (snakes[0].direction != Direction::Right) desired[0] = Direction::Left;  break;
        case Down:  if (snakes[0].direction != Direction::Up)    desired[0] = Direction::Down;  break;
        case Right: if (snakes[0].direction != Direction::Left)  desired[0] = Direction::Right; break;

        case Esc:
            CloseWindow();
            return false;
        }

        // This technically only allows a single input per frame, but
        // it's required for no inputs to be "lost" when trying to do
        // a single pixel jaunt.
        if (desired[0] != snakes[0].direction
         || desired[1] != snakes[1].direction) break;
    }

    snakes[0].direction = desired[0];
    snakes[1].direction = desired[1];
    return false;
}

Point AdvancePoint(Point p, Direction d)
{
    switch (d)
    {
    case Direction::Left:  return Point{ p.x - 1, p.y };
    case Direction::Right: return Point{ p.x + 1, p.y };
    case Direction::Up:    return Point{ p.x, p.y - 1 };
    case Direction::Down:  return Point{ p.x, p.y + 1 };
    default: return p;
    }
}

void DrawScores(int playerCount)
{
    DrawRectangle(0, 0, Width, 10, Background, true);

    if (playerCount > 0) DrawString(  5, 3,               to_string(snakes[0].score) + " <-- Sammy", snakes[0].color);
    if (playerCount > 1) DrawString(110, 3, "Jake --> " + to_string(snakes[1].score),                snakes[1].color);

    for (int i = 0; i < snakes[0].lives - 1; ++i) DrawRectangle(  2 + i * 6, 114, 4, 2, snakes[0].color);
    for (int i = 0; i < snakes[1].lives - 1; ++i) DrawRectangle(154 - i * 6, 114, 4, 2, snakes[1].color);
}

bool GameOverPlayAgain()
{
    // Discard any pre-existing input
    LastKey();

    DrawRectangle(41, 41, 80, 40, Black);
    DrawRectangle(40, 40, 80, 40, DarkGray);
    DrawRectangle(40, 40, 80, 40, Transparent, LightGray);
    DrawString(80, 50, "G A M E   O V E R", LightGray, true);
    DrawString(80, 66, "Play Again? (Y/N)", White, true);

    while (true)
    {
        const char c = LastKey();
        if (c == 'y' || c == 'Y') return true;
        if (c == 'n' || c == 'N') return false;
        Wait(1);
    }
}

int HowManyPlayers()
{
    // Discard any pre-existing input
    LastKey();

    DrawString(80, 40, "How many players (1 or 2)?", LightGray, true);

    while (true)
    {
        const char c = LastKey();
        if (c == '1') return 1;
        if (c == '2') return 2;
        Wait(1);
    }
}


// Music timing
constexpr int MsPerMinute = 60000;
constexpr int BPM = 160;
constexpr int BeatsPerMs = MsPerMinute / BPM;

constexpr int Duration4 =  BeatsPerMs * 1 / 1;
constexpr int Duration8 =  BeatsPerMs * 1 / 2;
constexpr int Duration10 = BeatsPerMs * 2 / 5;
constexpr int Duration16 = BeatsPerMs * 1 / 4;
constexpr int Duration20 = BeatsPerMs * 1 / 5;
constexpr int Duration32 = BeatsPerMs * 1 / 8;

// Nothing to see here except lists of music note IDs (60 is middle C)
void PlayTitleMusic()
{
    for (int n : { 0, 48, 50, 52, 50, 48, 50 }) PlayMusic(n, Duration8);
    for (int n : { 52, 48, 48 }) PlayMusic(n, Duration4);
}
void PlayLevelStart()
{
    for (int n : { 60, 62, 64, 62, 60, 62 }) PlayMusic(n, Duration20);
    for (int n : { 64, 60, 60 }) PlayMusic(n, Duration10);
}
void PlayApplePickup() { for (int n : { 48, 48, 48, 52 }) PlayMusic(n, Duration16); }
void PlaySnakeDead() { for (int n : { 36, 37, 39, 36, 37, 34, 32 }) PlayMusic(n, Duration32); }



void run()
{
    DrawString(80, 10, "C + +    N i b b l e s", White, true);
    DrawLine(47, 17, 111, 17, 1, DarkGray);
    DrawString(80, 26, "Eat apples while avoiding walls & snakes.", LightGray, true);
    DrawString(80, 32, "The more you eat, the longer you become.", LightGray, true);
    DrawString(80, 50, "Player 1: Arrow keys", Yellow, true);
    DrawString(80, 56, "Player 2: W, A, S, D", LightMagenta, true);
    DrawString(80, 62, "P to Pause", LightGray, true);
    DrawString(80, 90, "Press any key to continue", White, true);

    PlayTitleMusic();
    while (LastKey() == 0) Wait(1);

    // This is the only one-time initialization our snakes need
    snakes[0].color = Yellow;
    snakes[1].color = LightMagenta;

    while (true)
    {
        Clear();
        const int players = HowManyPlayers();

        // Reset scores and lives at the start of each new game
        snakes[0].lives = snakes[1].lives = 5;
        snakes[0].score = snakes[1].score = 0;

        // Kill off player 2 right away if we're in 1-player mode
        if (players == 1) snakes[1].lives = 0;

        int level = 1;
        int pickupCount = 0;
        constexpr int PickupLimit = 9;

        while (snakes[0].lives > 0 || snakes[1].lives > 0)
        {
            InitLevel(level);
            DrawScores(players);
            Pause("Level " + to_string(level) + ", push space");
            PlayLevelStart();

            GenerateApple();

            bool dead = false;
            while (!dead && pickupCount < PickupLimit)
            {
                if (ReadInputAndDelay()) break;

                // Process each snake's movement
                for (Snake &s : snakes)
                {
                    // Skip dead snakes
                    if (s.lives == 0) continue;

                    const Point newHead = AdvancePoint(s.body.front(), s.direction);

                    // Check to see if we've hit anything
                    const Color hit = GetBigPixel(newHead.x, newHead.y);
                    if (hit == Apple)
                    {
                        PlayApplePickup();
                        pickupCount++;
                        if (pickupCount < PickupLimit) GenerateApple();

                        s.score += pickupCount;
                        const int newSegments = pickupCount * 4;
                        for (int j = 0; j < newSegments; ++j) s.body.push_back(s.body.back());

                        DrawScores(players);
                    }
                    else if (hit != Background)
                    {
                        PlaySnakeDead();

                        s.lives--;
                        dead = true;
                        break;
                    }

                    SetBigPixel(newHead.x, newHead.y, s.color);
                    s.body.push_front(newHead);

                    Point oldTail = s.body.back();
                    SetBigPixel(oldTail.x, oldTail.y, Background);
                    s.body.pop_back();
                }
            }

            if (pickupCount >= PickupLimit)
            {
                ++level;
                pickupCount = 0;
            }

            EraseSnakes();
        }

        if (!GameOverPlayAgain()) CloseWindow();
    }

}
