#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <cmath>
#include <deque>
#include <vector>

enum class MovementMode { moveUp, moveDown, moveLeft, moveRight };
struct GameState {
    MovementMode currentMode = MovementMode::moveRight;
    MovementMode pendingMode = MovementMode::moveRight;

    std::vector<Vector2> pxUnitPos;
    std::deque<Vector2> path;
    Vector2 nextPxTurnHead;
    float speed = 20.0f;
    bool dead = false;
    bool turnPending = false;
};

struct Config {
    int columns = 16.0f;
    int rows = 9.0f;
    Vector2 windowSize = {};
    Vector2 cellSize = {};
    Vector2 unitSize = {};
    Vector2 unitGapFromLines = {};
};

Config makeConfig(const Vector2 &windowSize) {
    Config c;
    c.windowSize = windowSize;
    c.cellSize = {windowSize.x / c.columns, windowSize.y / c.rows};
    c.unitSize = {roundf(c.cellSize.x / 100 * 75), roundf(c.cellSize.y / 100 * 75)};
    c.unitGapFromLines = {(c.cellSize.x - c.unitSize.x) / 2, (c.cellSize.y - c.unitSize.y) / 2};
    return c;
}

struct UiState {
    bool debug = false;
    bool settingsOpen = false;
};

struct FrameState {
    Vector2 cellPosOfHead;
    bool crossed = false;
    Vector2 cellFraction;
};

Vector2 halfCell(const Config &cfg) {
    return {(cfg.windowSize.x / cfg.columns) / 2, (cfg.windowSize.y / cfg.rows) / 2};
}

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

// void moveSpeed(Vector2 &pixelUnitPos, const MovementMode &currentMode, const float &speed) {
void moveSpeed(GameState &gs) {
    const float move = GetFrameTime() * 6.0f * gs.speed;

    switch (gs.currentMode) {
    case MovementMode::moveUp:
        gs.pxUnitPos[0].y -= move;
        break;
    case MovementMode::moveDown:
        gs.pxUnitPos[0].y += move;
        break;
    case MovementMode::moveLeft:
        gs.pxUnitPos[0].x -= move;
        break;
    case MovementMode::moveRight:
        gs.pxUnitPos[0].x += move;
        break;
    }
}

