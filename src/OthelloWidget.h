/*
  OthelloWidget.h
*/

#ifndef __OTHELLOWIDGET_H__
#define __OTHELLOWIDGET_H__

#define CELL_WH 32                      // cell width height
#define CELL_RC 8                       // cell rows columns
#define BOARD_WH (CELL_WH * CELL_RC)    // board width height
#define RIGHT_W 200                     // right status width
#define FONT_WO 0                       // font width offset
#define FONT_HO 6                       // font height offset
#define FONT_H 24                       // font height
#define BOTTOM_H FONT_H                 // bottom status height
#define SIZE_W (BOARD_WH + RIGHT_W)     // window Width
#define SIZE_H (BOARD_WH + BOTTOM_H)    // window Height
#define BUFMAX 1024
#define RGBA_B RGB(0, 0, 0)
#define RGBA_G RGB(32, 128, 32)
#define RGBA_LY RGB(192, 192, 32)
#define RGBA_W RGB(255, 255, 255)

#define BLACK_DISC 0                    // must be 0
#define WHITE_DISC 1                    // must be 1
#define NULL_DISC -1

#define UNDO_MAX 30
#define TURN_MIDDLE 20
#define TURN_FINISH 44                  // must be even number
#define TURN_MAX 60
#define SCAN_DEPTH 8                    // must be even number
#define SCAN_FINDEPTH (TURN_MAX - TURN_FINISH)

#define ReverseTurnDisc(d) do{ d = WHITE_DISC - d; }while(FALSE)

typedef struct _BestInfo{
  BOOL pass;
  BOOL vorb; // TRUE: val or FALSE: best_xy
  int val, x, y;
} BestInfo;

typedef struct _UndoInfo{
  int count;
  int pos[UNDO_MAX];
} UndoInfo;

typedef struct _BoardInfo{
  int board[CELL_RC][CELL_RC];
  int discs[2]; // 0: black, 1: white
  int turnCount;
  int turnDisc;
  BOOL isHumanWhite;
  BOOL inGame;
  BOOL bothPath;
  int depth; // it is >= 0 when thinking
} BoardInfo;

typedef struct _OthelloWidget{
  BOOL threadRunning;
  BOOL show;
  int mode; // 0: novice, 1: easy, 2: medium, 3=<: hard
  HWND hWnd;
  HBRUSH brgreen;
  HBRUSH brblack;
  HBRUSH brwhite;
} OthelloWidget;

void CreateOthelloWidget(HWND hWnd);
void DestroyOthelloWidget();
void SetShowOthelloWidget(BOOL show);
void SetModeOthelloWidget(int mode);
void RedrawOthelloWidget();

void OnUpdateStartBlack();
void OnUpdateStartWhite();
void OnStartBlack();
void OnStartWhite();
void OnLClicked(int cx, int cy);

void CreateBoard(BOOL inGame, BOOL isHumanWhite);
void DisplayBoard(HDC hdc);
void DisplayState(HDC hdc);
void DisplaySearch(BOOL done, int val, int x, int y);
void CheckGameFinished();
int Discs(int disc);

BOOL IsValidTurn(int x, int y);
BOOL IsReversibleDirection(int x, int y, int dx, int dy);
void Reverse(UndoInfo *pUndo, int x, int y);
void ReverseNeighbor(UndoInfo *pUndo, int dx, int dy);
void UnReverse(UndoInfo *pUndo);

int Evaluation();
int EvalDiscs();
int EvalPosition();
int EvalReversiblePos();
int MinMax(BOOL isAI, int depth, BOOL prevpass, int alpha, int beta);
BOOL IsHumanPass();
void TurnCmp();
void RunThread(LPVOID pargs);
UINT __stdcall DoThread(LPVOID pargs);

#endif // __OTHELLOWIDGET_H__
