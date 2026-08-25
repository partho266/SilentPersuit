#include "iGraphics.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =========================================================
// SILENT PURSUIT
// =========================================================

enum GameState
{
    LOADING,
    MENU,
    LEVEL_SELECT,
    OPTIONS,
    CREDITS,
    GAMEPLAY,
    GAME_OVER
};

GameState state = LOADING;


// =========================================================
// WINDOW
// =========================================================

const int WIN_W = 800;
const int WIN_H = 600;


// =========================================================
// LOADING & SCENES
// =========================================================

int loadingProgress = 0;
bool loadingDone = false;
double animTime = 0.0;

unsigned int loadingBgTexture = 0;
unsigned int menuBgTexture = 0;
unsigned int level1BgTexture = 0;
unsigned int level1Scene2BgTexture = 0;
unsigned int level1Scene3BgTexture = 0;

bool landmineActive = true;
bool landmineTriggered = false;
bool killedByMine = false;
double landmineTimer = 0.0;

const double SWITCH_X = 1450.0; // Point where progression is blocked if Enemy 2 is alive
const double MINE_X = 1520.0;   // Point where the landmine triggers


// =========================================================
// CONTINUOUS WORLD / SCROLLING CAMERA
// =========================================================

const double WORLD_WIDTH = WIN_W * 2.0;

double cameraX = 0.0;
const double CAMERA_FOLLOW = 0.12;

double worldToScreen(double worldX)
{
    return worldX - cameraX;
}


// =========================================================
// COLORS
// =========================================================

const int CLR_BTN_TEXT_R = 200;
const int CLR_BTN_TEXT_G = 200;
const int CLR_BTN_TEXT_B = 200;

const int CLR_GLASS_R = 190;
const int CLR_GLASS_G = 115;
const int CLR_GLASS_B = 40;

const int CLR_TEXT_R = 250;
const int CLR_TEXT_G = 235;
const int CLR_TEXT_B = 210;


// =========================================================
// BUTTONS
// =========================================================

const int BTN_LEFT_X = 50;
const int BTN_RIGHT_MARGIN = 25;

const int BTN_W = 170;
const int BTN_H = 50;

const int BTN_X = WIN_W - BTN_RIGHT_MARGIN - BTN_W;


struct Button
{
    int x;
    int y;
    int w;
    int h;

    char label[30];

    bool hovered;
    bool locked;
};


Button btnNewGame =
{
    BTN_X, 380, BTN_W, BTN_H,
    "New Game", false, false
};

Button btnOptions =
{
    BTN_X, 310, BTN_W, BTN_H,
    "Options", false, false
};

Button btnCredits =
{
    BTN_X, 240, BTN_W, BTN_H,
    "Credits", false, false
};

Button btnQuit =
{
    BTN_X, 170, BTN_W, BTN_H,
    "Quit Game", false, false
};

Button btnAudio =
{
    BTN_LEFT_X, 380, BTN_W, BTN_H,
    "Audio", false, false
};

Button btnControls =
{
    BTN_LEFT_X, 310, BTN_W, BTN_H,
    "Controls", false, false
};

Button btnOptBack =
{
    BTN_LEFT_X, 240, BTN_W, BTN_H,
    "Back", false, false
};

Button btnCredBack =
{
    BTN_LEFT_X, 80, BTN_W, BTN_H,
    "Back", false, false
};

const int GO_BTN_W = 220;
const int GO_BTN_H = 56;

Button btnRetry =
{
    WIN_W / 2 - GO_BTN_W / 2, 260, GO_BTN_W, GO_BTN_H,
    "Retry", false, false
};

Button btnGoToMenu =
{
    WIN_W / 2 - GO_BTN_W / 2, 190, GO_BTN_W, GO_BTN_H,
    "Main Menu", false, false
};

// ---- LEVEL SELECT BUTTONS ----
const int LVL_BTN_W = 220;
const int LVL_BTN_H = 50;
const int LVL_BTN_X = WIN_W / 2 - LVL_BTN_W / 2;

Button btnLevel1 = { LVL_BTN_X, 400, LVL_BTN_W, LVL_BTN_H, "Level 1", false, false };
Button btnLevel2 = { LVL_BTN_X, 330, LVL_BTN_W, LVL_BTN_H, "Level 2 (Locked)", false, true };
Button btnLevel3 = { LVL_BTN_X, 260, LVL_BTN_W, LVL_BTN_H, "Level 3 (Locked)", false, true };
Button btnLevel4 = { LVL_BTN_X, 190, LVL_BTN_W, LVL_BTN_H, "Level 4 (Locked)", false, true };
Button btnLevel5 = { LVL_BTN_X, 120, LVL_BTN_W, LVL_BTN_H, "Level 5 (Locked)", false, true };
Button btnLvlBack = { BTN_LEFT_X, 80, BTN_W, BTN_H, "Back", false, false };

// =========================================================
// BUTTON HIT TEST
// =========================================================

bool isInside(Button b, int mx, int my)
{
    return
        (
            mx >= b.x &&
            mx <= b.x + b.w &&
            my >= b.y &&
            my <= b.y + b.h
            );
}


// =========================================================
// MOUSE COORDINATE CONVERSION
// =========================================================

void toLogicalCoords(int mx, int my, int& lx, int& ly)
{
    int actualW = glutGet(GLUT_WINDOW_WIDTH);
    int actualH = glutGet(GLUT_WINDOW_HEIGHT);

    if (actualW <= 0)
        actualW = WIN_W;

    if (actualH <= 0)
        actualH = WIN_H;

    lx = (int)((double)mx * WIN_W / actualW);

    ly = (int)
        (
            (double)(actualH - (WIN_H - my))
            * WIN_H
            / actualH
            );
}


// =========================================================
// AIM CROSSHAIR (mouse-controlled)
// =========================================================

int crosshairX = WIN_W / 2;
int crosshairY = WIN_H / 2;

void drawCrosshair()
{
    iSetColor(0, 255, 200);
    iCircle(crosshairX, crosshairY, 6);
    iLine(crosshairX - 10, crosshairY, crosshairX + 10, crosshairY);
    iLine(crosshairX, crosshairY - 10, crosshairX, crosshairY + 10);
}


// =========================================================
// DRAW BUTTON
// =========================================================

const double BTN_CLIP = 12.0;

void drawButton(Button b)
{
    double c = BTN_CLIP;

    double vx[8] =
    {
        c,
        b.w - c,
        b.w,
        b.w,
        b.w - c,
        c,
        0,
        0
    };

    double vy[8] =
    {
        0,
        0,
        c,
        b.h - c,
        b.h,
        b.h,
        b.h - c,
        c
    };

    double px[8];
    double py[8];

    for (int i = 0; i < 8; i++)
    {
        px[i] = b.x + vx[i];
        py[i] = b.y + vy[i];
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float alpha;
    if (b.hovered && !b.locked) alpha = 0.65f; else alpha = 0.50f;

    glColor4f(0.10f, 0.07f, 0.05f, alpha);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 8; i++)
        glVertex2d(px[i], py[i]);
    glEnd();

    glDisable(GL_BLEND);

    if (b.locked) {
        iSetColor(80, 80, 80);
    }
    else {
        iSetColor(CLR_GLASS_R, CLR_GLASS_G, CLR_GLASS_B);
    }
    iPolygon(px, py, 8);

    if (b.locked) {
        iSetColor(100, 100, 100);
    }
    else {
        iSetColor((int)(CLR_GLASS_R * 0.85), (int)(CLR_GLASS_G * 0.85), (int)(CLR_GLASS_B * 0.85));
    }
    iLine(b.x + (int)c + 4, b.y + 5, b.x + b.w - (int)c - 4, b.y + 5);
    iLine(b.x + (int)c + 4, b.y + b.h - 5, b.x + b.w - (int)c - 4, b.y + b.h - 5);

    int textX = b.x + (int)c + 14;
    int textY = b.y + b.h / 2 - 6;

    if (b.locked) {
        iSetColor(120, 120, 120);
    }
    else {
        iSetColor(230, 210, 175);
    }
    iText(textX, textY, b.label, GLUT_BITMAP_HELVETICA_18);
}


