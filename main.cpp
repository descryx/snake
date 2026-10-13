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

void moveSpeed(Vector2 &pixelUnitPos, MovementMode &currentMode, const float &speed) {
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

void addUnit(std::vector<Vector2> &pixelUnitPos, Vector2 unitSize, MovementMode &currentMode,
             Vector2 cellSize) {
    switch (currentMode) {
    case MovementMode::moveUp:
        pixelUnitPos.push_back({pixelUnitPos[0].x, pixelUnitPos[0].y - (unitSize.y + cellSize.y)});
        break; // FIX: spawns 1 pixel off, idk what exaclty or withc

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

int main(void) {

    Vector2 windowSize = {((1920.0f / 100) * 62.5), ((1080.0f / 100) * 62.5)};
    InitWindow(windowSize.x, windowSize.y, "Raylib Snake");

    SetTargetFPS(60);

    // NOTE: window dependent
    float collumns = 16.0f;
    float rows = 9.0f;
    Vector2 cellSize = {windowSize.x / collumns, windowSize.y / rows};
    std::vector<Vector2> pixelUnitPos;
    Vector2 nextCellToTurnHead = {0, 0};
    Vector2 nextCellToTurn = {0, 0};
    Vector2 cellPosOfHead = {0, 0};
    std::deque<Vector2> path = {{0, 0}};
    bool turnedHead = false;
    Vector2 unitFractionPosition = {0, 0};

    // NOTE: initial spawn of 1st head at ~middle of window
    pixelUnitPos.push_back({roundf(collumns / 2) * cellSize.x, roundf(rows / 2) * cellSize.y});

    float speed = 20.0f;
    bool dead = false;
    Vector2 unitSize = {roundf(cellSize.x / 100 * 75), roundf(cellSize.y / 100 * 75)};
    Vector2 unitGapFromLines = {(cellSize.x - unitSize.x) / 2, (cellSize.y - unitSize.y) / 2};

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_Q)) {
            break;
        }

        if (IsKeyPressed(KEY_A)) {
            // addUnit(pixelUnitPos, unitSize, currentMode, cellSize.x, cellSize.y);
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
        if (IsKeyPressed(KEY_MINUS)) {
            speed -= 10;
        } else if (IsKeyPressed(KEY_EQUAL)) {
            speed += 10;
        }

        if (IsWindowFocused() && !dead) {
            cellPosOfHead = {
                pixelUnitPos[0].x / cellSize.x,
                pixelUnitPos[0].y / cellSize.y,
            };

            if (currentMode == MovementMode::moveDown) {
                nextCellToTurn = {cellSize.x * cellPosOfHead.x,
                                  cellSize.y * ceilf(cellPosOfHead.y)};

            } else if (currentMode == MovementMode::moveUp) {
                nextCellToTurn = {cellSize.x * cellPosOfHead.x,
                                  cellSize.y * floorf(cellPosOfHead.y)};

            } else if (currentMode == MovementMode::moveRight) {
                nextCellToTurn = {cellSize.x * ceilf(cellPosOfHead.x),
                                  cellSize.y * cellPosOfHead.y};

            } else if (currentMode == MovementMode::moveLeft) {
                nextCellToTurn = {cellSize.x * floorf(cellPosOfHead.x),
                                  cellSize.y * cellPosOfHead.y};
            }

            if (!turnedHead) {
                nextCellToTurnHead = nextCellToTurn;
            }

            unitFractionPosition = {cellPosOfHead.x - floorf(cellPosOfHead.x),
                                    cellPosOfHead.y - floorf(cellPosOfHead.y)};

            if (moveKeys(pendingMode)) {
                turnedHead = true;
            }

            if (currentMode != oppositeDir(pendingMode)) {
                if (currentMode == MovementMode::moveDown &&
                    pixelUnitPos[0].y > nextCellToTurnHead.y) {
                    pixelUnitPos[0] = nextCellToTurnHead;
                    path.push_front({nextCellToTurn.x, nextCellToTurn.y});
                    if (turnedHead) {
                        currentMode = pendingMode;
                        turnedHead = false;
                    }
                } else if (currentMode == MovementMode::moveUp &&
                           pixelUnitPos[0].y < nextCellToTurnHead.y) {
                    pixelUnitPos[0] = nextCellToTurnHead;
                    path.push_front({nextCellToTurn.x, nextCellToTurn.y});
                    if (turnedHead) {
                        currentMode = pendingMode;
                        turnedHead = false;
                    }
                } else if (currentMode == MovementMode::moveRight &&
                           pixelUnitPos[0].x > nextCellToTurnHead.x) {
                    pixelUnitPos[0] = nextCellToTurnHead;
                    path.push_front({nextCellToTurn.x, nextCellToTurn.y});
                    if (turnedHead) {
                        currentMode = pendingMode;
                        turnedHead = false;
                    }
                } else if (currentMode == MovementMode::moveLeft &&
                           pixelUnitPos[0].x < nextCellToTurnHead.x) {
                    pixelUnitPos[0] = nextCellToTurnHead;
                    path.push_front({nextCellToTurn.x, nextCellToTurn.y});
                    if (turnedHead) {
                        currentMode = pendingMode;
                        turnedHead = false;
                    }
                }
                if (path.size() > pixelUnitPos.size()) {
                    path.pop_back();
                }
            }

            for (float i = 1; i < pixelUnitPos.size(); i++) {
                pixelUnitPos[i] = {std::lerp(path[i].x, path[i - 1].x, unitFractionPosition.x),
                                   std::lerp(path[i].y, path[i - 1].y, unitFractionPosition.y)};
            }

            moveSpeed(pixelUnitPos[0], currentMode, speed);

            is_oob(pixelUnitPos, windowSize, dead,
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
            for ([[maybe_unused]] Vector2 vec2 : pixelUnitPos) {
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
            debugInfo(currentMode, pendingMode, speed, pixelUnitPos, nextCellToTurnHead,
                      cellPosOfHead);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();

    return 0;
}