void isOob(GameState &gs, const Config &cfg) {

    if (gs.pxUnitPos[0].x + cfg.unitSize.x + ((cfg.cellSize.x - cfg.unitSize.x) / 2) >
            cfg.windowSize.x ||

        gs.pxUnitPos[0].x + ((cfg.cellSize.x - cfg.unitSize.x) / 2) < 0 ||

        gs.pxUnitPos[0].y + cfg.unitSize.y + ((cfg.cellSize.y - cfg.unitSize.y) / 2) >
            cfg.windowSize.y ||

        gs.pxUnitPos[0].y + ((cfg.cellSize.x - cfg.unitSize.x) / 2) < 0) {
        gs.dead = true;
    }
}
void drawGrid(const Config &cfg) {
    for (int i = 0; i <= cfg.columns; i++) { // FIX: make it draw the missing edge line
        DrawLineV({i * cfg.cellSize.x, 0}, {i * cfg.cellSize.x, cfg.rows * cfg.cellSize.y}, PINK);
    }
    for (int i = 0; i <= cfg.rows; i++) {
        DrawLineV({0, i * cfg.cellSize.y}, {cfg.columns * cfg.cellSize.x, i * cfg.cellSize.y},
                  PINK);
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
void drawDebugInfo(const GameState &gs, const Config &cfg, const FrameState &fs) {
    // FIX: okay... genuenly wtf is this u bummass

    DrawText(
        TextFormat("current:  %s\npending: %s", modeName(gs.currentMode), modeName(gs.pendingMode)),
        50, 50, 18, GREEN);
    DrawText(TextFormat("speed: %f", gs.speed), 50, 100, 18, GREEN);
    DrawText(TextFormat("UnitPos \nX: %f\nY: %f", gs.pxUnitPos[0].x, gs.pxUnitPos[0].y), 50, 150,
             18, GREEN);
    DrawText(TextFormat("cellPos: X = %f\nY = %f\nnewTile: X = %f\nY = %f", fs.cellPosOfHead.x,
                        fs.cellPosOfHead.y, gs.nextPxTurnHead.x, gs.nextPxTurnHead.y),
             50, 250, 18, GREEN);
    DrawText(TextFormat("unit Amount: %i", gs.pxUnitPos.size()), 50, 350, 18, GREEN);

    for ([[maybe_unused]] Vector2 vec2 : gs.pxUnitPos) {
        DrawRectangle(vec2.x, vec2.y, cfg.unitSize.x - (cfg.unitSize.x / 100 * 75),
                      cfg.unitSize.y - (cfg.unitSize.y / 100 * 90), BLUE);
        DrawRectangle(vec2.x, vec2.y, cfg.unitSize.x - (cfg.unitSize.x / 100 * 90),
                      cfg.unitSize.y - (cfg.unitSize.y / 100 * 75), BLUE);
    }
    for (int i = 0; i <= cfg.columns; i++) {
        for (int x = 0; x <= cfg.rows; x++) {
            DrawRectangle(i * cfg.cellSize.x, x * cfg.cellSize.y, 5, 5, SKYBLUE);
        }
    }
}

MovementMode oppositeDir(GameState &gs) {

    if (gs.pendingMode == MovementMode::moveDown) {
        return MovementMode::moveUp;
    } else if (gs.pendingMode == MovementMode::moveUp) {
        return MovementMode::moveDown;
    } else if (gs.pendingMode == MovementMode::moveRight) {
        return MovementMode::moveLeft;
    } else if (gs.pendingMode == MovementMode::moveLeft) {
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

void addUnit(GameState &gs, const Config &cfg) {

    if (IsKeyPressed(KEY_A)) {
        if (gs.currentMode == MovementMode::moveDown) {
            gs.pxUnitPos.push_back({gs.pxUnitPos[gs.pxUnitPos.size() - 1].x,
                                    gs.pxUnitPos[gs.pxUnitPos.size() - 1].y - cfg.cellSize.y});

        } else if (gs.currentMode == MovementMode::moveUp) {
            gs.pxUnitPos.push_back({gs.pxUnitPos[gs.pxUnitPos.size() - 1].x,
                                    gs.pxUnitPos[gs.pxUnitPos.size() - 1].y + cfg.cellSize.y});

        } else if (gs.currentMode == MovementMode::moveRight) {
            gs.pxUnitPos.push_back({gs.pxUnitPos[gs.pxUnitPos.size() - 1].x - cfg.cellSize.x,
                                    gs.pxUnitPos[gs.pxUnitPos.size() - 1].y});

        } else if (gs.currentMode == MovementMode::moveLeft) {
            gs.pxUnitPos.push_back({gs.pxUnitPos[gs.pxUnitPos.size() - 1].x + cfg.cellSize.x,
                                    gs.pxUnitPos[gs.pxUnitPos.size() - 1].y});
        }
    }
}

void keybinds(GameState &gs, const Config &cfg) {

    if (IsKeyPressed(KEY_MINUS)) {
        gs.speed -= 10;
    } else if (IsKeyPressed(KEY_EQUAL)) {
        gs.speed += 10;
    }

    addUnit(gs, cfg);
}

void crossAndTurn(GameState &gs, FrameState &fs) {

    gs.pxUnitPos[0] = gs.nextPxTurnHead;
    gs.path.push_front({gs.nextPxTurnHead.x, gs.nextPxTurnHead.y});
    if (gs.turnPending) {
        gs.currentMode = gs.pendingMode;
        gs.turnPending = false;
    }
    fs.crossed = true;
}

int main(void) {

    Vector2 windowSize = {((1920.0f / 100) * 62.5), ((1080.0f / 100) * 62.5)};

    GameState gs;
    const Config cfg = makeConfig(windowSize);
    UiState uis;
    FrameState fs;

    InitWindow(windowSize.x, windowSize.y, "Raylib Snake");

    SetTargetFPS(60);

    gs.pxUnitPos.push_back(
        {roundf(cfg.columns / 2) * cfg.cellSize.x, roundf(cfg.rows / 2) * cfg.cellSize.y});
    gs.nextPxTurnHead = {gs.pxUnitPos[0].x + cfg.cellSize.x, gs.pxUnitPos[0].y};
    gs.path.push_back(gs.pxUnitPos[0]);

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_O) && !uis.settingsOpen) {
            uis.settingsOpen = true;
        }

        if (IsKeyPressed(KEY_Q)) {
            break;
        }

        keybinds(gs, cfg);

        if (IsWindowFocused() && !gs.dead && !uis.settingsOpen) {

            fs.cellPosOfHead = {
                gs.pxUnitPos[0].x / cfg.cellSize.x,
                gs.pxUnitPos[0].y / cfg.cellSize.y,
            };

            fs.cellFraction = {fs.cellPosOfHead.x - floorf(fs.cellPosOfHead.x),
                               fs.cellPosOfHead.y - floorf(fs.cellPosOfHead.y)};

            if (moveKeys(gs.pendingMode)) {
                gs.turnPending = true;
            }

            if (gs.currentMode != oppositeDir(gs)) {

                if (gs.currentMode == MovementMode::moveRight &&
                    gs.pxUnitPos[0].x >= gs.nextPxTurnHead.x) {
                    crossAndTurn(gs, fs);
                } else if (gs.currentMode == MovementMode::moveLeft &&
                           gs.pxUnitPos[0].x <= gs.nextPxTurnHead.x) {
                    crossAndTurn(gs, fs);
                } else if (gs.currentMode == MovementMode::moveDown &&
                           gs.pxUnitPos[0].y >= gs.nextPxTurnHead.y) {
                    crossAndTurn(gs, fs);
                } else if (gs.currentMode == MovementMode::moveUp &&
                           gs.pxUnitPos[0].y <= gs.nextPxTurnHead.y) {
                    crossAndTurn(gs, fs);
                }

                if (fs.crossed) {
                    const Vector2 advance = step(gs.currentMode, cfg.cellSize);
                    gs.nextPxTurnHead.x += advance.x;
                    gs.nextPxTurnHead.y += advance.y;
                    if (gs.path.size() > gs.pxUnitPos.size()) {
                        gs.path.pop_back();
                    }
                    fs.crossed = false;
                }
            }

            for (float i = 1; i < gs.pxUnitPos.size(); i++) {
                if (gs.currentMode == MovementMode::moveUp) {
                    gs.pxUnitPos[i] = {
                        std::lerp(gs.path[i - 1].x, gs.path[i].x, fs.cellFraction.y),
                        std::lerp(gs.path[i - 1].y, gs.path[i].y, fs.cellFraction.y)};
                } else if (gs.currentMode == MovementMode::moveLeft) {
                    gs.pxUnitPos[i] = {
                        std::lerp(gs.path[i - 1].x, gs.path[i].x, fs.cellFraction.x),
                        std::lerp(gs.path[i - 1].y, gs.path[i].y, fs.cellFraction.x)};
                } else if (gs.currentMode == MovementMode::moveDown) {
                    gs.pxUnitPos[i] = {
                        std::lerp(gs.path[i].x, gs.path[i - 1].x, fs.cellFraction.y),
                        std::lerp(gs.path[i].y, gs.path[i - 1].y, fs.cellFraction.y)};
                } else if (gs.currentMode == MovementMode::moveRight) {
                    gs.pxUnitPos[i] = {
                        std::lerp(gs.path[i].x, gs.path[i - 1].x, fs.cellFraction.x),
                        std::lerp(gs.path[i].y, gs.path[i - 1].y, fs.cellFraction.x)};
                }
            }

            moveSpeed(gs);

            isOob(gs, cfg);

            // TODO: add self collisions
        }

        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(BLACK);
        const Vector2 btn = halfCell(cfg);

        if (uis.settingsOpen) {
            GuiCheckBox((Rectangle){btn.x * 1, btn.y * 1, 50, 50}, "DebugInfo", &uis.debug);

            if (GuiButton((Rectangle){btn.x * 1, btn.y * 16, 150, 50}, "save & exit") ||
                IsKeyPressed(KEY_S)) {
                uis.settingsOpen = false;
            }
        }

        if (!uis.settingsOpen) {
            if (gs.dead) {
                DrawText("skill issue\n", (windowSize.x / 2) - 50, (windowSize.y / 2) - 25, 24,
                         WHITE);

                if (GuiButton((Rectangle){(windowSize.x / 2) - 50, (windowSize.y / 2) - 25 + 250,
                                          100, 50},
                              "restart") ||
                    IsKeyPressed(KEY_R)) {

                    gs = GameState{};

                    // FIX: need to manage:
                    gs.pxUnitPos.clear();
                    gs.pxUnitPos.push_back({roundf(cfg.columns / 2) * cfg.cellSize.x,
                                            roundf(cfg.rows / 2) * cfg.cellSize.y});
                    gs.nextPxTurnHead = {gs.pxUnitPos[0].x + cfg.cellSize.x, gs.pxUnitPos[0].y};
                }

            } else {
                DrawText("press <O> for settings and tutor", btn.x * 24, btn.y * 0.5, 16, WHITE);
                for ([[maybe_unused]] Vector2 vec2 : gs.pxUnitPos) {
                    DrawRectangle(vec2.x + cfg.unitGapFromLines.x, vec2.y + cfg.unitGapFromLines.y,
                                  cfg.unitSize.x, cfg.unitSize.y, RED);
                }
                drawGrid(cfg);

                if (uis.debug) {
                    drawDebugInfo(gs, cfg, fs);
                }
            }
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();

    return 0;
}
