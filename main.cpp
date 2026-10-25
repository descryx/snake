#include "raylib.h"
#include <cmath>
#include <cstdlib>
#include <deque>
#include <numbers>
#include <ranges>
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

void moveSpeed(Vector2 &pixelUnitPos, const MovementMode &currentMode, const float &speed) {
    const float move = GetFrameTime() * 6.0f * speed;

    switch (currentMode) {
    case MovementMode::moveUp:
        pixelUnitPos.y -= move;
        break;
    case MovementMode::moveDown:
        pixelUnitPos.y += move;
        break;
    case MovementMode::moveLeft:
        pixelUnitPos.x -= move;
        break;
    case MovementMode::moveRight:
        pixelUnitPos.x += move;
        break;
    }
}

void addUnit(std::vector<Vector2> &pixelUnitPos, const Vector2 &unitSize, MovementMode &currentMode,
             const Vector2 &cellSize) {
    switch (currentMode) {
    case MovementMode::moveUp:
        pixelUnitPos.push_back({pixelUnitPos[0].x, pixelUnitPos[0].y - (unitSize.y + cellSize.y)});
        break;

    case MovementMode::moveDown:
        pixelUnitPos.push_back({pixelUnitPos[0].x, pixelUnitPos[0].y + (unitSize.y + cellSize.y)});
        break;

    case MovementMode::moveLeft:
        pixelUnitPos.push_back({pixelUnitPos[0].x - (unitSize.x + cellSize.x), pixelUnitPos[0].y});
        break;

    case MovementMode::moveRight:
        pixelUnitPos.push_back({pixelUnitPos[0].x + (unitSize.x + cellSize.x), pixelUnitPos[0].y});
        break;
    }
}

