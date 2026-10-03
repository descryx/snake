#include "raylib.h"
#include <cmath>
#include <vector>

enum class MovementMode { moveUp, moveDown, moveLeft, moveRight };
MovementMode currentMode = MovementMode::moveRight;

bool moveKeys(MovementMode &currentMode) {
    if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_K)) && (currentMode != MovementMode::moveDown)) {
        currentMode = MovementMode::moveUp;
        return true;
    }
    if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_J)) && (currentMode != MovementMode::moveUp)) {
        currentMode = MovementMode::moveDown;
        return true;
    }
    if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_H)) &&
        (currentMode != MovementMode::moveRight)) {
        currentMode = MovementMode::moveLeft;
        return true;
    }
    if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_L)) &&
        (currentMode != MovementMode::moveLeft)) {
        currentMode = MovementMode::moveRight;
        return true;
    }
    return false;
}

void moveSpeed(Vector2 &gridUnitPos, MovementMode &currentMode, const float &speed) {
    const float move = GetFrameTime() * 6.0f * speed;

    switch (currentMode) {
    case MovementMode::moveUp:
        gridUnitPos.y -= move;
        break;
    case MovementMode::moveDown:
        gridUnitPos.y += move;
        break;
    case MovementMode::moveLeft:
        gridUnitPos.x -= move;
        break;
    case MovementMode::moveRight:
        gridUnitPos.x += move;
        break;
    }
}

void addUnit(std::vector<Vector2> &gridUnitPos, Vector2 unitSize, MovementMode &currentMode) {
    switch (currentMode) {
    case MovementMode::moveUp:
        gridUnitPos.push_back(
            {gridUnitPos[gridUnitPos.size() - 1].x,
             gridUnitPos[gridUnitPos.size() - 1].y - (gridUnitPos.size() * unitSize.y)});
        break;

    case MovementMode::moveDown:
        gridUnitPos.push_back(
            {gridUnitPos[gridUnitPos.size() - 1].x,
             gridUnitPos[gridUnitPos.size() - 1].y + (gridUnitPos.size() * unitSize.y)});
        break;

    case MovementMode::moveLeft:
        gridUnitPos.push_back(
            {gridUnitPos[gridUnitPos.size() - 1].x - (gridUnitPos.size() * unitSize.x),
             gridUnitPos[gridUnitPos.size() - 1].y});
        break;

    case MovementMode::moveRight:
        gridUnitPos.push_back(
            {gridUnitPos[gridUnitPos.size() - 1].x + (gridUnitPos.size() * unitSize.x),
             gridUnitPos[gridUnitPos.size() - 1].y});
        break;
    }
}

void is_oob(Vector2 &gridUnitPos, Vector2 &gridSize, bool &dead) {
    if (gridUnitPos.x > gridSize.x || gridUnitPos.x < 0 || gridUnitPos.y > gridSize.y ||
        gridUnitPos.y < 0) {
        dead = true;
    }
}

