
//
// Example B - A Little Game
//
// This is a remaster/remake of an ancient game I made back in high-school
// (circa 1997?).  The only controls are the arrow keys.  Avoid the moving
// bugs, gather coins, get to the exit door.  The blue "triggers" make things
// in the level change.
//
// Tinker around in the exampleData/littleGame/levels.h file (and with the
// built in level editor; see below) to make your own levels.
//
// This example demonstrates loading images from resource files (among other
// things). The LoadImage call looks the same as loading from files or a
// Base64 encoded string, but the images are stored right inside the app
// itself.  The real action is in the exampleB_game.rc file that accompanies
// this one.  You can open that in any text editor to see the (very) simple
// pattern to follow.
//
// Getting the image resources to be included with your app also takes only
// a few steps.
//
// If you're building inside Visual Studio:
//
// 1. Just drag the .rc file to your project in the Solution Explorer pane.
// 2. Visual Studio will take care of the rest.
//
// If you're building from a Native Tools command prompt:
//
// 1. Run the following:  rc.exe myImages.rc
// 2. That will generate a .res file where all the images have been combined
//    into something that can be linked with the rest of your object files,
//    like this:  cl.exe /EHsc myCode.cpp myImages.res
//

#define IMM2D_IMPLEMENTATION
#include "immediate2d.h"

#include <algorithm>
#include <vector>
#include <map>

// To avoid a giant wall of something resembling ASCII art, the levels are
// broken out into their own file.  In Visual Studio you can put your text
// cursor over this filename and press F12 to open and go to it.
#include "exampleData/littleGame/levels.h"

// Our image tiles are square and 10 pixels on a side.  Defining this here
// (instead of sprinkling 10's in every calculation) would make it easier
// to swap out a larger/smaller tile set later.  It also makes the intent
// clearer.
constexpr int TileS = 10;

// TileS should divide Width and Height evenly so we don't end up with
// partial tiles hanging off the bottom or right side of the window.
static_assert(Width % TileS == 0);
static_assert(Height % TileS == 0);

// Since our tiles evenly divide our Width and Height, we can talk about
// a coarser grid in units of tiles.  These screen dimensions (in units
// of tiles) will be handy later:
constexpr int TileW = Width / TileS;
constexpr int TileH = Height / TileS;

// As long as the following lists are kept in sync, it's easier to
// handle all of our images together instead of treating them in a more
// separate, piecemeal way.
//
// The advantages of this style include:
// 1. We can load everything in a loop instead of a line per image.
// 2. Retrieving an image looks like "images[Door]" which is easy to
//    read.  Later we're going to be using variable names like "door"
//    for other things, so having the extra "images" in front of these
//    variable names will be a helpful clarification.
//
// TileLookup is more tangentially related, but we use it to convert
// an ASCII character into a TileId in several places, so it needs to
// be kept in sync with the other lists.
//
// Note: there isn't actually a "floor" image resource.  As a handy trick,
// we rely on the load failing and returning InvalidImage.  Then, when we
// try to "draw" a floor, it's a no-op that returns immediately.  Again,
// this is done for convenience so everything can be treated uniformly.
//
enum TileId { Smile, Door, Wall, Trigger, Coin, BugH, BugV, Floor };
static constexpr const char *ImageResourceNames[] = { "smile", "door", "wall", "trigger", "coin", "bugH", "bugV", "floor" };
static constexpr const char TileLookup[] = "@!#1$-| ";

// A "tile" represents a stationary square on the map.
// Things that move, like bugs, are stored separately.
//
// The data we actually care about is mostly just a TileId
// (which is what we store here 90% of the time) but in the
// case of triggers, we also need to know the template.  So
// instead of a TileId of Trigger, we use '0' to '9'.
using Tile = char;

static constexpr TileId LookupTileId(char c)
{
    if (c >= '0' && c <= '9') return Trigger;
    for (int i = 0; TileLookup[i]; ++i) if (c == TileLookup[i]) return (TileId)i;
    return Floor;
}

// Reading the description of how levels work in levels.h (included
// above) will inform the rest of the structures used here.

// A single trigger can perform many actions.  An action is the atomic unit
// of change in a level.  During level load, the shorthand used to specify
// trigger behavior is expanded out to as many actions as necessary.  The
// "7:G$." example in levels.h where point G is used ten times on the map will
// lead to ten separate actions being generated.
struct Action
{
    int x, y; // Where the action takes place
    Tile t;   // What should be placed there
};