// =========================================================
// LEVEL 1 (world-space coordinates)
// =========================================================

const double GRAVITY = 0.6;

const double GROUND_Y = 100.0;
const double GROUND_Y_SCENE2 = 155.0;

const double GAP_START_X = 340.0;
const double GAP_END_X = 470.0;

bool isOverGap(double worldX)
{
    return (worldX > GAP_START_X && worldX < GAP_END_X);
}

const double FALL_DEATH_Y = -250.0;

const double OBSTACLE_X0 = WIN_W + 100.0;
const double OBSTACLE_X1 = WIN_W + 300.0;
const double OBSTACLE_TOP_Y = GROUND_Y_SCENE2 + 95.0;

// Flat plateau strip on top of the rock, wide enough for two enemies
const double OBSTACLE_PLATEAU_MARGIN = 22.0;

double groundYAt(double worldX)
{
    const double blendStart = WIN_W - 40.0;
    const double blendEnd = WIN_W + 40.0;

    if (worldX <= blendStart) return GROUND_Y;
    if (worldX >= blendEnd) return GROUND_Y_SCENE2;

    double t = (worldX - blendStart) / (blendEnd - blendStart);
    return GROUND_Y + t * (GROUND_Y_SCENE2 - GROUND_Y);
}

int sceneAt(double worldX)
{
    return (worldX >= WIN_W) ? 2 : 1;
}


// =========================================================
// WALKING SPRITES & IDLE
// =========================================================

unsigned int idleFrame; // Standing still default

const int WALK_FRAME_COUNT = 12;
unsigned int walkRightFrames[WALK_FRAME_COUNT];

const int WALK_GUN_FRAME_COUNT = 4;
unsigned int walkGunFrames[WALK_GUN_FRAME_COUNT];

const int SPRITE_DRAW_W = 140;
const int SPRITE_DRAW_H = 260;

const double SPRITE_Y_OFFSET = 8.0;

const double WALK_FRAME_DURATION = 0.08;


// =========================================================
// JUMPING SPRITES
// =========================================================

const int JUMP_FRAME_COUNT = 9;
unsigned int jumpFrames[JUMP_FRAME_COUNT];

const double JUMP_FRAME_DURATION = 0.19;

// ---- Jumping WITH the gun equipped - separate 6-frame sequence ----
const int JUMP_GUN_FRAME_COUNT = 5;
unsigned int jumpGunFrames[JUMP_GUN_FRAME_COUNT];

const int JUMP_GUN_DRAW_H = 260;

const int JUMP_GUN_DRAW_W[JUMP_GUN_FRAME_COUNT] =
{
    130,
    130,
    130,
    130,
    130
};


// =========================================================
// GUN-POINTING SPRITES (mouse-aimed)
// =========================================================

const int GUN_FRAME_COUNT = 7;
unsigned int gunFrames[GUN_FRAME_COUNT];

const int GUN_DOWN3 = 0;
const int GUN_DOWN2 = 1;
const int GUN_DOWN1 = 2;
const int GUN_NEUTRAL = 3;
const int GUN_UP1 = 4;
const int GUN_UP2 = 5;
const int GUN_UP3 = 6;

const int GUN_SPRITE_DRAW_W = 140;
const int GUN_SPRITE_DRAW_H = 260;
const double GUN_SPRITE_Y_OFFSET = 8.0;

const double AIM_DEADZONE = 18.0;
const double AIM_TIER1 = 70.0;
const double AIM_TIER2 = 160.0;

bool gunEquipped = false;
bool gKeyWasHeld = false;

const double GUN_SRC_W = 446.0;
const double GUN_SRC_H = 560.0;

struct NozzleOffset { double dx; double dy; };

NozzleOffset gunNozzle[GUN_FRAME_COUNT] =
{
    { 445.0 / GUN_SRC_W * GUN_SPRITE_DRAW_W, (GUN_SRC_H - 222.0) / GUN_SRC_H * GUN_SPRITE_DRAW_H }, // DOWN3
    { 441.0 / GUN_SRC_W * GUN_SPRITE_DRAW_W, (GUN_SRC_H - 203.0) / GUN_SRC_H * GUN_SPRITE_DRAW_H }, // DOWN2
    { 445.0 / GUN_SRC_W * GUN_SPRITE_DRAW_W, (GUN_SRC_H - 173.0) / GUN_SRC_H * GUN_SPRITE_DRAW_H }, // DOWN1
    { 431.0 / GUN_SRC_W * GUN_SPRITE_DRAW_W, (GUN_SRC_H - 131.0) / GUN_SRC_H * GUN_SPRITE_DRAW_H }, // NEUTRAL
    { 417.0 / GUN_SRC_W * GUN_SPRITE_DRAW_W, (GUN_SRC_H - 150.0) / GUN_SRC_H * GUN_SPRITE_DRAW_H }, // UP1
    { 397.0 / GUN_SRC_W * GUN_SPRITE_DRAW_W, (GUN_SRC_H - 101.0) / GUN_SRC_H * GUN_SPRITE_DRAW_H }, // UP2
    { 390.0 / GUN_SRC_W * GUN_SPRITE_DRAW_W, (GUN_SRC_H - 65.0) / GUN_SRC_H * GUN_SPRITE_DRAW_H }, // UP3
};


// =========================================================
// FIRING (player)
// =========================================================

bool isFiring = false;
double muzzleFlashTimer = 0.0;
const double MUZZLE_FLASH_DURATION = 0.06;

const double FIRE_COOLDOWN = 0.5;
double fireCooldownTimer = 0.0;

int lastGunPoseIdx = GUN_NEUTRAL;

double playerRecoilTimer = 0.0;
const double PLAYER_RECOIL_DURATION = 0.1;
const double PLAYER_RECOIL_PIXELS = 6.0;


// =========================================================
// PLAYER HEALTH
// =========================================================

const int PLAYER_MAX_HP = 200;
int playerHP = PLAYER_MAX_HP;
const int PLAYER_DAMAGE_PER_HIT = 10;


// =========================================================
// PLAYER (x/y are WORLD coordinates)
// =========================================================

struct Player
{
    double x;
    double y;

    double dx;
    double dy;

    bool isJumping;

    int width;
    int height;

    bool facingRight;

    int walkFrame;
    double walkFrameTimer;

    int jumpFrame;
    double jumpFrameTimer;

    double jumpDx;
};

Player assassin =
{
    200.0, GROUND_Y,
    0.0, 0.0,
    false,
    30, 60,
    true,
    0, 0.0,
    0, 0.0,
    0.0
};


// =========================================================
// ENEMIES
// =========================================================

const int ENEMY_MAX_HP = 50;
const int ENEMY_DAMAGE_PER_HIT = 10;
const double ENEMY_FIRE_INTERVAL = 0.5;
const double ENEMY_HIT_FLASH_DURATION = 0.15;
const double ENEMY_DYING_HOLD_TIME = 1.0;
const double ENEMY_RECOIL_DURATION = 0.1;
const double ENEMY_RECOIL_PIXELS = 6.0;
const double ENEMY_HIT_RADIUS = 55.0;

// Enemy 1 Block Data (Moved deeper into scene 2 so it appears AFTER the bridge)
const double ENEMY_BLOCK_X = 850.0;
const double ENEMY_BLOCK_W = 80.0;
const double ENEMY_BLOCK_H = 65.0;

const double ENEMY1_X = ENEMY_BLOCK_X + 25.0;
const double ENEMY2_X = OBSTACLE_X1 + 140.0;

// Different Y offsets for sitting vs standing sprites
const double ENEMY1_Y_OFFSET = -115.0;
const double ENEMY2_Y_OFFSET = -95.0;

struct Enemy
{
    double x, y;
    double dy; // Vertical velocity for realistic falling

    int hp;
    bool active;
    bool alive;
    bool facingRight;

    double dyingTimer;
    bool corpseGone;

    double hitFlashTimer;
    double fireCooldownTimer;
    double muzzleFlashTimer;
    double recoilTimer;

    unsigned int idleTex;
    unsigned int hitTex;
    unsigned int fallTex;
    unsigned int deadTex;
};

