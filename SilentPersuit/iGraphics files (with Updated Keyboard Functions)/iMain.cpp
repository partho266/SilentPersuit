#include "iGraphics.h"
#include<stdio.h>
#include<string.h>

// ---------------------------------------------------------
// SILENT PURSUIT - Loading Screen + Start Screen Template
// ---------------------------------------------------------

// ---------------- Game States ----------------
enum GameState { LOADING, MENU, OPTIONS, CREDITS, GAMEPLAY };
GameState state = LOADING;

// ---------------- Window Size ----------------
const int WIN_W = 800;
const int WIN_H = 600;

// ---------------- Loading Screen ----------------
int loadingProgress = 0;      // 0 to 100
bool loadingDone = false;

// ---------------- Background Textures ----------------
unsigned int loadingBgTexture;
// Add more here later, e.g:
// unsigned int menuBgTexture;

// ---------------- Simple Button Struct ----------------
struct Button {
	double x, y, w, h;      // x,y = bottom-left corner
	char label[30];
	bool hovered;
};

// Main Menu buttons
Button btnNewGame = { 300, 380, 200, 50, "New Game", false };
Button btnOptions = { 300, 310, 200, 50, "Options",  false };
Button btnCredits = { 300, 240, 200, 50, "Credits",  false };
Button btnQuit = { 300, 170, 200, 50, "Quit Game",false };

// Options submenu buttons
Button btnAudio = { 300, 380, 200, 50, "Audio",    false };
Button btnControls = { 300, 310, 200, 50, "Controls", false };
Button btnOptBack = { 300, 240, 200, 50, "Back",     false };

// Credits back button
Button btnCredBack = { 300, 80,  200, 50, "Back",     false };

// ---------------- Helper: point inside button? ----------------
bool isInside(Button b, int mx, int my)
{
	return (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h);
}

// ---------------- Helper: draw one button ----------------
void drawButton(Button b)
{
	if (b.hovered)
		iSetColor(90, 90, 90);
	else
		iSetColor(50, 50, 50);
	iFilledRectangle(b.x, b.y, b.w, b.h);

	iSetColor(255, 255, 255);
	iRectangle(b.x, b.y, b.w, b.h);

	// crude centering of text based on label length
	int textX = b.x + b.w / 2 - strlen(b.label) * 4;
	int textY = b.y + b.h / 2 - 5;
	iText(textX, textY, b.label, GLUT_BITMAP_HELVETICA_18);
}

// ===========================================================
//                          iDraw()
// ===========================================================
void iDraw()
{
	iClear();

	if (state == LOADING)
	{
		// ---- Background image (JPEG/PNG via iLoadImage/iShowImage) ----
		iShowImage(0, 0, WIN_W, WIN_H, loadingBgTexture);

		// ---- Title ----
		iSetColor(255, 255, 255);
		iText(WIN_W / 2 - 90, WIN_H / 2 + 80, "SILENT PURSUIT", GLUT_BITMAP_TIMES_ROMAN_24);

		// ---- Loading bar outline ----
		int barW = 400, barH = 30;
		int barX = WIN_W / 2 - barW / 2, barY = WIN_H / 2 - 20;
		iSetColor(255, 255, 255);
		iRectangle(barX, barY, barW, barH);

		// ---- Loading bar fill ----
		iSetColor(0, 200, 90);
		iFilledRectangle(barX, barY, (barW * loadingProgress) / 100, barH);

		// ---- Percentage text ----
		char buf[20];
		sprintf(buf, "Loading... %d%%", loadingProgress);
		iSetColor(255, 255, 255);
		iText(barX, barY - 25, buf, GLUT_BITMAP_HELVETICA_18);
	}
	else if (state == MENU)
	{
		// ---- Background (replace with your own image later) ----
		iSetColor(20, 20, 30);
		iFilledRectangle(0, 0, WIN_W, WIN_H);

		iSetColor(255, 255, 255);
		iText(WIN_W / 2 - 90, 500, "SILENT PURSUIT", GLUT_BITMAP_TIMES_ROMAN_24);

		drawButton(btnNewGame);
		drawButton(btnOptions);
		drawButton(btnCredits);
		drawButton(btnQuit);
	}
	else if (state == OPTIONS)
	{
		iSetColor(20, 20, 30);
		iFilledRectangle(0, 0, WIN_W, WIN_H);

		iSetColor(255, 255, 255);
		iText(WIN_W / 2 - 40, 500, "OPTIONS", GLUT_BITMAP_TIMES_ROMAN_24);

		drawButton(btnAudio);
		drawButton(btnControls);
		drawButton(btnOptBack);
	}
	else if (state == CREDITS)
	{
		iSetColor(20, 20, 30);
		iFilledRectangle(0, 0, WIN_W, WIN_H);

		iSetColor(255, 255, 255);
		iText(WIN_W / 2 - 50, 500, "CREDITS", GLUT_BITMAP_TIMES_ROMAN_24);

		iText(WIN_W / 2 - 150, 420, "Game Design & Programming:", GLUT_BITMAP_HELVETICA_18);
		iText(WIN_W / 2 - 100, 390, "Partho Das", GLUT_BITMAP_HELVETICA_18);
		iText(WIN_W / 2 - 100, 360, "Khadija Bibi Sumona", GLUT_BITMAP_HELVETICA_18);
		iText(WIN_W / 2 - 100, 330, "Shahriar Mostafa Razin", GLUT_BITMAP_HELVETICA_18);

		iText(WIN_W / 2 - 150, 280, "Built with iGraphics", GLUT_BITMAP_HELVETICA_18);

		drawButton(btnCredBack);
	}
	else if (state == GAMEPLAY)
	{
		// Placeholder - your actual game screen goes here
		iSetColor(30, 60, 30);
		iFilledRectangle(0, 0, WIN_W, WIN_H);
		iSetColor(255, 255, 255);
		iText(WIN_W / 2 - 120, WIN_H / 2, "GAMEPLAY GOES HERE", GLUT_BITMAP_TIMES_ROMAN_24);
	}
}