// A trigger template is just a list of actions.
using TriggerTemplate = std::vector<Action>;

constexpr int CentipixelsPerPixel = 100;

struct Bug
{
    // These are in units of centi-pixels (or 1/100th of a pixel).
    int x, y;

    // We use the sprite to determine horizontal or vertical movement.
    TileId id;

    // Are they moving South/East (false means North or West).
    bool increasing{ true };

    // After it bonks into a wall, this counts down the wait
    // frames before starting to move in the other direction.
    int wait{};

    // Find the Manhattan distance in centi-pixels to some tile.
    // In a grid-based situation, Manhattan distance is more
    // useful than, say, Euclidean distance.
    int DistanceTo(int tX, int tY) const
    {
        const int cX = tX * TileS * CentipixelsPerPixel;
        const int cY = tY * TileS * CentipixelsPerPixel;
        return std::abs(cX - x) + std::abs(cY - y);
    }
};

struct Player
{
    // Players move a square at a time so the units here are in tiles.
    int x, y;
};

struct Level
{
    Level() = default;

    // Create a level from an encoded level string. The
    // levelNumber here is only used for reporting errors
    // on the screen if there was a problem loading the
    // level.  Set it to whatever you like.
    Level(const char *text, int levelNumber);

    // "Runs" an action, changing a single tile.
    void Apply(const Action &a) { map[a.y][a.x] = a.t; }

    Tile map[TileH][TileW]{};
    std::vector<Bug> bugs;
    std::vector<Player> players;

    // Our possible trigger names are 0 through 9, so ten at most.
    static constexpr int TemplateCount = 10;
    TriggerTemplate templates[TemplateCount];
};

// A few helpers at the end of the file so we can get right to the good stuff.
void PlaySFX(TileId t);
void DrawTitleCard(const char *text, Color c);
void SaveToClipboard(Level level);