Enemy enemy1;
Enemy enemy2;

void resetLevel1()
{
    assassin.x = 200.0;
    assassin.y = GROUND_Y;
    assassin.dx = 0.0;
    assassin.dy = 0.0;
    assassin.isJumping = false;
    assassin.facingRight = true;
    assassin.walkFrame = 0;
    assassin.walkFrameTimer = 0.0;
    assassin.jumpFrame = 0;
    assassin.jumpFrameTimer = 0.0;
    assassin.jumpDx = 0.0;

    cameraX = 0.0;
    gunEquipped = false;
    isFiring = false;
    muzzleFlashTimer = 0.0;
    fireCooldownTimer = 0.0;
    playerRecoilTimer = 0.0;

    playerHP = PLAYER_MAX_HP;

    landmineActive = true;
    landmineTriggered = false;
    killedByMine = false;
    landmineTimer = 0.0;

    // Enemy 1: Rendered sitting on the custom block past the bridge
    enemy1.x = ENEMY1_X;
    enemy1.y = groundYAt(ENEMY_BLOCK_X) + ENEMY_BLOCK_H + ENEMY1_Y_OFFSET;
    enemy1.dy = 0.0;
    enemy1.hp = ENEMY_MAX_HP;
    enemy1.active = false;
    enemy1.alive = true;
    enemy1.facingRight = false;
    enemy1.dyingTimer = 0.0;
    enemy1.corpseGone = false;
    enemy1.hitFlashTimer = 0.0;
    enemy1.fireCooldownTimer = ENEMY_FIRE_INTERVAL;
    enemy1.muzzleFlashTimer = 0.0;
    enemy1.recoilTimer = 0.0;

    // Enemy 2: Rendered on the ground behind the rock in Scene 2
    enemy2.x = ENEMY2_X;
    enemy2.y = GROUND_Y_SCENE2 + ENEMY2_Y_OFFSET;
    enemy2.dy = 0.0;
    enemy2.hp = ENEMY_MAX_HP;
    enemy2.active = false;
    enemy2.alive = true;
    enemy2.facingRight = false;
    enemy2.dyingTimer = 0.0;
    enemy2.corpseGone = false;
    enemy2.hitFlashTimer = 0.0;
    enemy2.fireCooldownTimer = ENEMY_FIRE_INTERVAL;
    enemy2.muzzleFlashTimer = 0.0;
    enemy2.recoilTimer = 0.0;
}


// =========================================================
// EFFECTS
// =========================================================

void drawLandmineExplosion(double screenX, double screenY, double timer)
{
    if (timer <= 0.0) return;
    double duration = 1.5;
    double t = timer / duration; // 1.0 down to 0.0
    double invT = 1.0 - t;       // 0.0 up to 1.0

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Expanding thick smoke
    glColor4f(0.2f, 0.2f, 0.2f, 0.8f * t);
    iFilledCircle(screenX, screenY + invT * 120, 50 + invT * 80);
    iFilledCircle(screenX - 40 - invT * 40, screenY + invT * 90 + 20, 40 + invT * 60);
    iFilledCircle(screenX + 40 + invT * 40, screenY + invT * 100 + 10, 45 + invT * 65);

    // Dust/Debris
    glColor4f(0.1f, 0.1f, 0.1f, 0.9f * t);
    iFilledCircle(screenX - 70 * invT, screenY + 40 * invT, 20 + invT * 20);
    iFilledCircle(screenX + 60 * invT, screenY + 50 * invT, 25 + invT * 25);

    // Fire Burst (Only visible for the first half of the explosion)
    if (t > 0.5)
    {
        double fireT = (t - 0.5) / 0.5; // 1.0 down to 0.0

        // Orange outer fire
        glColor4f(1.0f, 0.5f, 0.0f, 0.9f * fireT);
        iFilledCircle(screenX, screenY + (1.0 - fireT) * 40 + 10, 40 + (1.0 - fireT) * 50);

        // Yellow inner fire
        glColor4f(1.0f, 0.9f, 0.2f, 1.0f * fireT);
        iFilledCircle(screenX, screenY + (1.0 - fireT) * 20 + 10, 25 + (1.0 - fireT) * 30);

        // White hot core
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f * fireT);
        iFilledCircle(screenX, screenY + (1.0 - fireT) * 10 + 10, 10 + (1.0 - fireT) * 15);
    }

    glDisable(GL_BLEND);
}


// =========================================================
// PLATFORM / OBSTACLE DRAWING
// =========================================================

void drawWorldPlatform(double worldX0, double worldX1)
{
    int blockW = 42;

    for (double wx = worldX0; wx < worldX1; wx += blockW)
    {
        double segEnd = wx + blockW;
        if (segEnd > worldX1) segEnd = worldX1;

        if (wx < GAP_START_X && segEnd > GAP_START_X) segEnd = GAP_START_X;
        if (wx >= GAP_START_X && wx < GAP_END_X)
        {
            wx = GAP_END_X - blockW;
            continue;
        }

        double w = segEnd - wx;
        if (w <= 0) continue;

        double y = groundYAt(wx);
        int blockH = 30;

        int sx = (int)worldToScreen(wx);
        int sw = (int)w;

        iSetColor(96, 70, 38);
        iFilledRectangle(sx, (int)y - blockH, sw, blockH);

        iSetColor(150, 118, 62);
        iFilledRectangle(sx, (int)y - 10, sw, 10);

        iSetColor(60, 42, 20);
        iLine(sx, (int)y, sx, (int)y - blockH);

        iSetColor(190, 160, 90);
        iFilledCircle(sx + 8, (int)y - 8, 2);
        if (sw > 16)
            iFilledCircle(sx + sw - 8, (int)y - 8, 2);
    }
}

void drawRockObstacle(double x0, double x1, double groundY, double topY)
{
    int cx = (int)((x0 + x1) / 2.0);
    int baseY = (int)groundY;
    int height = (int)(topY - groundY);
    int width = (int)(x1 - x0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.30f);
    int segs = 20;
    glBegin(GL_POLYGON);
    for (int s = 0; s < segs; s++)
    {
        double ang = (2 * M_PI * s) / segs;
        glVertex2d(cx + cos(ang) * (width * 0.55), baseY + 2 + sin(ang) * 6);
    }
    glEnd();
    glDisable(GL_BLEND);

    iSetColor(95, 92, 88);
    iFilledCircle((int)x0 + width / 6, baseY + height / 3, (int)(width * 0.24));
    iFilledCircle((int)x0 + width / 2, baseY + height / 3, (int)(width * 0.26));
    iFilledCircle((int)x0 + (5 * width) / 6, baseY + height / 3, (int)(width * 0.24));

    iSetColor(120, 116, 110);
    iFilledCircle((int)x0 + width / 3, baseY + height / 2, (int)(width * 0.24));
    iFilledCircle((int)x0 + (2 * width) / 3, baseY + height / 2, (int)(width * 0.24));

    iSetColor(70, 67, 64);
    iFilledCircle((int)x0 + width / 6, baseY + height / 5, (int)(width * 0.14));
    iFilledCircle((int)x0 + width / 2, baseY + height / 6, (int)(width * 0.12));

    double plateauY0 = baseY + height * 0.62;
    double plateauY1 = topY;
    double plateauX0 = x0 + OBSTACLE_PLATEAU_MARGIN * 0.4;
    double plateauX1 = x1 - OBSTACLE_PLATEAU_MARGIN * 0.4;

    double px[8] =
    {
        plateauX0 + 10, plateauX1 - 10,
        plateauX1, plateauX1,
        plateauX1 - 10, plateauX0 + 10,
        plateauX0, plateauX0
    };
    double py[8] =
    {
        plateauY1, plateauY1,
        plateauY1 - 10, plateauY0 + 14,
        plateauY0, plateauY0,
        plateauY0 + 14, plateauY1 - 10
    };

    iSetColor(128, 124, 116);
    iFilledPolygon(px, py, 8);

    iSetColor(160, 155, 144);
    iFilledRectangle((int)plateauX0 + 6, (int)plateauY1 - 6, (int)(plateauX1 - plateauX0) - 12, 6);

    iSetColor(75, 72, 68);
    iLine((int)plateauX0, (int)plateauY0, (int)plateauX1, (int)plateauY0);

    iSetColor(80, 76, 72);
    iFilledCircle((int)x0 - 8, baseY + 4, 6);
    iFilledCircle((int)x1 + 10, baseY + 3, 5);
    iFilledCircle((int)cx, baseY + 2, 4);
}