int main(void) {

    Vector2 windowSize = {((1920.0f / 100) * 62.5), ((1080.0f / 100) * 62.5)};
    InitWindow(windowSize.x, windowSize.y, "Raylib Snake");

    SetTargetFPS(60);

    // NOTE: window dependent
    int collumns = 16;
    int rows = 9;
    float cellWidth = (windowSize.x / collumns);
    float cellHeight = (windowSize.y / rows);
    std::vector<Vector2> gridUnitPos;

    // NOTE: initial spawn of 1st head at ~middle of window
    gridUnitPos.push_back({roundf((collumns / 2)) * cellWidth, roundf((rows / 2)) * cellHeight});

    float speed = 50.0f;
    bool dead = false;
    Vector2 unitSize = {roundf((cellWidth / 100) * 75), roundf((cellHeight / 100) * 75)};

    // FIX: important: read the corrections.
    // TODO: implement so that while it moves i calculate for unit/units (idk yet)
    //       the grid position (eg. 16 and 9 grid) and if a unit is at a turn grid tile
    //       it turns. needed for that to work is a vector that stored the tiles that are
    //       considered turn tiles.
    //       need to store what tile and what direction it leads eg: tile: (7 * 3), dir: up
    //       and if any unit is on one of those tiles it turns in said direction.
    //
    //       FIX: 7 * 3 and 3 * 7 do collide, find another way to figure out grid coordinate
    //            eg (width 16 height 9) (coll 3 row 7):
    //            formula === ((row-1)*width)+coll
    //            ((7-1)*16)+3 == 99
    //            ((3-1)*16)+7 == 39
    //            name it tileId or smth like that
    //
    // TODO: recap of waht is to add: vector with tiel num and dir to turn to.
    //      calculation of the grid tile num. eg we have pos  x 1784, y 888.
    //      we need to take the cellHeight and cellWidth and look if the body is in the center
    //      of the cell. go and divide the x and y with height and width. we get a float number
    //      of the grid cell, but we need to round it to the bigger natural number.
    //      do not round down. if we get 8.2 we round to 9, because we are moving forward the
    //      whole time and need to see next tile we can turn at. past tiles are useless.
    //      after calculated witch tile we will turn at, upon arriving at that tile we see
    //      what last input was and turn acording.
    //      after this we store the tile at a vector via push_back as a number
    //      of the multiplication of the rounded x and y results we get, and the direction that
    //      we turned to.
    //      using a for loop we make each head check if it has arrived at that tile and if it has
    //      we turn that head. once the last head has turned we delete that tile from the vector.
    //      important to mention that (i forgot)
    //
    //      FIX: only round up the axis you are moving along!
    //           if you round up x while moving y it could lead to problems,
    //           theoreticaly it should not since you always stay at rounded numbers
    //           for the other axis, but safe is safe.
    //           also, if moving in -dir you should round down, via floor
    //           round = up
    //           floor = down
    //
    //      NOTE: also pay attentino to the fact the coordinate
    //            is not the real center of the square
    //
    // TODO: calculations:
    //       tile number: (x or y pixel pos) / (x or y cell width) for each seperate (x and y)
    //       !!! important to round to the next bigger whole number
    //       ...
    //       actuall tile: result of tile numbers multiplied
    //
    // TODO: to make a valid addition function, we need to grab the last snake unit we have and
    //       coresponding to the moving direciton add it behind the last unit and make it move
    //       in the same direction the last unit moves.
    //       each unit needs to have a seperate check if they have already apssed/arrived at a
    //       certain turning point tile.
    //       for that we can add a 3rd storing point to the tile where it coundts witch units
    //       already passed the tile to figure out when to "die" aka remove itself out of the
    //       list of tiles that make you turn.
    //       or i could make the units count waht tiles they pased at already. althought it
    //       feels a bit overcomplicated for no reason. having to store the units at 1 place
    //       for a tile is easier than having 10 different units remember each at least 1 tile.
    //       tile with 3 info fields it is.
    //
    //       FIX: dont make them target the real center of a tile, do: if close to tile middle
    //       turn at tile middle.
    //       by close enought i mean a window to turn that is big enought so that the frames
    //       let it even happen.
    //       a safe rule is a window of ±step/2,
    //       where step = speed * dt. Consecutive positions are step apart,
    //       so exactly one frame lands inside:
    //
    //
    // NOTE: the reason i use the grid and screen coordinates is to provide the easy playfield of
    //       a grid while still keeping the smoothnes of a sluently moving snake.

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_Q)) {
            break;
        }

        if (IsKeyPressed(KEY_A)) {
            addUnit(gridUnitPos, unitSize, currentMode);
        }

        if (IsWindowFocused() && !dead) {
            for (Vector2 &vec2 : gridUnitPos) {
                moveKeys(currentMode);
                moveSpeed(vec2, currentMode, speed);

                // is_oob(vec2, gridSize, dead); // FIX:
            }
        }

        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(BLACK);

        if (dead) {
            DrawText("skill issue\n(cool and defenetly appropriatly centered text)",
                     (windowSize.x / 2) - 50, (windowSize.y / 2) - 25, 24, WHITE);

        } else {
            for ([[maybe_unused]] Vector2 vec2 : gridUnitPos) {
                DrawRectangle(vec2.x, vec2.y, unitSize.x, unitSize.y, RED);
            }
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();

    return 0;
}
