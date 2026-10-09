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

void addUnit(std::vector<Vector2> &gridUnitPos, Vector2 unitSize, MovementMode &currentMode,
             Vector2 cellSize) {
    switch (currentMode) {
    case MovementMode::moveUp:
        gridUnitPos.push_back({gridUnitPos[0].x, gridUnitPos[0].y - (unitSize.y + cellSize.y)});
        break;
        // TODO: need to make so spawn 1 behind last unit or
        //

    case MovementMode::moveDown:
        gridUnitPos.push_back({gridUnitPos[0].x, gridUnitPos[0].y + (unitSize.y + cellSize.y)});
        break;

    case MovementMode::moveLeft:
        gridUnitPos.push_back({gridUnitPos[0].x - (unitSize.x + cellSize.x), gridUnitPos[0].y});
        break;

    case MovementMode::moveRight:
        gridUnitPos.push_back({gridUnitPos[0].x + (unitSize.x + cellSize.x), gridUnitPos[0].y});
        break;
    }
}

void is_oob(const std::vector<Vector2> &gridUnitPos, const Vector2 &windowSize, bool &dead,
            const Vector2 &unitSize) {
    if (gridUnitPos[0].x + unitSize.x >
            windowSize.x || // FIX: no fucking idea why the one affect another
        gridUnitPos[0].x + (unitSize.x / 76 * 100) < 0 ||
        gridUnitPos[0].y + unitSize.y > windowSize.y ||
        gridUnitPos[0].y + (unitSize.y / 75 * 100) < 0) {
        dead = true;
    }
}
void drawGrid(const float &collumns, const float &rows, const Vector2 cellSize) {
    for (int i = 0; i <= collumns; i++) { // FIX: make it draw the missing edge line
        DrawLineV({i * cellSize.x, 0}, {i * cellSize.x, rows * cellSize.y}, PINK);
    }
    for (int i = 0; i <= rows; i++) {
        DrawLineV({0, i * cellSize.y}, {collumns * cellSize.x, i * cellSize.y}, PINK);
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
    float collumns = 16.0f;
    float rows = 9.0f;
    Vector2 cellSize = {windowSize.x / collumns, windowSize.y / rows};
    std::vector<Vector2> gridUnitPos;
    Vector2 turnCell = {0, 0};
    Vector2 cellPos = {0, 0};
    bool turned = true;

    // NOTE: initial spawn of 1st head at ~middle of window
    gridUnitPos.push_back({roundf(collumns / 2) * cellSize.x, roundf(rows / 2) * cellSize.y});

    float speed = 10.0f;
    bool dead = false;
    Vector2 unitSize = {roundf(cellSize.x / 100 * 75), roundf(cellSize.y / 100 * 75)};
    Vector2 unitGapFromLines = {(cellSize.x - unitSize.x) / 2, (cellSize.y - unitSize.y) / 2};

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_Q)) {
            break;
        }

        if (IsKeyPressed(KEY_A)) {
            // addUnit(gridUnitPos, unitSize, currentMode, cellSize.x, cellSize.y);
            if (currentMode == MovementMode::moveDown) {
                gridUnitPos.push_back({gridUnitPos[gridUnitPos.size() - 1].x,
                                       gridUnitPos[gridUnitPos.size() - 1].y - cellSize.y});

            } else if (currentMode == MovementMode::moveUp) {
                gridUnitPos.push_back({gridUnitPos[gridUnitPos.size() - 1].x,
                                       gridUnitPos[gridUnitPos.size() - 1].y + cellSize.y});

            } else if (currentMode == MovementMode::moveRight) {
                gridUnitPos.push_back({gridUnitPos[gridUnitPos.size() - 1].x - cellSize.x,
                                       gridUnitPos[gridUnitPos.size() - 1].y});

            } else if (currentMode == MovementMode::moveLeft) {
                gridUnitPos.push_back({gridUnitPos[gridUnitPos.size() - 1].x + cellSize.x,
                                       gridUnitPos[gridUnitPos.size() - 1].y});
            }
        }
        if (IsKeyPressed(KEY_MINUS)) {
            speed -= 10;
        } else if (IsKeyPressed(KEY_EQUAL)) {
            speed += 10;
        }

        if (IsWindowFocused() && !dead) {
            if (turned) {
                cellPos = {
                    gridUnitPos[0].x / cellSize.x,
                    gridUnitPos[0].y / cellSize.y,
                };
                moveKeys(pendingMode); // FIX: currently skips all input after first
                                       // registered inpt until turned
            }

            if (currentMode != pendingMode) {
                turned = false;
                if (currentMode == MovementMode::moveDown) {
                    turnCell = {cellSize.x * cellPos.x, cellSize.y * ceilf(cellPos.y)};
                    if (gridUnitPos[0].y > turnCell.y) {
                        gridUnitPos[0] = {turnCell.x, turnCell.y};
                        turned = true;
                        currentMode = pendingMode;
                    }
                } else if (currentMode == MovementMode::moveUp) {
                    turnCell = {cellSize.x * cellPos.x, cellSize.y * floorf(cellPos.y)};
                    if (gridUnitPos[0].y < turnCell.y) {
                        gridUnitPos[0] = {turnCell.x, turnCell.y};
                        turned = true;
                        currentMode = pendingMode;
                    }
                } else if (currentMode == MovementMode::moveRight) {
                    turnCell = {cellSize.x * ceilf(cellPos.x), cellSize.y * cellPos.y};
                    if (gridUnitPos[0].x > turnCell.x) {
                        gridUnitPos[0] = {turnCell.x, turnCell.y};
                        turned = true;
                        currentMode = pendingMode;
                    }
                } else if (currentMode == MovementMode::moveLeft) {
                    turnCell = {cellSize.x * floorf(cellPos.x), cellSize.y * cellPos.y};
                    if (gridUnitPos[0].x < turnCell.x) {
                        gridUnitPos[0] = {turnCell.x, turnCell.y};
                        turned = true;
                        currentMode = pendingMode;
                    }
                }
            }

            for (Vector2 &vec2 : gridUnitPos) {
                moveSpeed(vec2, currentMode, speed);
            }

            is_oob(gridUnitPos, windowSize, dead,
                   unitSize); // FIX: one hitbox calc affects another, either one + or -
                              // achses work at a time
        }

        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(BLACK);

        if (dead) {
            DrawText("skill issue\n(cool and defenetly appropriatly centered text)",
                     (windowSize.x / 2) - 50, (windowSize.y / 2) - 25, 24, WHITE);

        } else {
            for ([[maybe_unused]] Vector2 vec2 : gridUnitPos) {
                DrawRectangle(vec2.x + unitGapFromLines.x, vec2.y + unitGapFromLines.y, unitSize.x,
                              unitSize.y, RED);

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
                    DrawRectangle(i * cellSize.x, x * cellSize.y, 5, 5, SKYBLUE);
                }
            }

            drawGrid(collumns, rows, cellSize);
            debugInfo(currentMode, pendingMode, speed, gridUnitPos, turnCell, cellPos);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();

    return 0;
}
