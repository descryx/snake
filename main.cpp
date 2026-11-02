#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
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
    int randomX = 3;
    int randomY = 0;
    bool ateApple = false;
    bool pause = false;
};

struct Config {
    int columns = 9.0f;
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
    bool debug = true;
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

    if (!CheckCollisionPointRec({gs.pxUnitPos[0].x + cfg.unitGapFromLines.x,
                                 gs.pxUnitPos[0].y + cfg.unitGapFromLines.y},
                                (Rectangle){0, 0, cfg.windowSize.x, cfg.windowSize.y})) {

        gs.dead = true;
    } else if (!CheckCollisionPointRec(
                   {gs.pxUnitPos[0].x + cfg.unitGapFromLines.x + cfg.unitSize.x,
                    gs.pxUnitPos[0].y + cfg.unitGapFromLines.y + cfg.unitSize.y},
                   (Rectangle){0, 0, cfg.windowSize.x, cfg.windowSize.y})) {

        gs.dead = true;
    }
}
void drawGrid(const Config &cfg) {
    for (int i = 0; i <= cfg.columns; i++) { // FIX: make it draw the missing edge line
        DrawLineV({i * cfg.cellSize.x, 0}, {i * cfg.cellSize.x, cfg.rows * cfg.cellSize.y}, GRAY);
    }
    DrawLineV({1, 0}, {1, cfg.windowSize.y}, GRAY);

    for (int i = 0; i <= cfg.rows; i++) {
        DrawLineV({0, i * cfg.cellSize.y}, {cfg.columns * cfg.cellSize.x, i * cfg.cellSize.y},
                  GRAY);
    }
    DrawLineV({0, cfg.windowSize.y - 1}, {cfg.windowSize.x, cfg.windowSize.y - 1}, GRAY);
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

    DrawText(
        TextFormat("current:  %s\npending: %s", modeName(gs.currentMode), modeName(gs.pendingMode)),
        50, 50, 18, WHITE);

    DrawText(TextFormat("speed: %f", gs.speed), 50, 100, 18, WHITE);

    DrawText(TextFormat("UnitPos \nX: %f\nY: %f", gs.pxUnitPos[0].x, gs.pxUnitPos[0].y), 50, 150,
             18, WHITE);

    DrawText(TextFormat("cellPos: X = %f\nY = %f\nnewTile: X = %f\nY = %f", fs.cellPosOfHead.x,
                        fs.cellPosOfHead.y, gs.nextPxTurnHead.x, gs.nextPxTurnHead.y),
             50, 250, 18, WHITE);

    DrawText(TextFormat("unit Amount: %i", gs.pxUnitPos.size()), 50, 350, 18, WHITE);

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

    DrawText(TextFormat("Apple Cell pos\nX: %i\nY: %i", gs.randomX, gs.randomY), 50, 400, 18,
             WHITE);
    // TODO: add for loop and declare beforehand waht params to draw.
    // then loop thru them
    //
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

void addUnit(GameState &gs, const Config &cfg) {

    // FIX: need path init to avoid 00 spawns on KEY_A
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

void keybinds(GameState &gs, const Config &cfg) {

    if (IsKeyPressed(KEY_MINUS)) {
        gs.speed -= 10;
    } else if (IsKeyPressed(KEY_EQUAL)) {
        gs.speed += 10;
    }

    if (IsKeyPressed(KEY_A)) {
        addUnit(gs, cfg);
    }
}

void crossAndTurn(GameState &gs, FrameState &fs) {

    gs.pxUnitPos[0] = gs.nextPxTurnHead;
    gs.path.push_front({gs.nextPxTurnHead.x, gs.nextPxTurnHead.y});
    if (gs.turnPending && gs.currentMode != oppositeDir(gs.pendingMode)) {
        gs.currentMode = gs.pendingMode;
        gs.turnPending = false;
    }
    fs.crossed = true;
}

void appleSpwnNdEat(GameState &gs, const Config &cfg) {

    if (floor(gs.pxUnitPos[0].x / cfg.cellSize.x) == gs.randomX &&
        floor(gs.pxUnitPos[0].y / cfg.cellSize.y) == gs.randomY) {

        gs.randomX = rand() % cfg.columns;
        gs.randomY = rand() % cfg.rows;

        addUnit(gs, cfg);
        gs.ateApple = true;
    }
    if (gs.pxUnitPos.size() % 2 == 0 && gs.ateApple) {
        gs.speed += 5;
        gs.ateApple = false;
    }
}

void checkSelfCollision(GameState &gs, const Config &cfg) {

    for (size_t i = 4; i < gs.pxUnitPos.size(); i++) {

        if (CheckCollisionRecs((Rectangle){gs.pxUnitPos[0].x + cfg.unitGapFromLines.x,
                                           gs.pxUnitPos[0].y + cfg.unitGapFromLines.y,
                                           cfg.unitSize.x, cfg.unitSize.y},
                               (Rectangle){gs.pxUnitPos[i].x + cfg.unitGapFromLines.x,
                                           gs.pxUnitPos[i].y + cfg.unitGapFromLines.y,
                                           cfg.unitSize.x, cfg.unitSize.y})) {
            gs.dead = true;
        }
    }
}

void setup(GameState &gs, const Config &cfg) {

    gs = GameState{};

    // FIX: need to manage:
    // maybe add bool for them to run at start
    gs.randomX += floor(cfg.columns / 2.0f);
    gs.randomY += floor(cfg.rows / 2.0f);

    gs.pxUnitPos.clear();
    gs.pxUnitPos.push_back(
        {floor(cfg.columns / 2.0f) * cfg.cellSize.x, floor(cfg.rows / 2.0f) * cfg.cellSize.y});
    gs.nextPxTurnHead = {gs.pxUnitPos[0].x + cfg.cellSize.x, gs.pxUnitPos[0].y};
}

int main(void) {

    // Vector2 windowSize = {((1920.0f / 100) * 62.5), ((1080.0f / 100) * 62.5)};
    Vector2 windowSize = {720, 720};

    GameState gs;
    const Config cfg = makeConfig(windowSize);
    UiState uis;
    FrameState fs;

    InitWindow(windowSize.x, windowSize.y, "Raylib Snake");

    SetTargetFPS(60);

    setup(gs, cfg);
    srand(time(0));

    // gs.path.push_back(gs.pxUnitPos[0]);

    // TODO: playfield
    //  int dropdownBoxActive = 0;
    //  bool dropDownEditMode = false;

    while (true) {

        // TODO: make playfield size adjustable (choose from)
        //
        //  if (dropdownBoxActive == 0) {
        //      cfg = makeConfig(windowSize);
        //  } else if (dropdownBoxActive == 1) {
        //      windowSize.x *= 2;
        //      windowSize.y *= 2;
        //      cfg = makeConfig(windowSize);
        //  }

        if (IsKeyPressed(KEY_O) && !uis.settingsOpen) {
            uis.settingsOpen = true;
        }

        if (IsKeyPressed(KEY_P)) {
            if (!gs.pause) {
                gs.pause = true;
            } else {
                gs.pause = false;
            }
        }

        if (IsKeyPressed(KEY_Q)) {
            break;
        }

        keybinds(gs, cfg);

        if (IsWindowFocused() && !gs.dead && !uis.settingsOpen && !gs.pause) {
            fs.cellPosOfHead = {
                gs.pxUnitPos[0].x / cfg.cellSize.x,
                gs.pxUnitPos[0].y / cfg.cellSize.y,
            };

            fs.cellFraction = {fs.cellPosOfHead.x - floorf(fs.cellPosOfHead.x),
                               fs.cellPosOfHead.y - floorf(fs.cellPosOfHead.y)};

            if (!gs.turnPending && moveKeys(gs.pendingMode)) {
                gs.turnPending = true;
            }

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
                appleSpwnNdEat(gs, cfg);
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

            checkSelfCollision(gs, cfg);
        }

        //----------------------------------------------------------------------------------
        BeginDrawing();
        Color myTintSettings = {0, 0, 0, 128};
        Color myTintDead = {0, 0, 0, 170};
        Color coinBg = {205, 170, 0, 255};
        Color coinFg = {255, 205, 0, 255};
        // TODO: add connected snake body. make body connected from head to tail.

        ClearBackground(BLACK);
        const Vector2 btn = halfCell(cfg);

        drawGrid(cfg);
        DrawText("press <O> for settings and tutor", btn.x * 24, btn.y * 0.5, 16, WHITE);
        for ([[maybe_unused]] Vector2 vec2 : gs.pxUnitPos) {
            DrawRectangleRounded((Rectangle){vec2.x + cfg.unitGapFromLines.x,
                                             vec2.y + cfg.unitGapFromLines.y, cfg.unitSize.x,
                                             cfg.unitSize.y},
                                 0.4f, 100, RED);
        }
        DrawRectangleRounded(
            (Rectangle){gs.pxUnitPos[0].x + cfg.unitGapFromLines.x + (cfg.unitSize.x / 5),
                        gs.pxUnitPos[0].y + cfg.unitGapFromLines.y + (cfg.unitSize.y / 5),
                        cfg.unitSize.x / 5, cfg.unitSize.y / 5},
            0.4f, 100, GREEN);
        DrawRectangleRounded(
            (Rectangle){gs.pxUnitPos[0].x + cfg.unitGapFromLines.x + (cfg.unitSize.x / 5) * 3,
                        gs.pxUnitPos[0].y + cfg.unitGapFromLines.y + (cfg.unitSize.y / 5),
                        cfg.unitSize.x / 5, cfg.unitSize.y / 5},
            0.4f, 100, GREEN);

        // TODO: smile :)
        //  Vector2 splinePoints[3] = {
        //      // start
        //      {gs.pxUnitPos[0].x + cfg.unitGapFromLines.x,
        //       gs.pxUnitPos[0].y + cfg.unitGapFromLines.y},
        //
        //      // mid
        //      {gs.pxUnitPos[0].x + cfg.cellSize.x / 2, gs.pxUnitPos[0].y + cfg.cellSize.y / 2},
        //
        //      // end
        //      {gs.pxUnitPos[0].x + cfg.unitGapFromLines.x + cfg.cellSize.x,
        //       gs.pxUnitPos[0].y + cfg.unitGapFromLines.y},
        //  };
        // DrawSplineBezierQuadratic(splinePoints, 3, 4.0f, ORANGE);

        DrawCircle(gs.randomX * cfg.cellSize.x + btn.x, gs.randomY * cfg.cellSize.y + btn.y,
                   btn.x - cfg.unitGapFromLines.x, coinBg);
        DrawCircleLinesEx(
            {gs.randomX * cfg.cellSize.x + btn.x, gs.randomY * cfg.cellSize.y + btn.y},
            btn.x - cfg.unitGapFromLines.x, 5, coinFg);

        if (uis.debug) {
            drawDebugInfo(gs, cfg, fs);
        }
        if (uis.settingsOpen) {
            DrawRectangle(0, 0, cfg.windowSize.x, cfg.windowSize.y, myTintSettings);

            DrawRectangle(btn.x * 1, btn.y * 1, 150, 50, BLACK);
            GuiCheckBox((Rectangle){btn.x * 1, btn.y * 1, 50, 50}, "DebugInfo", &uis.debug);

            // TODO: playfield
            //  if (GuiDropdownBox((Rectangle){btn.x * 1, btn.y * 3, 100, 50},
            //  "ONE;TWO;THREE;FOUR",
            //                     &dropdownBoxActive, dropDownEditMode))
            //      dropDownEditMode = !dropDownEditMode;

            if (GuiButton((Rectangle){btn.x * 1, btn.y * 16, 150, 50}, "save & exit") ||
                IsKeyPressed(KEY_S) || IsKeyPressed(KEY_CAPS_LOCK)) {
                uis.settingsOpen = false;
            }
        }
        if (gs.dead) {
            DrawText("skill issue\n", (windowSize.x / 2) - 50, (windowSize.y / 2) - 25, 24, WHITE);
            DrawRectangle(0, 0, cfg.windowSize.x, cfg.windowSize.y, myTintDead);

            if (GuiButton(
                    (Rectangle){(windowSize.x / 2) - 50, (windowSize.y / 2) - 25 + 250, 100, 50},
                    "restart") ||
                IsKeyPressed(KEY_R)) {
                setup(gs, cfg);
            }
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();

    return 0;
}