void run()
{
    UseDoubleBuffering(true);

    // This isn't required, but loading every level up front will catch basic
    // typos in our strings without needing to play through the game each time.
    //
    // NOTE: We cast to void to let Visual Studio know we're intentionally
    //       discarding a temporary object.  (This supresses a warning.)
    int levelNumber{};
    for (const auto &text : LevelList) void(Level(text, ++levelNumber));

    std::vector<Image> images;
    for (auto *name : ImageResourceNames) images.push_back(LoadImage(name));

    // There is a crude level editor built in.  Activate it with the backtick
    // key (`).  Move your mouse over a tile, and press a character matching
    // the level file format.  Shortcuts: left click for walls.  Right click
    // for floor.  Enter will write the level string to the Windows clipboard.
    // Trigger actions still have to be entered manually in the text format.
    bool levelEditing = false;

    Level level;
    levelNumber = 0;
    for (const auto &text : LevelList)
    {
        ++levelNumber;

        bool reload = true;
        while (true)
        {
            Wait(10);

            // Getting hit by a bug makes the level restart, but we do it
            // at the top of the frame instead of deep in some loop where
            // we'd be pulling the state rug out from under ourselves.
            if (reload)
            {
                char buffer[40];
                sprintf_s(buffer, 40, "Level %d", levelNumber);

                static const Color TitleColor = MakeColorHSB(210, 255, 255);
                DrawTitleCard(buffer, TitleColor);
                Wait(1100);

                level = Level(text, levelNumber);
                ClearInputBuffer();
            }
            reload = false;

            Clear();

            // Map housekeeping and drawing.
            for (int y = 0; y < TileH; ++y)
            for (int x = 0; x < TileW; ++x)
            {
                auto tile = level.map[y][x];
                if (tile >= '0' && tile <= '9') tile = Trigger;

                // Convert bug tiles to real, living bugs.
                if (tile == BugH || tile == BugV)
                {
                    level.bugs.push_back(Bug{ x * TileS * CentipixelsPerPixel, y * TileS * CentipixelsPerPixel, (TileId)tile });
                    level.map[y][x] = Floor;
                }

                // Convert smile tiles to real, living players.
                if (tile == Smile)
                {
                    level.players.push_back(Player{ x, y });
                    level.map[y][x] = Floor;
                }

                DrawImage(x * TileS, y * TileS, images[tile]);
            };

            const char c = LastBufferedKey();
            const int playerDx = (c == Right) - (c == Left);
            const int playerDy = (c == Down) - (c == Up);

            // Shortest level editor, ever.
            if (c == '`') levelEditing = !levelEditing;
            if (levelEditing && MouseX() && MouseY())
            {
                DrawRectangle(0, 0, Width, Height, Transparent, LightRed);
                if (c == Enter) SaveToClipboard(level);

                const int mouseX = std::clamp(MouseX() / TileS, 0, TileW - 1);
                const int mouseY = std::clamp(MouseY() / TileS, 0, TileH - 1);
                Tile &mouseTile = level.map[mouseY][mouseX];

                if (LeftMousePressed()) mouseTile = Wall;
                if (RightMousePressed()) mouseTile = Floor;

                Tile fromKeyboard = LookupTileId(c);
                if (fromKeyboard == Trigger) fromKeyboard = c;
                if (fromKeyboard != Floor) mouseTile = fromKeyboard;
            }

            bool hitDoor{};

            // Player drawing and movement.
            for (auto &p : level.players)
            {
                DrawImage(p.x * TileS, p.y * TileS, images[Smile]);

                const int targetX = std::clamp(p.x + playerDx, 0, TileW - 1);
                const int targetY = std::clamp(p.y + playerDy, 0, TileH - 1);
                if (targetX == p.x && targetY == p.y) continue;

                auto tile = level.map[targetY][targetX];
                const int trigger = tile - '0';

                if (tile >= '0' && tile <= '9') tile = Trigger;
                PlaySFX((TileId)tile);

                switch (tile)
                {
                case Wall: continue;

                case Door:
                    hitDoor = true;
                    break;

                case Trigger:
                    for (const auto &a : level.templates[trigger]) level.Apply(a);
                    break;

                case Coin: break; // Coins only play a sound effect.
                }

                p.x = targetX;
                p.y = targetY;
                level.map[p.y][p.x] = Floor;
            }

            if (hitDoor)
            {
                // Wait for the level-finish music.
                Wait(1000);
                break;
            }

            for (int i = 0; i < level.bugs.size(); ++i)
            {
                auto &b = level.bugs[i];

                DrawImage(b.x / CentipixelsPerPixel, b.y / CentipixelsPerPixel, images[b.id]);

                const int tX = b.x / CentipixelsPerPixel / TileS;
                const int tY = b.y / CentipixelsPerPixel / TileS;

                // We treat bugs as usually occupying two tiles (even when they're
                // centered in one) to reduce the number of situations we need to
                // handle.  The only exception is when they've bonked into something
                // and are stopped.  Then we assume they only occupy a single tile.

                const int dX = b.id == BugH;
                const int dY = b.id == BugV;

                const int toCheckX[] = { tX, tX + dX };
                const int toCheckY[] = { tY, tY + dY };

                const bool waiting = b.wait > 0;
                const int tilesToCheck = waiting ? 1 : 2;

                constexpr int CrashDistance = TileS * CentipixelsPerPixel * 5 / 10;
                constexpr int SquishDistance = TileS * CentipixelsPerPixel * 99 / 100;

                bool squished = false;
                for (int t = 0; t < tilesToCheck; ++t)
                {
                    const int checkX = toCheckX[t];
                    const int checkY = toCheckY[t];
                    const bool insideBounds = checkX >= 0 && checkY >= 0 && checkX < TileW && checkY < TileH;

                    // Show which tiles are being hit tested against.
                    //DrawRectangle(checkX * TileS + 3, checkY * TileS + 3, 4, 4, White);

                    Tile tile = insideBounds ? level.map[checkY][checkX] : Wall;
                    if (tile >= '0' && tile <= '9') tile = Trigger;

                    switch (tile)
                    {
                    case Floor: break;

                    default:
                        b.increasing = !b.increasing;
                        b.wait = 35;

                        // Snap us back to the nearest pixel so we get back on the grid.
                        b.x = int((b.x + CentipixelsPerPixel / 2) / CentipixelsPerPixel) * CentipixelsPerPixel;
                        b.y = int((b.y + CentipixelsPerPixel / 2) / CentipixelsPerPixel) * CentipixelsPerPixel;

                        if (b.DistanceTo(checkX, checkY) < SquishDistance) squished = true;
                        break;
                    }
                }

                if (squished)
                {
                    // We use the same trick here as we do at the end of
                    // exampleA_snow to remove bugs in-place without
                    // copying every element in the list after this one.
                    level.bugs[i--] = level.bugs.back();
                    level.bugs.pop_back();

                    PlaySFX(BugH);
                    continue;
                }

                if (b.wait > 0) --b.wait;
                if (b.wait == 0)
                {
                    // This value was determined experimentally to match the animation speed.
                    constexpr int BugVelocityCentipixelsPerFrame = 13;

                    auto &value = (b.id == BugH) ? b.x : b.y;
                    value += BugVelocityCentipixelsPerFrame * (b.increasing ? 1 : -1);
                }

                // Check for collisions with players.
                for (const auto &p : level.players)
                {
                    if (b.DistanceTo(p.x, p.y) > CrashDistance) continue;

                    reload = true;
                    PlaySFX(b.id);

                    const int midX = (p.x * TileS + b.x / CentipixelsPerPixel + TileS) / 2;
                    const int midY = (p.y * TileS + b.y / CentipixelsPerPixel + TileS) / 2;

                    // Animate a little dust cloud.
                    for (int radius = 1; radius < 16; ++radius)
                    {
                        const double angle = RandomDouble() * Tau;
                        const int cloudX = int(midX + cos(angle) * radius);
                        const int cloudY = int(midY + sin(angle) * radius);
                        DrawCircle(cloudX, cloudY, radius, LightGray);

                        Present();
                        Wait(24);
                    }

                    break;
                }
            }

            Present();
        }
    }

    int hue = 0;
    while (LastKey() != Esc)
    {
        DrawTitleCard("Congratulations!", MakeColorHSB(hue, 180, 255));
        Sleep(16);

        hue += 4;
        if (hue >= 360) hue = 0;
    }
    CloseWindow();
}