void is_oob(const std::vector<Vector2> &pixelUnitPos, const Vector2 &windowSize, bool &dead,
            const Vector2 &unitSize) {
    if (pixelUnitPos[0].x + unitSize.x >
            windowSize.x || // FIX: no fucking idea why the one affect another
        pixelUnitPos[0].x + (unitSize.x / 76 * 100) < 0 ||
        pixelUnitPos[0].y + unitSize.y > windowSize.y ||
        pixelUnitPos[0].y + (unitSize.y / 75 * 100) < 0) {
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
               const std::vector<Vector2> &pixelUnitPos, const Vector2 &targetCell,
               const Vector2 &cellPos) {
    DrawText(TextFormat("current:  %s\npending: %s", modeName(currentMode), modeName(pendingMode)),
             50, 50, 18, GREEN);
    DrawText(TextFormat("speed: %f", speed), 50, 100, 18, GREEN);
    DrawText(TextFormat("UnitPos \nX: %f\nY: %f", pixelUnitPos[0].x, pixelUnitPos[0].y), 50, 150,
             18, GREEN);
    DrawText(TextFormat("cellPos: X = %f\nY = %f\nnewTile: X = %f\nY = %f", cellPos.x, cellPos.y,
                        targetCell.x, targetCell.y),
             50, 250, 18, GREEN);
}

MovementMode oppositeDir(MovementMode &modeToEvaluate) {

    if (modeToEvaluate == MovementMode::moveDown) {
        return MovementMode::moveUp;
    } else if (modeToEvaluate == MovementMode::moveUp) {
        return MovementMode::moveDown;
    } else if (modeToEvaluate == MovementMode::moveRight) {
        return MovementMode::moveLeft;
    } else if (modeToEvaluate == MovementMode::moveLeft) {
        return MovementMode::moveRight;
    }
    return MovementMode::moveRight; // idk if its okay to do so
}

Vector2 step(const MovementMode &m, const Vector2 &cellSize) {
    switch (m) {
    case MovementMode::moveLeft:
        return {-cellSize.x, 0};
    case MovementMode::moveRight:
        return {+cellSize.x, 0};
    case MovementMode::moveUp:
        return {0, -cellSize.y};
    case MovementMode::moveDown:
        return {0, +cellSize.y};
    }
    return {0, 0};
}

void addUnit(std::vector<Vector2> &pixelUnitPos, const Vector2 &cellSize) {

    if (IsKeyPressed(KEY_A)) {
        if (currentMode == MovementMode::moveDown) {
            pixelUnitPos.push_back({pixelUnitPos[pixelUnitPos.size() - 1].x,
                                    pixelUnitPos[pixelUnitPos.size() - 1].y - cellSize.y});

        } else if (currentMode == MovementMode::moveUp) {
            pixelUnitPos.push_back({pixelUnitPos[pixelUnitPos.size() - 1].x,
                                    pixelUnitPos[pixelUnitPos.size() - 1].y + cellSize.y});

        } else if (currentMode == MovementMode::moveRight) {
            pixelUnitPos.push_back({pixelUnitPos[pixelUnitPos.size() - 1].x - cellSize.x,
                                    pixelUnitPos[pixelUnitPos.size() - 1].y});

        } else if (currentMode == MovementMode::moveLeft) {
            pixelUnitPos.push_back({pixelUnitPos[pixelUnitPos.size() - 1].x + cellSize.x,
                                    pixelUnitPos[pixelUnitPos.size() - 1].y});
        }
    }
}

void keybinds(float &speed, std::vector<Vector2> &pixelUnitPos, const Vector2 &cellSize) {
    if (IsKeyPressed(KEY_MINUS)) {
        speed -= 10;
    } else if (IsKeyPressed(KEY_EQUAL)) {
        speed += 10;
    }

    addUnit(pixelUnitPos, cellSize);
}

void crossAndTurn(std::vector<Vector2> &pixelUnitPos, const Vector2 &nextPixelTurnHead,
                  std::deque<Vector2> &path, bool &turnedHead, bool &crossed) {
    pixelUnitPos[0] = nextPixelTurnHead;
    path.push_front({nextPixelTurnHead.x, nextPixelTurnHead.y});
    if (turnedHead) {
        currentMode = pendingMode;
        turnedHead = false;
    }
    crossed = true;
}

int main(void) {

    Vector2 windowSize = {((1920.0f / 100) * 62.5), ((1080.0f / 100) * 62.5)};
    InitWindow(windowSize.x, windowSize.y, "Raylib Snake");

    SetTargetFPS(60);

    // NOTE: window dependent
    float collumns = 16.0f;
    float rows = 9.0f;
    Vector2 cellSize = {windowSize.x / collumns, windowSize.y / rows};
    std::vector<Vector2> pxUnitPos;
    Vector2 nextPxTurnHead;
    Vector2 cellPosOfHead;
    std::deque<Vector2> path;
    bool turnedHead = false;
    bool crossed = false;
    Vector2 unitFractionPosition;

    // NOTE: initial spawn of 1st head at ~middle of window
    pxUnitPos.push_back({roundf(collumns / 2) * cellSize.x, roundf(rows / 2) * cellSize.y});
    nextPxTurnHead = {pxUnitPos[0].x + cellSize.x, pxUnitPos[0].y};
    path.push_back(pxUnitPos[0]);

    float speed = 20.0f;
    bool dead = false;
    Vector2 unitSize = {roundf(cellSize.x / 100 * 75), roundf(cellSize.y / 100 * 75)};
    Vector2 unitGapFromLines = {(cellSize.x - unitSize.x) / 2, (cellSize.y - unitSize.y) / 2};

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_Q)) {
            break;
        }

        keybinds(speed, pxUnitPos, cellSize);

        if (IsWindowFocused() && !dead) {
            cellPosOfHead = {
                pxUnitPos[0].x / cellSize.x,
                pxUnitPos[0].y / cellSize.y,
            };

            unitFractionPosition = {cellPosOfHead.x - floorf(cellPosOfHead.x),
                                    cellPosOfHead.y - floorf(cellPosOfHead.y)};

            if (moveKeys(pendingMode)) {
                turnedHead = true;
            }

            if (currentMode != oppositeDir(pendingMode)) {

                if (currentMode == MovementMode::moveRight && pxUnitPos[0].x >= nextPxTurnHead.x) {
                    crossAndTurn(pxUnitPos, nextPxTurnHead, path, turnedHead, crossed);
                } else if (currentMode == MovementMode::moveLeft &&
                           pxUnitPos[0].x <= nextPxTurnHead.x) {
                    crossAndTurn(pxUnitPos, nextPxTurnHead, path, turnedHead, crossed);
                } else if (currentMode == MovementMode::moveDown &&
                           pxUnitPos[0].y >= nextPxTurnHead.y) {
                    crossAndTurn(pxUnitPos, nextPxTurnHead, path, turnedHead, crossed);
                } else if (currentMode == MovementMode::moveUp &&
                           pxUnitPos[0].y <= nextPxTurnHead.y) {
                    crossAndTurn(pxUnitPos, nextPxTurnHead, path, turnedHead, crossed);
                }

                if (crossed) {
                    const Vector2 advance = step(currentMode, cellSize);
                    nextPxTurnHead.x += advance.x;
                    nextPxTurnHead.y += advance.y;
                    if (path.size() > pxUnitPos.size()) {
                        path.pop_back();
                    }
                    crossed = false;
                }
            }

            // FIX: not moving smooth when dir is not same achsis
            // TODO: smooth turn
            for (float i = 1; i < pxUnitPos.size(); i++) {
                if (currentMode == MovementMode::moveUp) {
                    pxUnitPos[i] = {std::lerp(path[i - 1].x, path[i].x, unitFractionPosition.y),
                                    std::lerp(path[i - 1].y, path[i].y, unitFractionPosition.y)};
                } else if (currentMode == MovementMode::moveLeft) {
                    pxUnitPos[i] = {std::lerp(path[i - 1].x, path[i].x, unitFractionPosition.x),
                                    std::lerp(path[i - 1].y, path[i].y, unitFractionPosition.x)};
                } else if (currentMode == MovementMode::moveDown) {
                    pxUnitPos[i] = {std::lerp(path[i].x, path[i - 1].x, unitFractionPosition.y),
                                    std::lerp(path[i].y, path[i - 1].y, unitFractionPosition.y)};
                } else if (currentMode == MovementMode::moveRight) {
                    pxUnitPos[i] = {std::lerp(path[i].x, path[i - 1].x, unitFractionPosition.x),
                                    std::lerp(path[i].y, path[i - 1].y, unitFractionPosition.x)};
                }
            }

            moveSpeed(pxUnitPos[0], currentMode, speed);

            // is_oob(pixelUnitPos, windowSize, dead,
            //        unitSize); // FIX: one hitbox calc affects another, either one + or -
            //                   // achses work at a time
        }

        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(BLACK);

        if (dead) {
            DrawText("skill issue\n(cool and defenetly appropriatly centered text)",
                     (windowSize.x / 2) - 50, (windowSize.y / 2) - 25, 24, WHITE);

        } else {
            for ([[maybe_unused]] Vector2 vec2 : pxUnitPos) {
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
            debugInfo(currentMode, pendingMode, speed, pxUnitPos, nextPxTurnHead, cellPosOfHead);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();

    return 0;
}