void drawMuzzleFlash(double nozzleScreenX, double nozzleScreenY, double flashTimer)
{
    if (flashTimer <= 0.0) return;

    double t = flashTimer / MUZZLE_FLASH_DURATION;
    if (t > 1.0) t = 1.0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.85f, 0.3f, 0.55f * (float)t);
    int glowSegs = 16;
    double glowR = 16.0 * t + 4.0;
    glBegin(GL_POLYGON);
    for (int s = 0; s < glowSegs; s++)
    {
        double ang = (2 * M_PI * s) / glowSegs;
        glVertex2d(nozzleScreenX + cos(ang) * glowR, nozzleScreenY + sin(ang) * glowR * 0.8);
    }
    glEnd();

    glColor4f(1.0f, 1.0f, 0.9f, 0.9f * (float)t);
    int coreSegs = 10;
    double coreR = 7.0 * t + 2.0;
    glBegin(GL_POLYGON);
    for (int s = 0; s < coreSegs; s++)
    {
        double ang = (2 * M_PI * s) / coreSegs;
        glVertex2d(nozzleScreenX + cos(ang) * coreR, nozzleScreenY + sin(ang) * coreR);
    }
    glEnd();

    glDisable(GL_BLEND);

    iSetColor(255, 240, 180);
    double spikeLen = 14.0 * t + 3.0;
    iLine((int)(nozzleScreenX - spikeLen), (int)nozzleScreenY, (int)(nozzleScreenX + spikeLen), (int)nozzleScreenY);
    iLine((int)nozzleScreenX, (int)(nozzleScreenY - spikeLen * 0.6), (int)nozzleScreenX, (int)(nozzleScreenY + spikeLen * 0.6));
}

void drawEnemyHpBar(double screenX, double topScreenY, int hp, int maxHp)
{
    int barW = 60, barH = 8;
    int bx = (int)(screenX - barW / 2.0);
    int by = (int)(topScreenY + 10);

    iSetColor(20, 20, 20);
    iFilledRectangle(bx - 2, by - 2, barW + 4, barH + 4);

    iSetColor(60, 60, 60);
    iFilledRectangle(bx, by, barW, barH);

    double hpFrac = (double)hp / (double)maxHp;
    if (hpFrac < 0.0) hpFrac = 0.0;

    iSetColor(210, 40, 30);
    iFilledRectangle(bx, by, (int)(barW * hpFrac), barH);
}

void drawEnemy(Enemy& e)
{
    if (!e.active || e.corpseGone) return;

    unsigned int tex = e.idleTex;

    if (!e.alive)
    {
        if (&e == &enemy1)
        {
            tex = e.fallTex;
        }
        else
        {
            if (e.fallTex != 0 && e.dyingTimer < 0.4)
                tex = e.fallTex;
            else if (e.deadTex != 0)
                tex = e.deadTex;
            else if (e.fallTex != 0)
                tex = e.fallTex;
        }
    }
    else if (e.hitFlashTimer > 0.0 && e.hitTex != 0)
    {
        tex = e.hitTex;
    }

    if (tex == 0) return;

    double screenX = worldToScreen(e.x);

    double recoilOffset = 0.0;
    if (e.recoilTimer > 0.0)
    {
        double t = e.recoilTimer / ENEMY_RECOIL_DURATION;
        recoilOffset = ENEMY_RECOIL_PIXELS * t * (e.facingRight ? -1.0 : 1.0);
    }

    double drawX = screenX - SPRITE_DRAW_W / 2.0 + recoilOffset;
    double drawY = e.y + SPRITE_Y_OFFSET;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    glTranslated(screenX, 0.0, 0.0);

    if (e.facingRight) glScaled(-1.0, 1.0, 1.0);

    glTranslated(-screenX, 0.0, 0.0);

    iShowImage((int)drawX, (int)drawY, SPRITE_DRAW_W, SPRITE_DRAW_H, tex);

    glPopMatrix();
    glDisable(GL_BLEND);

    if (e.alive)
    {
        if (e.muzzleFlashTimer > 0.0)
        {
            double nozzleLocalX = SPRITE_DRAW_W * 0.12;
            double nozzleLocalY = SPRITE_DRAW_H * 0.55;

            double nozzleScreenX;

            if (!e.facingRight)
            {
                nozzleScreenX = drawX + nozzleLocalX;
            }
            else
            {
                double offsetFromCenter = (drawX + nozzleLocalX) - screenX;
                nozzleScreenX = screenX - offsetFromCenter;
            }

            double nozzleScreenY = drawY + nozzleLocalY;

            drawMuzzleFlash(nozzleScreenX, nozzleScreenY, e.muzzleFlashTimer);
        }

        drawEnemyHpBar(screenX, drawY + SPRITE_DRAW_H, e.hp, ENEMY_MAX_HP);
    }
}

void updateEnemy(Enemy& e, bool canShoot)
{
    if (!e.active) return;

    if (e.alive)
    {
        e.facingRight = (assassin.x >= e.x);

        if (e.fireCooldownTimer > 0.0) e.fireCooldownTimer -= 0.02;

        if (canShoot && e.fireCooldownTimer <= 0.0)
        {
            e.fireCooldownTimer = ENEMY_FIRE_INTERVAL;
            e.muzzleFlashTimer = MUZZLE_FLASH_DURATION;
            e.recoilTimer = ENEMY_RECOIL_DURATION;

            playerHP -= ENEMY_DAMAGE_PER_HIT;
            if (playerHP < 0) playerHP = 0;
        }

        if (e.hitFlashTimer > 0.0) e.hitFlashTimer -= 0.02;

        if (e.muzzleFlashTimer > 0.0)
        {
            e.muzzleFlashTimer -= 0.02;
            if (e.muzzleFlashTimer < 0.0) e.muzzleFlashTimer = 0.0;
        }

        if (e.recoilTimer > 0.0)
        {
            e.recoilTimer -= 0.02;
            if (e.recoilTimer < 0.0) e.recoilTimer = 0.0;
        }

        if (e.hp <= 0)
        {
            e.alive = false;
            e.dyingTimer = 0.0;
            e.dy = 4.0;
        }
    }
    else
    {
        if (&e == &enemy1)
        {
            e.dy -= GRAVITY;
            e.y += e.dy;
            e.x -= 2.5;

            if (e.y < FALL_DEATH_Y)
            {
                e.corpseGone = true;
                e.active = false;
            }
        }
        else
        {
            e.dyingTimer += 0.02;

            if (e.dyingTimer >= ENEMY_DYING_HOLD_TIME)
            {
                e.corpseGone = true;
                e.active = false;
            }
        }
    }
}


// =========================================================
// DRAW
// =========================================================