// Draws an error message to the screen and sleeps until the window is closed.
// x and y should be provided when available to help narrow things down faster.
[[noreturn]] static void ReportLoadingErrorForever(const char *message, int levelNumber, int x = -1, int y = -1)
{
    Clear();
    StopAntiAliasing();

    char buffer[40];
    sprintf_s(buffer, 40, "Error loading level %d", levelNumber);
    DrawString(1, 1, buffer, "Small Fonts", 8, Red);

    if (x > 0 && y > 0)
    {
        sprintf_s(buffer, 40, "at point (%d, %d)", x, y);
        DrawString(1, 15, buffer, "Small Fonts", 8, Red);
    }

    DrawString(1, 28, message, "Small Fonts", 7, LightRed);

    Present();
    while (true) Wait(5000);
}

Level::Level(const char *text, int levelNumber)
{
    struct Point { int x, y; };
    std::map<char, std::vector<Point>> interest;

    // We're going to need these later when we're parsing trigger templates,
    // but we can squeeze a little bit of use out of them early, too.
    enum class Token { Number, Tilde, Colon, Letter, Tile, Period, Unknown };
    constexpr auto Tokenize = [](char c)
    {
        if (c == '~') return Token::Tilde;
        if (c == ':') return Token::Colon;
        if (c == '.') return Token::Period;
        if (c >= '0' && c <= '9') return Token::Number;
        if (c >= 'a' && c <= 'z') return Token::Letter;
        if (c >= 'A' && c <= 'Z') return Token::Letter;
        
        // We need to be careful that this is after Token::Number detection
        // because TileLookup contains a (placeholder) '1' as a stand-in for
        // Trigger to keep it the same length as the other lists.
        for (auto ch : TileLookup) if (c == ch) return Token::Tile;

        return Token::Unknown;
    };
    
    for (int y = 0; y < TileH; ++y)
    {
        for (int x = 0; x < TileW; ++x)
        {
            Tile &tile = map[y][x];
            tile = Floor;

            const char c = *text;
            if (c == 0) ReportLoadingErrorForever("Unexpected end of string", levelNumber, x, y);
            
            const auto t = Tokenize(c);
            if (t == Token::Unknown) ReportLoadingErrorForever("Unrecognized character", levelNumber, x, y);
            if (t == Token::Tile) tile = (Tile)LookupTileId(c);

            // Triggers are stored as their own ASCII number.
            if (t == Token::Number) tile = c;

            // Capitalized points of interest start as walls.
            if (c >= 'A' && c <= 'Z') tile = Wall;

            if (t == Token::Number || t == Token::Letter) interest[c].push_back({ x, y });

            ++text;

            // The way we define things in levels.h, we don't have any newlines, but if
            // you were reading this from, say, a text file, we'd need to skip over them.
            while (*text == '\n' || *text == '\r') ++text;
        }
    }

    // Without the period delimiter at the end of action lists, this would be
    // a little trickier to parse.  Consider the partial string "1:E#6".  The
    // first action is to place a wall at point E.  But when the six is read,
    // we wouldn't immediately know whether it was the beginning of the next
    // template or the next action for this template.  We'd need to keep more
    // state around to make the distinction.  (Worse, if we didn't have the
    // colon, it would become completely ambiguous.)  By adding a delimiter,
    // it makes it easy to parse every situation again.

    enum class State { Name, Target, Value };
    State state{};

    char name{};
    char target{};

    while (*text)
    {
        char c = *(text++);
        if (c == 0) break;

        Token t = Tokenize(c);

        // In general we're very permissive about ignoring extranneous input,
        // to allow for whitespace, etc.  It gets a little dicey with space
        // also counting as a floor tile, but the only place it can be read
        // as a floor tile is immediately after reading a target point.
        //
        // One good example of the permissivity is that we don't actually
        // parse the colon as a separate state.  It is simply treated as
        // an unrecognized character and skipped automatically.
        switch (state)
        {
        case State::Name:
            if (t != Token::Number && t != Token::Tilde) break;
            state = State::Target;
            name = c;
            break;

        case State::Target:
            // A period breaks the target-value cycle and starts us over.
            if (t == Token::Period) { state = State::Name; break; }

            if (t != Token::Letter && t != Token::Number) break;
            state = State::Value;
            target = c;
            break;

        case State::Value:
            if (t != Token::Tile && t != Token::Number) break;
                
            // Search the map for matching targets and add
            // an action to the list for each one.
            auto found = interest.find(target);
            if (found != cend(interest))
                for (auto &p : found->second)
                {
                    Tile tile = (Tile)LookupTileId(c);
                    if (tile == Trigger) tile = c;
                        
                    const Action a{ p.x, p.y, tile };
                        
                    // Actions that belong to the on-startup trigger are
                    // run immediately with no intermediate storage.
                    if (name == '~') Apply(a);
                    else templates[name - '0'].push_back(a);
                }

            state = State::Target;
            break;
        }

    }
}