// ===========================================================
//                       iMouseMove()
// ===========================================================
void iMouseMove(int mx, int my)
{

}

// ===========================================================
//                    iPassiveMouseMove()
// (used here to update hover state on buttons)
// ===========================================================
void iPassiveMouseMove(int mx, int my)
{
	if (state == MENU)
	{
		btnNewGame.hovered = isInside(btnNewGame, mx, my);
		btnOptions.hovered = isInside(btnOptions, mx, my);
		btnCredits.hovered = isInside(btnCredits, mx, my);
		btnQuit.hovered = isInside(btnQuit, mx, my);
	}
	else if (state == OPTIONS)
	{
		btnAudio.hovered = isInside(btnAudio, mx, my);
		btnControls.hovered = isInside(btnControls, mx, my);
		btnOptBack.hovered = isInside(btnOptBack, mx, my);
	}
	else if (state == CREDITS)
	{
		btnCredBack.hovered = isInside(btnCredBack, mx, my);
	}
}

// ===========================================================
//                          iMouse()
// ===========================================================
void iMouse(int button, int mstate, int mx, int my)
{
	if (button == GLUT_LEFT_BUTTON && mstate == GLUT_DOWN)
	{
		if (state == MENU)
		{
			if (isInside(btnNewGame, mx, my))
			{
				state = GAMEPLAY;   // TODO: hook up your actual game start
			}
			else if (isInside(btnOptions, mx, my))
			{
				state = OPTIONS;
			}
			else if (isInside(btnCredits, mx, my))
			{
				state = CREDITS;
			}
			else if (isInside(btnQuit, mx, my))
			{
				exit(0);
			}
		}
		else if (state == OPTIONS)
		{
			if (isInside(btnAudio, mx, my))
			{
				// TODO: open audio settings sub-panel
			}
			else if (isInside(btnControls, mx, my))
			{
				// TODO: open controls settings sub-panel
			}
			else if (isInside(btnOptBack, mx, my))
			{
				state = MENU;
			}
		}
		else if (state == CREDITS)
		{
			if (isInside(btnCredBack, mx, my))
			{
				state = MENU;
			}
		}
	}
}

// ===========================================================
//                        iKeyboard()
// ===========================================================
void iKeyboard(unsigned char key)
{
	if (key == 27) // ESC key
	{
		if (state == OPTIONS || state == CREDITS)
			state = MENU;
	}
}

// Special Keys:
// GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6, GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12,
// GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP, GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT
void iSpecialKeyboard(unsigned char key)
{

}

// ===========================================================
//                       fixedUpdate()
// (called repeatedly - used here to advance loading bar)
// ===========================================================
void fixedUpdate()
{
	if (state == LOADING && !loadingDone)
	{
		loadingProgress++;
		if (loadingProgress >= 100)
		{
			loadingProgress = 100;
			loadingDone = true;
			state = MENU;   // auto-transition to Start Screen
		}
	}
}

// ===========================================================
//                          main()
// ===========================================================
int main()
{
	// Load audio (uncomment and set your files once ready)
	// mciSendString("open \"Audios//background.mp3\" alias bgsong", NULL, 0, NULL);
	// mciSendString("play bgsong repeat", NULL, 0, NULL);

	iInitialize(WIN_W, WIN_H, "Silent Pursuit");

	// Load background images AFTER iInitialize() - needs an active OpenGL context
	loadingBgTexture = iLoadImage("Images//loading_bg.jpeg");

	iStart();
	return 0;
}
