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
             gridUnitPos[gridUnitPos.size() - 1].y - (gridUnitPos.size() + unitSize.y)});
        break;

    case MovementMode::moveDown:
        gridUnitPos.push_back(
            {gridUnitPos[gridUnitPos.size() - 1].x,
             gridUnitPos[gridUnitPos.size() - 1].y + (gridUnitPos.size() + unitSize.y)});
        break;

    case MovementMode::moveLeft:
        gridUnitPos.push_back(
            {gridUnitPos[gridUnitPos.size() - 1].x - (gridUnitPos.size() + unitSize.x),
             gridUnitPos[gridUnitPos.size() - 1].y});
        break;

    case MovementMode::moveRight:
        gridUnitPos.push_back(
            {gridUnitPos[gridUnitPos.size() - 1].x + (gridUnitPos.size() + unitSize.x),
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
    std::vector<Vector3> turnTile;
    std::vector<MovementMode> tileMode;

    // NOTE: initial spawn of 1st head at ~middle of window
    gridUnitPos.push_back({roundf(collumns / 2) * cellWidth, roundf(rows / 2) * cellHeight});

    float speed = 25.0f;
    bool dead = false;
    Vector2 unitSize = {roundf(cellWidth / 100 * 75), roundf(cellHeight / 100 * 75)};

    // FIX: a 16x9 grid playfield, but units move smoothly in pixels (not cell-by-cell).
    //
    // NOTE:
    //   Segment { Vector2 pos; MovementMode dir; }       // every unit has its OWN direction
    //   Turn    { int col, row; MovementMode dir; int passed = 0; }
    //   (Store col and row separately. Multiplying them collides: 7*3 == 3*7.
    //   If you want one number: tileId = row * columns + col, 0-based.)
    //
    // TODO:
    //  WHEN THE PLAYER TURNS THE HEAD
    //   1. Find the NEXT tile the head will reach along its current axis of movement:
    //        moving + direction (right/down) -> ceil
    //        moving - direction (left/up)    -> floor
    //      Only round the axis you are moving along. The other axis is already on a grid line.
    //   2. push_back a Turn { col, row, newDir, passed = 0 }.
    //
    // TODO:
    // EVERY FRAME, FOR EACH SEGMENT i
    //   1. Target = the first Turn in the list with passed == i
    //      (segments reach tiles in order, so passed == "how many segments already went through").
    //   2. step = speed * GetFrameTime().
    //      If the segment is within +-step/2 of the tile's position on its moving axis:
    //        - snap pos to the tile position
    //        - seg.dir = turn.dir
    //        - turn.passed++
    //      Window of +-step/2 guarantees exactly one frame lands inside it.
    //   3. Move the segment ONCE, using seg.dir (never the head's dir, never inside the turn loop).
    //
    // TODO:
    // AFTER THE LOOP
    //   Remove turns where passed == snake.size() (erase-remove, never erase while iterating).
    //
    // TODO:
    // ADD A SEGMENT
    //   Spawn one cell behind the tail, opposite the TAIL's dir, and copy the tail's dir.
    //   (Edge case to think about: pending turns the new segment still has to pass.)
    //
    // NOTE:
    //   - pos is the top-left of the drawn rect, NOT the true center of the cell.
    //     The unit is 75% of the cell, so offset by (cell - unit) / 2 if you want it centered.
    //   - x uses cellWidth, y uses cellHeight (currently y uses cellWidth in two places).
    //   - Why grid + pixel coordinates: easy grid logic, smooth movement.ping the smoothnes of a
    //   sluently moving snake.

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_Q)) {
            break;
        }

        if (IsKeyPressed(KEY_A)) {
            addUnit(gridUnitPos, unitSize, currentMode);
        }

        if (IsWindowFocused() && !dead) {

            if (moveKeys(currentMode)) {
                tileMode.push_back(currentMode);
                if (currentMode == MovementMode::moveUp) { // TODO: make back into 1 liner
                    Vector2 snapTile = {gridUnitPos[0].x / cellWidth,
                                        roundf(gridUnitPos[0].y / cellHeight)};
                    turnTile.push_back({snapTile.x, snapTile.y, 0});
                }
                if (currentMode == MovementMode::moveRight) {
                    Vector2 snapTile = {roundf(gridUnitPos[0].x / cellWidth),
                                        gridUnitPos[0].y / cellHeight};
                    turnTile.push_back({snapTile.x, snapTile.y, 0});
                }
                if (currentMode == MovementMode::moveDown) {
                    Vector2 snapTile = {gridUnitPos[0].x / cellWidth,
                                        floorf(gridUnitPos[0].y / cellHeight)};
                    turnTile.push_back({snapTile.x, snapTile.y, 0});
                }
                if (currentMode == MovementMode::moveLeft) {
                    Vector2 snapTile = {floorf(gridUnitPos[0].x / cellWidth),
                                        gridUnitPos[0].y / cellHeight};
                    turnTile.push_back({snapTile.x, snapTile.y, 0});
                }
            }
            // FIX: IMPORTANT: refer to claude chat about +- step/2 and restruct
            for (Vector2 &vec2 : gridUnitPos) {
                for (Vector3 &tileVec3 : turnTile) {
                    // TODO: need to make sure if only check for
                    // the unit that is behind the last turned unit
                    if (vec2.x >= tileVec3.x * cellWidth &&
                        // FIX: currently this looks for a unit
                        // and then proceeds to compare it with every turn tile
                        // NEED: see how many units pased the tile (needs to be tile dependent so
                        // the tile tracks the amount of passed units)
                        // and eg. if passed 0 then compare the unit 0 only with this tile.
                        // another eg. if passed 6 then compare unit 6 to this specific tile.
                        // if tile 0 has eg. 8 passed the other 8 need to target the next tile if
                        // there is one. after it has passed a tile it needs to moveSpeed into the
                        // direction it got targeted by the tile.
                        // need to make loop that loops the moveSpeed for each tile.
                        //
                        // TODO: add: per units direcition
                        currentMode == MovementMode::moveRight) {
                        vec2.x = tileVec3.x * cellWidth;
                        tileVec3.z += 1;
                    }
                    if (vec2.x <= tileVec3.x * cellWidth && currentMode == MovementMode::moveLeft) {
                        vec2.x = tileVec3.x * cellWidth;
                        tileVec3.z += 1;
                    }
                    if (vec2.y >= tileVec3.y * cellHeight && currentMode == MovementMode::moveUp) {
                        vec2.y = tileVec3.y * cellHeight;
                        tileVec3.z += 1;
                    }
                    if (vec2.y <= tileVec3.y * cellHeight &&
                        currentMode == MovementMode::moveDown) {
                        vec2.y = tileVec3.y * cellHeight;
                        tileVec3.z += 1;
                    }
                    if (tileVec3.z == gridUnitPos.size()) {
                        turnTile.erase(turnTile.begin());
                        // FIX:Collect finished tiles and erase after the loop (erase-remove idiom)
                        // basicaly: move out of loop to avoid ub
                    }
                    moveSpeed(vec2, tileMode[0], speed); // FIX: figure out witch and how to use
                }
                moveSpeed(vec2, currentMode, speed); // FIX: same

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
