#include "raylib.h"
#include <cmath>
#include <string>
#include <vector>

enum class MovementMode { moveUp, moveDown, moveLeft, moveRight };
MovementMode currentMode = MovementMode::moveRight;
MovementMode pendingMode = MovementMode::moveRight;

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
void drawGrid(const float &collumns, const float &rows, const float &cellWidth,
              const float &cellHeight) {
    for (int i = 0; i <= collumns; i++) { // FIX: make it draw the missing edge line
        DrawLineV({i * cellWidth, 0}, {i * cellWidth, rows * cellHeight}, PINK);
    }
    for (int i = 0; i <= rows; i++) {
        DrawLineV({0, i * cellHeight}, {collumns * cellWidth, i * cellHeight}, PINK);
    }
}

const char *modeName(MovementMode m) {
    switch (m) {
    case MovementMode::moveUp:
        return "up";
    case MovementMode::moveDown:
        return "down";
    case MovementMode::moveLeft:
        return "left";
    case MovementMode::moveRight:
        return "right";
    }
    return "?";
}
void debugInfo(const MovementMode &currentMode, const MovementMode &pendingMode, const float &speed,
               const std::vector<Vector2> &gridUnitPos, const Vector2 &targetCell,
               const Vector2 &cellPos) {
    DrawText(TextFormat("current:  %s\npending: %s", modeName(currentMode), modeName(pendingMode)),
             50, 50, 18, GREEN);
    DrawText(TextFormat("speed: %f", speed), 50, 100, 18, GREEN);
    DrawText(TextFormat("UnitPos \nX: %f\nY: %f", gridUnitPos[0].x, gridUnitPos[0].y), 50, 150, 18,
             GREEN);
    DrawText(TextFormat("cellPos: X = %f\nY = %f\nnewTile: X = %f\nY = %f", cellPos.x, cellPos.y,
                        targetCell.x, targetCell.y),
             50, 250, 18, GREEN);
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
    Vector2 targetCell = {0, 0};
    Vector2 cellPos = {0, 0};

    // NOTE: initial spawn of 1st head at ~middle of window
    gridUnitPos.push_back({roundf(collumns / 2) * cellWidth, roundf(rows / 2) * cellHeight});

    float speed = 25.0f;
    bool dead = false;
    Vector2 unitSize = {roundf(cellWidth / 100 * 75), roundf(cellHeight / 100 * 75)};

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_Q)) {
            break;
        }

        if (IsKeyPressed(KEY_A)) {
            addUnit(gridUnitPos, unitSize, currentMode); // NOTE: forget about this shit for a while
        }

        if (IsWindowFocused() && !dead) {
            cellPos = {
                gridUnitPos[0].x / cellWidth,
                gridUnitPos[0].y / cellHeight,
            };

            moveKeys(pendingMode);

            if (currentMode != pendingMode) {
                switch (currentMode) {
                case MovementMode::moveDown:
                    cellPos.y += 1;

                case MovementMode::moveUp:
                    cellPos.y -= 1;

                case MovementMode::moveRight:
                    cellPos.x += 1;

                case MovementMode::moveLeft:
                    cellPos.x -= 1;
                }
                targetCell = {cellWidth * (roundf(cellPos.x)), cellHeight * (roundf(cellPos.y))};
                gridUnitPos[0] = {targetCell.x, targetCell.y};
                currentMode = pendingMode;
            }
            moveSpeed(gridUnitPos[0], currentMode, speed);

            // is_oob(gridUnitPos, gridSize, dead); // FIX:
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

                // NOTE: this is the bounding corner the coordinate of the unit
                // debug slop
                DrawRectangle(vec2.x, vec2.y, unitSize.x - (unitSize.x / 100 * 75),
                              unitSize.y - (unitSize.y / 100 * 90), BLUE);
                DrawRectangle(vec2.x, vec2.y, unitSize.x - (unitSize.x / 100 * 90),
                              unitSize.y - (unitSize.y / 100 * 75), BLUE);
            }
            // NOTE: debug slop aswell, bounding coords used for grid
            for (int i = 0; i <= collumns; i++) {
                for (int x = 0; x <= rows; x++) {
                    DrawRectangle(i * cellWidth, x * cellHeight, 5, 5, SKYBLUE);
                }
            }

            drawGrid(collumns, rows, cellWidth, cellHeight);
            debugInfo(currentMode, pendingMode, speed, gridUnitPos, targetCell, cellPos);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();

    return 0;
}