void iDraw()
{
    iClear();

    if (state == LOADING)
    {
        iShowImage(0, 0, WIN_W, WIN_H, loadingBgTexture);

        int barY = 60;
        int barW = 500;
        int barH = 24;
        int startX = WIN_W / 2 - barW / 2;

        double bgX[6] =
        {
            startX - 25, startX, startX + barW,
            startX + barW + 25, startX + barW, startX
        };
        double bgY[6] =
        {
            barY + barH / 2, barY + barH + 6, barY + barH + 6,
            barY + barH / 2, barY - 6, barY - 6
        };

        iSetColor(110, 110, 115);
        iFilledPolygon(bgX, bgY, 6);
        iSetColor(180, 180, 190);
        iLine(bgX[0], bgY[0], bgX[1], bgY[1]);
        iLine(bgX[1], bgY[1], bgX[2], bgY[2]);
        iSetColor(130, 130, 140);
        iLine(bgX[2], bgY[2], bgX[3], bgY[3]);
        iSetColor(35, 35, 40);
        iLine(bgX[0], bgY[0], bgX[5], bgY[5]);
        iLine(bgX[5], bgY[5], bgX[4], bgY[4]);
        iLine(bgX[4], bgY[4], bgX[3], bgY[3]);

        iSetColor(40, 40, 40);
        iFilledRectangle(startX, barY, barW, barH);
        iSetColor(20, 20, 20);
        iLine(startX, barY + barH, startX + barW, barY + barH);
        iSetColor(90, 90, 90);
        iLine(startX, barY, startX + barW, barY);

        int fillW = barW * loadingProgress / 100;

        if (fillW > 0)
        {
            iSetColor(25, 130, 190);
            iFilledRectangle(startX, barY + barH / 2, fillW, barH / 2);
            iSetColor(0, 90, 140);
            iFilledRectangle(startX, barY, fillW, barH / 2);
            iSetColor(100, 190, 240);
            iLine(startX, barY + barH - 1, startX + fillW, barY + barH - 1);
        }

        char buf[20];
        sprintf(buf, "%d%%", loadingProgress);

        int textX;
        if (fillW > 50) textX = startX + fillW - 45; else textX = startX + fillW + 10;
        if (fillW > 50) iSetColor(255, 255, 255); else iSetColor(200, 200, 200);

        iText(textX, barY + 6, buf, GLUT_BITMAP_HELVETICA_18);
    }

    else if (state == MENU)
    {
        iShowImage(0, 0, WIN_W, WIN_H, menuBgTexture);
        iSetColor(255, 255, 255);
        iText(WIN_W / 2 - 90, 500, "SILENT PURSUIT", GLUT_BITMAP_TIMES_ROMAN_24);
        drawButton(btnNewGame);
        drawButton(btnOptions);
        drawButton(btnCredits);
        drawButton(btnQuit);
    }

    else if (state == LEVEL_SELECT)
    {
        iShowImage(0, 0, WIN_W, WIN_H, menuBgTexture);
        iSetColor(255, 255, 255);
        iText(WIN_W / 2 - 80, 500, "SELECT LEVEL", GLUT_BITMAP_TIMES_ROMAN_24);

        drawButton(btnLevel1);
        drawButton(btnLevel2);
        drawButton(btnLevel3);
        drawButton(btnLevel4);
        drawButton(btnLevel5);
        drawButton(btnLvlBack);
    }

    else if (state == OPTIONS)
    {
        iSetColor(14, 11, 8);
        iFilledRectangle(0, 0, WIN_W, WIN_H);
        iSetColor(255, 255, 255);
        iText(WIN_W / 2 - 40, 500, "OPTIONS", GLUT_BITMAP_TIMES_ROMAN_24);
        drawButton(btnAudio);
        drawButton(btnControls);
        drawButton(btnOptBack);
    }

    else if (state == CREDITS)
    {
        iSetColor(14, 11, 8);
        iFilledRectangle(0, 0, WIN_W, WIN_H);
        iSetColor(255, 255, 255);
        iText(WIN_W / 2 - 50, 500, "CREDITS", GLUT_BITMAP_TIMES_ROMAN_24);
        iSetColor(CLR_BTN_TEXT_R, CLR_BTN_TEXT_G, CLR_BTN_TEXT_B);
        iText(WIN_W / 2 - 150, 420, "Game Design & Programming:", GLUT_BITMAP_HELVETICA_18);
        iText(WIN_W / 2 - 100, 390, "Partho Das", GLUT_BITMAP_HELVETICA_18);
        iText(WIN_W / 2 - 100, 360, "Khadija Bibi Sumona", GLUT_BITMAP_HELVETICA_18);
        iText(WIN_W / 2 - 100, 330, "Shahriar Mostafa Razin", GLUT_BITMAP_HELVETICA_18);
        iText(WIN_W / 2 - 150, 280, "Built with iGraphics", GLUT_BITMAP_HELVETICA_18);
        drawButton(btnCredBack);
    }

    else if (state == GAMEPLAY)
    {
        // Draw continuous scrolling map (Image 1 into Image 3 permanently)
        iShowImage((int)worldToScreen(0), 0, WIN_W, WIN_H, level1BgTexture);
        iShowImage((int)worldToScreen(WIN_W), 0, WIN_W, WIN_H, level1Scene3BgTexture);

        drawWorldPlatform(0.0, GAP_START_X);
        drawWorldPlatform(GAP_END_X, WORLD_WIDTH);

        // Draw the landmine BEFORE the explosion occurs (if active)
        if (landmineActive)
        {
            double mineScreenX = worldToScreen(MINE_X);
            double mineGroundY = groundYAt(MINE_X);

            // Dark grey base
            iSetColor(60, 65, 60);
            iFilledRectangle((int)mineScreenX - 12, (int)mineGroundY, 24, 5);

            // Dome top
            iSetColor(90, 95, 90);
            iFilledCircle((int)mineScreenX, (int)mineGroundY + 5, 8);

            // Blinking red light
            iSetColor(220, 30, 30);
            iFilledCircle((int)mineScreenX, (int)mineGroundY + 11, 3);
        }

        double blockScreenX = worldToScreen(ENEMY_BLOCK_X);
        double blockGroundY = groundYAt(ENEMY_BLOCK_X);

        iSetColor(190, 185, 175);
        iFilledRectangle((int)blockScreenX, (int)blockGroundY, (int)ENEMY_BLOCK_W, (int)ENEMY_BLOCK_H);

        iSetColor(220, 215, 205);
        iFilledRectangle((int)blockScreenX, (int)blockGroundY + (int)ENEMY_BLOCK_H - 6, (int)ENEMY_BLOCK_W, 6);
        iSetColor(140, 135, 125);
        iFilledRectangle((int)blockScreenX + (int)ENEMY_BLOCK_W - 8, (int)blockGroundY, 8, (int)ENEMY_BLOCK_H);
        iSetColor(100, 95, 85);
        iLine((int)blockScreenX, (int)blockGroundY, (int)blockScreenX, (int)blockGroundY + (int)ENEMY_BLOCK_H);
        iLine((int)blockScreenX, (int)blockGroundY + (int)ENEMY_BLOCK_H, (int)blockScreenX + (int)ENEMY_BLOCK_W, (int)blockGroundY + (int)ENEMY_BLOCK_H);

        drawRockObstacle(
            worldToScreen(OBSTACLE_X0), worldToScreen(OBSTACLE_X1),
            GROUND_Y_SCENE2, OBSTACLE_TOP_Y
        );

        drawEnemy(enemy1);
        drawEnemy(enemy2);

        double groundY = groundYAt(assassin.x);

        if (!isOverGap(assassin.x))
        {
            double heightAboveGround = assassin.y - groundY;
            double shadowScale = 1.0 - (heightAboveGround / 160.0);
            if (shadowScale < 0.25) shadowScale = 0.25;
            if (shadowScale > 1.0) shadowScale = 1.0;

            int shadowW = (int)(50 * shadowScale);
            int shadowH = (int)(12 * shadowScale);
            double shadowScreenX = worldToScreen(assassin.x);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.0f, 0.0f, 0.0f, 0.35f * (float)shadowScale + 0.15f);
            int segs = 24;
            glBegin(GL_POLYGON);
            for (int s = 0; s < segs; s++)
            {
                double ang = (2 * M_PI * s) / segs;
                glVertex2d(shadowScreenX + cos(ang) * shadowW, groundY + 3 + sin(ang) * shadowH);
            }
            glEnd();
            glDisable(GL_BLEND);
        }

        double screenX = worldToScreen(assassin.x);

        unsigned int currentSprite = 0;
        int drawW = SPRITE_DRAW_W;
        int drawH = SPRITE_DRAW_H;
        double yOffset = SPRITE_Y_OFFSET;

        int gunPoseIdx = GUN_NEUTRAL;
        bool usingGunPose = false;

        if (assassin.isJumping)
        {
            if (gunEquipped)
            {
                int jgframe = assassin.jumpFrame;
                if (jgframe < 0) jgframe = 0;
                if (jgframe >= JUMP_GUN_FRAME_COUNT) jgframe = JUMP_GUN_FRAME_COUNT - 1;

                currentSprite = jumpGunFrames[jgframe];
                drawW = JUMP_GUN_DRAW_W[jgframe];
                drawH = JUMP_GUN_DRAW_H;
                yOffset = SPRITE_Y_OFFSET;
            }
            else
            {
                int jframe = assassin.jumpFrame;
                if (jframe < 0) jframe = 0;
                if (jframe >= JUMP_FRAME_COUNT) jframe = JUMP_FRAME_COUNT - 1;
                currentSprite = jumpFrames[jframe];
            }
        }
        else if (gunEquipped)
        {
            if (fabs(assassin.dx) > 0.5)
            {
                int frame = assassin.walkFrame;
                if (frame >= WALK_GUN_FRAME_COUNT) frame = 0;

                currentSprite = walkGunFrames[frame];
                drawW = GUN_SPRITE_DRAW_W;
                drawH = GUN_SPRITE_DRAW_H;
                yOffset = GUN_SPRITE_Y_OFFSET;

                gunPoseIdx = GUN_NEUTRAL;
                usingGunPose = true;
            }
            else
            {
                double shoulderScreenY = assassin.y + SPRITE_Y_OFFSET + SPRITE_DRAW_H * 0.6;
                double dy = crosshairY - shoulderScreenY;

                int idx;
                if (dy > AIM_TIER2) idx = GUN_UP3;
                else if (dy > AIM_TIER1) idx = GUN_UP2;
                else if (dy > AIM_DEADZONE) idx = GUN_UP1;
                else if (dy < -AIM_TIER2) idx = GUN_DOWN3;
                else if (dy < -AIM_TIER1) idx = GUN_DOWN2;
                else if (dy < -AIM_DEADZONE) idx = GUN_DOWN1;
                else idx = GUN_NEUTRAL;

                currentSprite = gunFrames[idx];
                drawW = GUN_SPRITE_DRAW_W;
                drawH = GUN_SPRITE_DRAW_H;
                yOffset = GUN_SPRITE_Y_OFFSET;
                gunPoseIdx = idx;
                usingGunPose = true;
            }
        }
        else
        {
            if (fabs(assassin.dx) > 0.5)
            {
                int frame = assassin.walkFrame;
                if (frame < 0) frame = 0;
                if (frame >= WALK_FRAME_COUNT) frame = 0;
                currentSprite = walkRightFrames[frame];
            }
            else
            {
                currentSprite = idleFrame;

                // Made slightly bigger to match the scale of the other action sprites
                drawW = 162;
                drawH = 300;
                yOffset = -22.0; // Pulls the standing sprite down to the ground
            }
        }

        lastGunPoseIdx = gunPoseIdx;

        double playerRecoilOffset = 0.0;
        if (playerRecoilTimer > 0.0)
        {
            double t = playerRecoilTimer / PLAYER_RECOIL_DURATION;
            playerRecoilOffset = PLAYER_RECOIL_PIXELS * t * (assassin.facingRight ? -1.0 : 1.0);
        }

        double drawX = 0.0;
        double drawY = 0.0;

        if (currentSprite != 0)
        {
            drawX = screenX - drawW / 2.0 + playerRecoilOffset;
            drawY = assassin.y + yOffset;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glPushMatrix();
            glTranslated(screenX, 0.0, 0.0);
            if (!assassin.facingRight)
                glScaled(-1.0, 1.0, 1.0);
            glTranslated(-screenX, 0.0, 0.0);

            iShowImage((int)drawX, (int)drawY, drawW, drawH, currentSprite);

            glPopMatrix();
            glDisable(GL_BLEND);
        }

        if (landmineTriggered && landmineTimer > 0.0)
        {
            drawLandmineExplosion(worldToScreen(MINE_X), groundYAt(MINE_X), landmineTimer);
        }

        if (usingGunPose && muzzleFlashTimer > 0.0)
        {
            NozzleOffset noz = gunNozzle[gunPoseIdx];

            double localX = noz.dx;
            double localY = noz.dy;

            double nozzleScreenX;
            if (assassin.facingRight)
            {
                nozzleScreenX = drawX + localX;
            }
            else
            {
                double worldOffsetFromCenter = (drawX + localX) - screenX;
                nozzleScreenX = screenX - worldOffsetFromCenter;
            }

            double nozzleScreenY = drawY + localY;

            drawMuzzleFlash(nozzleScreenX, nozzleScreenY, muzzleFlashTimer);
        }

        drawCrosshair();

        iSetColor(255, 255, 255);
        if (sceneAt(assassin.x) == 2) {
            iText(15, WIN_H - 30, "LEVEL 1: The Castle Gates", GLUT_BITMAP_HELVETICA_18);
        }
        else {
            iText(15, WIN_H - 30, "LEVEL 1: The Outer Courtyard", GLUT_BITMAP_HELVETICA_18);
        }

        char hpBuf[32];
        sprintf(hpBuf, "HP: %d / %d", playerHP, PLAYER_MAX_HP);
        iText(15, WIN_H - 90, hpBuf, GLUT_BITMAP_HELVETICA_18);

        if (sceneAt(assassin.x) == 1)
        {
            iText(15, WIN_H - 50, "A/D to move | W to jump | G to draw/holster gun | Left-click to fire", GLUT_BITMAP_HELVETICA_12);
            iText(15, WIN_H - 68, "Jump the broken bridge - don't fall!", GLUT_BITMAP_HELVETICA_12);
        }
        else
        {
            iText(15, WIN_H - 50, "A/D to move | W to jump | G to draw/holster gun | Left-click to fire", GLUT_BITMAP_HELVETICA_12);
            if (enemy2.alive) {
                iText(15, WIN_H - 68, "Jump over the rock ahead! Kill the soldier to proceed.", GLUT_BITMAP_HELVETICA_12);
            }
            else {
                iText(15, WIN_H - 68, "Careful! Watch out for hidden traps near the gates.", GLUT_BITMAP_HELVETICA_12);
            }
        }
    }

    else if (state == GAME_OVER)
    {
        iShowImage(0, 0, WIN_W, WIN_H, level1BgTexture);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.72f);
        glBegin(GL_QUADS);
        glVertex2d(0, 0);
        glVertex2d(WIN_W, 0);
        glVertex2d(WIN_W, WIN_H);
        glVertex2d(0, WIN_H);
        glEnd();
        glDisable(GL_BLEND);

        iSetColor(220, 40, 30);
        iText(WIN_W / 2 - 95, 430, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
        iSetColor(220, 210, 200);

        if (playerHP <= 0 && killedByMine)
            iText(WIN_W / 2 - 165, 390, "You were killed by a hidden landmine.", GLUT_BITMAP_HELVETICA_18);
        else if (playerHP <= 0)
            iText(WIN_W / 2 - 105, 390, "You were shot to death.", GLUT_BITMAP_HELVETICA_18);
        else
            iText(WIN_W / 2 - 145, 390, "You fell into the ravine below the bridge.", GLUT_BITMAP_HELVETICA_18);

        drawButton(btnRetry);
        drawButton(btnGoToMenu);
    }
}