void DrawTitleCard(const char *text, Color c)
{
    Clear();
    DrawString(Width / 2, Height / 2 - 12, text, "Arial", 12, c, true);
    Present();
}

void PlaySFX(TileId t)
{
    if (t == Floor) return;
    ResetMusic();

    switch (t)
    {
    case Coin:
        PlayMusic(83, 60);
        PlayMusic(88, 150);
        break;

    case BugH:
    case BugV:
        for (int n : { 36, 37, 39, 36, 37, 34, 32 }) PlayMusic(n, 60);
        break;

    case Door:
        PlayMusic(49, 303);
        PlayMusic(50, 110);
        PlayMusic(49, 211);
        PlayMusic(47, 182);
        PlayMusic(45, 200);
        PlayMusic(45, 87);
        PlayMusic(49, 54);
        PlayMusic(52, 45);
        PlayMusic(57, 117);
        break;

    case Trigger:
        PlayMusic(69, 43);
        PlayMusic(73, 27);
        PlayMusic(66, 23);
        PlayMusic(81, 117);
        break;

    default:
        for (int n : { 37, 34, 32 }) PlayMusic(n, 20);
        break;
    }
}

void SaveToClipboard(Level level)
{
    // Stamp the players and bugs back on the map as
    // tiles (as best we can, in the case of bugs).
    for (const auto &p : level.players) level.map[p.y][p.x] = Smile;
    for (const auto &b : level.bugs) level.map[b.y / CentipixelsPerPixel / TileS][b.x / CentipixelsPerPixel / TileS] = b.id;

    // Each line is padded with quotes and ends with a CRLF
    // pair.  The final +1 is for the null terminator.
    constexpr int LevelLength = (TileW + 4) * TileH + 1;

    // Here are the Win32 API incantations to
    // write an ANSI string to the system clipboard.
    if (!OpenClipboard(nullptr)) return;

    HGLOBAL global = GlobalAlloc(GHND, LevelLength);
    if (!global) { CloseClipboard(); return; }

    auto local = (char*)GlobalLock(global);
    if (local)
    {
        for (int y = 0; y < TileH; ++y)
        {
            *local++ = '\"';
            for (int x = 0; x < TileW; ++x)
            {
                const Tile t = level.map[y][x];
                if (t >= '0' && t <= '9') *local++ = (char)t;
                else *local++ = TileLookup[t];
            }
            for (auto c : { '\"', '\r', '\n' }) *local++ = c;
        }
        local = 0;
    }
    GlobalUnlock(global);

    if (EmptyClipboard()) SetClipboardData(CF_TEXT, global);
    else GlobalFree(global);

    CloseClipboard();
}
