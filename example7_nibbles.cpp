#include "drawing.h"
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

// The snakes (and the screen) are our only real global state
Snake snakes[2];


// Secret functions from drawing.cpp!
char LastBufferedKey();
void ClearInputBuffer();
void DrawString(int x, int y, const string &s, const Color c, bool centered = false);
void PlayMidiNote(int noteId, int milliseconds);

// We draw the play field at double-size to reach the 80x50 size of the original
void SetBigPixel(int x, int y, Color c)
{
    // We have a little extra vertical space left over, so leave some room for a header
    SetPixel(x * 2,     y * 2 + 10, c);
    SetPixel(x * 2 + 1, y * 2 + 10, c);
    SetPixel(x * 2,     y * 2 + 11, c);
    SetPixel(x * 2 + 1, y * 2 + 11, c);
}

Color GetBigPixel(int x, int y)
{
    // We assume all four pixels are the same color.  This routine just exists
    // to do the x2 and header padding calculation for us.
    return GetPixel(x * 2, y * 2 + 10);
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
    for (int i = 0; i < 8; ++i)
    {
        for (Snake &s : snakes)
            for (int j = i; j < s.body.size(); j += 8)
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

    // We use the (secret!) buffered input from drawing.cpp to make sure we don't miss any keypresses
    // from either player.  LastKey() is usually fine for intermittent input around 60 fps, but this
    // particular game relies on fast, sequential, low-latency inputs for multiple players.
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

    for (int i = 0; i < snakes[0].lives - 1; ++i) DrawRectangle(  2 + i * 6, 114, 4, 2, snakes[0].color, true);
    for (int i = 0; i < snakes[1].lives - 1; ++i) DrawRectangle(154 - i * 6, 114, 4, 2, snakes[1].color, true);
}

bool GameOverPlayAgain()
{
    // Discard any pre-existing input
    LastKey();

    DrawRectangle(41, 41, 80, 40, Black, true);
    DrawRectangle(40, 40, 80, 40, DarkGray, true);
    DrawRectangle(40, 40, 80, 40, LightGray, false);
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
    for (int n : { 0, 48, 50, 52, 50, 48, 50 }) PlayMidiNote(n, Duration8);
    for (int n : { 52, 48, 48 }) PlayMidiNote(n, Duration4);
}
void PlayLevelStart()
{
    for (int n : { 60, 62, 64, 62, 60, 62 }) PlayMidiNote(n, Duration20);
    for (int n : { 64, 60, 60 }) PlayMidiNote(n, Duration10);
}
void PlayApplePickup() { for (int n : { 48, 48, 48, 52 }) PlayMidiNote(n, Duration16); }
void PlaySnakeDead() { for (int n : { 36, 37, 39, 36, 37, 34, 32 }) PlayMidiNote(n, Duration32); }



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