// =========================================================
// MOUSE
// =========================================================

void iMouseMove(int mx, int my)
{
    int lx, ly;
    toLogicalCoords(mx, my, lx, ly);

    if (state == GAMEPLAY)
    {
        crosshairX = lx;
        crosshairY = ly;
    }
}

void iPassiveMouseMove(int mx, int my)
{
    int lx, ly;
    toLogicalCoords(mx, my, lx, ly);

    if (state == MENU)
    {
        btnNewGame.hovered = isInside(btnNewGame, lx, ly);
        btnOptions.hovered = isInside(btnOptions, lx, ly);
        btnCredits.hovered = isInside(btnCredits, lx, ly);
        btnQuit.hovered = isInside(btnQuit, lx, ly);
    }
    else if (state == LEVEL_SELECT)
    {
        btnLevel1.hovered = isInside(btnLevel1, lx, ly);
        btnLevel2.hovered = isInside(btnLevel2, lx, ly);
        btnLevel3.hovered = isInside(btnLevel3, lx, ly);
        btnLevel4.hovered = isInside(btnLevel4, lx, ly);
        btnLevel5.hovered = isInside(btnLevel5, lx, ly);
        btnLvlBack.hovered = isInside(btnLvlBack, lx, ly);
    }
    else if (state == OPTIONS)
    {
        btnAudio.hovered = isInside(btnAudio, lx, ly);
        btnControls.hovered = isInside(btnControls, lx, ly);
        btnOptBack.hovered = isInside(btnOptBack, lx, ly);
    }
    else if (state == CREDITS)
    {
        btnCredBack.hovered = isInside(btnCredBack, lx, ly);
    }
    else if (state == GAMEPLAY)
    {
        crosshairX = lx;
        crosshairY = ly;
    }
    else if (state == GAME_OVER)
    {
        btnRetry.hovered = isInside(btnRetry, lx, ly);
        btnGoToMenu.hovered = isInside(btnGoToMenu, lx, ly);
    }
}

void iMouse(int button, int mstate, int mx, int my)
{
    int lx, ly;
    toLogicalCoords(mx, my, lx, ly);

    if (button == GLUT_LEFT_BUTTON && mstate == GLUT_DOWN)
    {
        if (state == MENU)
        {
            if (isInside(btnNewGame, lx, ly)) { state = LEVEL_SELECT; }
            else if (isInside(btnOptions, lx, ly)) state = OPTIONS;
            else if (isInside(btnCredits, lx, ly)) state = CREDITS;
            else if (isInside(btnQuit, lx, ly)) exit(0);
        }
        else if (state == LEVEL_SELECT)
        {
            if (isInside(btnLevel1, lx, ly) && !btnLevel1.locked) {
                resetLevel1();
                state = GAMEPLAY;
            }
            else if (isInside(btnLvlBack, lx, ly)) {
                state = MENU;
            }
        }
        else if (state == OPTIONS)
        {
            if (isInside(btnOptBack, lx, ly)) state = MENU;
        }
        else if (state == CREDITS)
        {
            if (isInside(btnCredBack, lx, ly)) state = MENU;
        }
        else if (state == GAME_OVER)
        {
            if (isInside(btnRetry, lx, ly)) { resetLevel1(); state = GAMEPLAY; }
            else if (isInside(btnGoToMenu, lx, ly)) state = MENU;
        }
    }

    if (button == GLUT_LEFT_BUTTON && state == GAMEPLAY)
    {
        if (mstate == GLUT_DOWN)
        {
            isFiring = true;
        }
        else if (mstate == GLUT_UP)
        {
            isFiring = false;
        }
    }
}


// =========================================================
// KEYBOARD
// =========================================================

void iKeyboard(unsigned char key)
{
    if (key == 27) // Escape key
    {
        if (state == OPTIONS || state == CREDITS || state == LEVEL_SELECT) state = MENU;
    }

    if (state == GAME_OVER && (key == 'r' || key == 'R'))
    {
        resetLevel1();
        state = GAMEPLAY;
    }
}

void keyRelease(unsigned char key, int x, int y) {}
void iSpecialKeyboard(unsigned char key) {}
void specialKeyRelease(int key, int x, int y) {}


// =========================================================
// UPDATE
// =========================================================

void fixedUpdate()
{
    animTime += 0.02;

    if (state == LOADING && !loadingDone)
    {
        loadingProgress++;
        if (loadingProgress >= 100)
        {
            loadingProgress = 100;
            loadingDone = true;
            state = MENU;
        }
    }
    else if (state == GAMEPLAY)
    {
        bool leftHeld = isKeyPressed('a') || isKeyPressed('A') || isSpecialKeyPressed(GLUT_KEY_LEFT);
        bool rightHeld = isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT);

        bool gHeld = isKeyPressed('g') || isKeyPressed('G');
        if (gHeld && !gKeyWasHeld)
        {
            gunEquipped = !gunEquipped;
            if (!gunEquipped)
            {
                isFiring = false;
            }
        }
        gKeyWasHeld = gHeld;

        if (leftHeld && !rightHeld) assassin.dx = -3.0;
        else if (rightHeld && !leftHeld) assassin.dx = 3.0;
        else if (assassin.isJumping) assassin.dx = assassin.jumpDx;
        else assassin.dx = 0.0;

        assassin.x += assassin.dx;

        // Block player from passing without killing Enemy 2
        if (enemy2.alive && assassin.x > SWITCH_X)
        {
            assassin.x = SWITCH_X;
        }

        // Trigger the Landmine Trap by stepping on it
        if (landmineActive && assassin.x >= MINE_X - 15.0 && assassin.x <= MINE_X + 15.0 && assassin.y <= groundYAt(assassin.x) + 5.0)
        {
            landmineActive = false;
            landmineTriggered = true;
            landmineTimer = 1.5;
            playerHP = 0; // Instant death for stepping on it
            killedByMine = true;
        }

        // Timer to fade the landmine smoke
        if (landmineTimer > 0.0)
        {
            landmineTimer -= 0.02;
            if (landmineTimer < 0.0) landmineTimer = 0.0;
        }

        if (assassin.y < OBSTACLE_TOP_Y - 25.0)
        {
            if (assassin.x > OBSTACLE_X0 - 20.0 && assassin.x < OBSTACLE_X1 + 20.0)
            {
                if (assassin.dx > 0.0) assassin.x = OBSTACLE_X0 - 20.0;
                else if (assassin.dx < 0.0) assassin.x = OBSTACLE_X1 + 20.0;
            }
        }

        double blockGroundY = groundYAt(ENEMY_BLOCK_X);
        if (assassin.y < blockGroundY + ENEMY_BLOCK_H - 25.0)
        {
            if (assassin.x > ENEMY_BLOCK_X - 20.0 && assassin.x < ENEMY_BLOCK_X + ENEMY_BLOCK_W + 20.0)
            {
                if (assassin.dx > 0.0) assassin.x = ENEMY_BLOCK_X - 20.0;
                else if (assassin.dx < 0.0) assassin.x = ENEMY_BLOCK_X + ENEMY_BLOCK_W + 20.0;
            }
        }

        if (assassin.x < 15.0) assassin.x = 15.0;
        if (assassin.x > WORLD_WIDTH - 15.0) assassin.x = WORLD_WIDTH - 15.0;

        double cameraTarget = assassin.x - WIN_W / 2.0;
        if (cameraTarget < 0.0) cameraTarget = 0.0;
        if (cameraTarget > WORLD_WIDTH - WIN_W) cameraTarget = WORLD_WIDTH - WIN_W;
        cameraX += (cameraTarget - cameraX) * CAMERA_FOLLOW;

        double groundY = groundYAt(assassin.x);
        bool overGap = isOverGap(assassin.x);

        if (assassin.x > ENEMY_BLOCK_X - 20.0 && assassin.x < ENEMY_BLOCK_X + ENEMY_BLOCK_W + 20.0) {
            if (assassin.dy <= 0.0 && assassin.y >= blockGroundY + ENEMY_BLOCK_H - 30.0) {
                groundY = blockGroundY + ENEMY_BLOCK_H;
            }
        }

        if (assassin.x > OBSTACLE_X0 - 20.0 && assassin.x < OBSTACLE_X1 + 20.0) {
            if (assassin.dy <= 0.0 && assassin.y >= OBSTACLE_TOP_Y - 30.0) {
                groundY = OBSTACLE_TOP_Y;
            }
        }

        bool onGround = (assassin.y <= groundY) && !overGap;

        if (!onGround) assassin.dy -= GRAVITY;
        assassin.y += assassin.dy;

        if (assassin.y <= groundY && !overGap)
        {
            assassin.y = groundY;
            assassin.dy = 0.0;
            assassin.isJumping = false;
            assassin.jumpFrame = 0;
            assassin.jumpFrameTimer = 0.0;
        }
        else if (assassin.y <= groundY && overGap)
        {
            if (!assassin.isJumping)
            {
                assassin.isJumping = true;
                assassin.jumpFrame = JUMP_FRAME_COUNT - 1;
            }
            if (assassin.y < FALL_DEATH_Y) state = GAME_OVER;
        }

        if ((isKeyPressed('w') || isKeyPressed('W') || isKeyPressed(' ') || isSpecialKeyPressed(GLUT_KEY_UP))
            && !assassin.isJumping)
        {
            assassin.dy = 12.0;
            assassin.isJumping = true;
            assassin.jumpFrame = 0;
            assassin.jumpFrameTimer = 0.0;

            if (leftHeld && !rightHeld) assassin.jumpDx = -4.0;
            else if (rightHeld && !leftHeld) assassin.jumpDx = 4.0;
            else assassin.jumpDx = assassin.facingRight ? 4.0 : -4.0;
        }

        if (assassin.isJumping)
        {
            assassin.jumpFrameTimer += 0.02;

            double frameDuration = gunEquipped
                ? (JUMP_FRAME_DURATION * JUMP_FRAME_COUNT / JUMP_GUN_FRAME_COUNT)
                : JUMP_FRAME_DURATION;

            if (assassin.jumpFrameTimer >= frameDuration)
            {
                assassin.jumpFrameTimer -= frameDuration;

                int maxJumpFrame = gunEquipped ? (JUMP_GUN_FRAME_COUNT - 1) : (JUMP_FRAME_COUNT - 1);
                if (assassin.jumpFrame < maxJumpFrame) assassin.jumpFrame++;
            }
        }

        double screenXNow = worldToScreen(assassin.x);
        if (crosshairX >= (int)screenXNow) assassin.facingRight = true;
        else assassin.facingRight = false;

        if (fabs(assassin.dx) > 0.5 && !assassin.isJumping)
        {
            assassin.walkFrameTimer += 0.02;
            if (assassin.walkFrameTimer >= WALK_FRAME_DURATION)
            {
                assassin.walkFrameTimer -= WALK_FRAME_DURATION;
                assassin.walkFrame++;

                int maxF = gunEquipped ? WALK_GUN_FRAME_COUNT : WALK_FRAME_COUNT;
                if (assassin.walkFrame >= maxF) assassin.walkFrame = 0;
            }
        }
        else
        {
            assassin.walkFrame = 0;
            assassin.walkFrameTimer = 0.0;
        }

        if (!enemy1.active && !enemy1.corpseGone && (cameraX + WIN_W) > ENEMY_BLOCK_X + 20.0)
        {
            enemy1.active = true;
        }

        if (!enemy2.active && !enemy2.corpseGone && (cameraX + WIN_W) > ENEMY2_X)
        {
            enemy2.active = true;
        }

        updateEnemy(enemy1, true);
        updateEnemy(enemy2, !enemy1.alive);

        if (fireCooldownTimer > 0.0)
        {
            fireCooldownTimer -= 0.02;
        }

        if (isFiring && gunEquipped && !assassin.isJumping && fireCooldownTimer <= 0.0)
        {
            muzzleFlashTimer = MUZZLE_FLASH_DURATION;
            fireCooldownTimer = FIRE_COOLDOWN;
            playerRecoilTimer = PLAYER_RECOIL_DURATION;

            // ---- Landmine Shooting Logic ----
            if (landmineActive)
            {
                double mineScreenX = worldToScreen(MINE_X);
                double mineScreenY = groundYAt(MINE_X) + 5.0;

                double distX = crosshairX - mineScreenX;
                double distY = crosshairY - mineScreenY;
                double dist = sqrt(distX * distX + distY * distY);

                if (dist <= 40.0) // Aiming directly at the mine
                {
                    landmineActive = false;
                    landmineTriggered = true;
                    landmineTimer = 1.5;
                    // Note: We DO NOT lower player HP here (safe distance dismantle)
                }
            }

            // ---- Enemy Shooting Logic ----
            Enemy* target = NULL;
            if (enemy1.active && enemy1.alive) target = &enemy1;
            else if (enemy2.active && enemy2.alive) target = &enemy2;

            if (target != NULL)
            {
                double targetScreenX = worldToScreen(target->x);
                double targetScreenY = target->y + SPRITE_DRAW_H * 0.55;

                double distX = crosshairX - targetScreenX;
                double distY = crosshairY - targetScreenY;
                double dist = sqrt(distX * distX + distY * distY);

                if (dist <= ENEMY_HIT_RADIUS)
                {
                    target->hp -= PLAYER_DAMAGE_PER_HIT;
                    if (target->hp < 0) target->hp = 0;
                    target->hitFlashTimer = ENEMY_HIT_FLASH_DURATION;
                }
            }
        }

        if (muzzleFlashTimer > 0.0)
        {
            muzzleFlashTimer -= 0.02;
            if (muzzleFlashTimer < 0.0) muzzleFlashTimer = 0.0;
        }

        if (playerRecoilTimer > 0.0)
        {
            playerRecoilTimer -= 0.02;
            if (playerRecoilTimer < 0.0) playerRecoilTimer = 0.0;
        }

        if (playerHP <= 0)
        {
            state = GAME_OVER;
        }
    }
}


int main()
{
    iInitialize(WIN_W, WIN_H, "Silent Pursuit");

    glutIgnoreKeyRepeat(0);

    loadingBgTexture = iLoadImage("Images//loading_bg.jpeg");
    menuBgTexture = iLoadImage("Images//menu_bg.jpeg");

    level1BgTexture = iLoadImage("Images//Backgrounds//01.jpeg");
    level1Scene2BgTexture = iLoadImage("Images//Backgrounds//02.jpeg"); // Kept loaded but not drawn
    level1Scene3BgTexture = iLoadImage("Images//Backgrounds//03.jpeg");

    // New standing sprite loaded
    idleFrame = iLoadImage("Images//Standing//1.png");

    for (int i = 0; i < WALK_FRAME_COUNT; i++)
    {
        char path[100];
        sprintf(path, "Images//WalkRight//tile%03d.png", i + 1);
        walkRightFrames[i] = iLoadImage(path);
    }

    for (int i = 0; i < WALK_GUN_FRAME_COUNT; i++)
    {
        char path[100];
        sprintf(path, "Images//WalkingWithGun//%d.png", i + 1);
        walkGunFrames[i] = iLoadImage(path);
    }

    for (int i = 0; i < JUMP_FRAME_COUNT; i++)
    {
        char path[100];
        sprintf(path, "Images//Jumping//tile%03d-removebg-preview.png", i + 1);
        jumpFrames[i] = iLoadImage(path);
    }

    for (int i = 0; i < JUMP_GUN_FRAME_COUNT; i++)
    {
        char path[100];
        sprintf(path, "Images//JumpingWithGun//%d.png", i + 1);
        jumpGunFrames[i] = iLoadImage(path);
    }

    gunFrames[GUN_DOWN3] = iLoadImage("Images//PointingGun//pointDown3.png");
    gunFrames[GUN_DOWN2] = iLoadImage("Images//PointingGun//pointDown2.png");
    gunFrames[GUN_DOWN1] = iLoadImage("Images//PointingGun//pointDown1.png");
    gunFrames[GUN_NEUTRAL] = iLoadImage("Images//PointingGun//Neutral.png");
    gunFrames[GUN_UP1] = iLoadImage("Images//PointingGun//pointUp1.png");
    gunFrames[GUN_UP2] = iLoadImage("Images//PointingGun//pointUp2.png");
    gunFrames[GUN_UP3] = iLoadImage("Images//PointingGun//pointUp3.png");

    enemy1.idleTex = iLoadImage("Images//Enemy//1stEnemyFiring.png");
    enemy1.hitTex = iLoadImage("Images//Enemy//1stEnemyTakingDamage.png");
    enemy1.fallTex = iLoadImage("Images//Enemy//1stEnemyFalling.png");
    enemy1.deadTex = 0;

    enemy2.idleTex = iLoadImage("Images//Enemy//2ndEnemyFiring.png");
    enemy2.hitTex = 0;
    enemy2.fallTex = iLoadImage("Images//Enemy//2ndEnemyFalling.png");
    enemy2.deadTex = iLoadImage("Images//Enemy//2ndEnemyDied.png");

    iSetTimer(20, fixedUpdate);

    glutKeyboardUpFunc(keyRelease);
    glutSpecialUpFunc(specialKeyRelease);

    iStart();

    return 0;
}