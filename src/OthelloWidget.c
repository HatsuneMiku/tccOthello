/*
  OthelloWidget.c
*/

#include <windows.h>
#include <process.h>
#include <string.h>
#include <assert.h>

#include "OthelloWidget.h"

static volatile OthelloWidget ow;
static BoardInfo bd;

void CreateOthelloWidget(HWND hWnd)
{
  ow.threadRunning = FALSE;
  ow.show = FALSE;
  ow.mode = 9999;
  ow.hWnd = hWnd;
  ow.brgreen = CreateSolidBrush(RGBA_G);
  ow.brblack = GetStockObject(BLACK_BRUSH);
  ow.brwhite = GetStockObject(WHITE_BRUSH);
  CreateBoard(FALSE, FALSE);
}

void DestroyOthelloWidget()
{
  DeleteObject(ow.brgreen);
}

void SetShowOthelloWidget(BOOL show)
{
  ow.show = show;
  RedrawOthelloWidget();
}

void SetModeOthelloWidget(int mode)
{
  ow.mode = mode;
  RedrawOthelloWidget();
}

void RedrawOthelloWidget()
{
  HDC hdc = GetDC(ow.hWnd);
  DisplayBoard(hdc);
  ReleaseDC(ow.hWnd, hdc);
}

void OnUpdateStartBlack()
{
//  if(ow.threadRunning) disable();
//  else enable;
}

void OnUpdateStartWhite()
{
//  if(ow.threadRunning) disable();
//  else enable();
}

void OnStartBlack()
{
  CreateBoard(TRUE, FALSE);
  RedrawOthelloWidget();
}

void OnStartWhite()
{
  CreateBoard(TRUE, TRUE);
  RedrawOthelloWidget();
  RunThread(TurnCmp);
}

void OnLClicked(int cx, int cy)
{
  int x, y;
  if(!bd.inGame) return;
  if(ow.threadRunning) return;
  x = cx / CELL_WH; y = cy / CELL_WH;
  if(IsValidTurn(x, y)){
    UndoInfo undo;
    Reverse(&undo, x, y);
    bd.turnCount++;
    RedrawOthelloWidget();
    if(bd.inGame) RunThread(TurnCmp);
  }
}

void CreateBoard(BOOL inGame, BOOL isHumanWhite)
{
  int x, y;
  for(y = 0; y < CELL_RC; y++)
    for(x = 0; x < CELL_RC; x++)
      bd.board[y][x] = NULL_DISC;
  bd.board[3][4] = bd.board[4][3] = BLACK_DISC;
  bd.board[3][3] = bd.board[4][4] = WHITE_DISC;
  bd.discs[BLACK_DISC] = bd.discs[WHITE_DISC] = 2;
  bd.turnCount = 0;
  bd.turnDisc = BLACK_DISC;
  bd.isHumanWhite = isHumanWhite;
  bd.inGame = inGame;
  bd.bothPath = FALSE;
  bd.depth = -1;
}

void DisplayBoard(HDC hdc)
{
  int x, y;
  for(y = 0; y < CELL_RC; y++){
    for(x = 0; x < CELL_RC; x++){
      SelectObject(hdc, ow.brgreen);
      Rectangle(hdc, x * CELL_WH, y * CELL_WH,
        (x + 1) * CELL_WH, (y + 1) * CELL_WH);
      if(bd.board[y][x] == NULL_DISC) continue;
      SelectObject(hdc, bd.board[y][x] ? ow.brwhite : ow.brblack); // 1 : 0
      Ellipse(hdc, x * CELL_WH + 2, y * CELL_WH + 2,
        (x + 1) * CELL_WH - 2, (y + 1) * CELL_WH - 2);
    }
  }
  DisplayState(hdc);
  if(bd.inGame) CheckGameFinished();
}

void DisplayState(HDC hdc)
{
  char buf[BUFMAX];
  RECT rc = {0, BOARD_WH, BOARD_WH, BOARD_WH + FONT_H};
  char *t = (bd.inGame && bd.depth >= 0) ? "Thinking" : "          ";
  char *y = (bd.inGame && bd.depth < 0) ? "YourTurn" : "          ";
  SetBkColor(hdc, RGBA_G);
  SetTextColor(hdc, RGBA_LY);
  DrawText(hdc, "        -        ", -1, &rc, DT_CENTER | DT_VCENTER);
  SetTextColor(hdc, RGBA_B);
  sprintf(buf, "%s Black:%2d", bd.isHumanWhite ? t : y, Discs(BLACK_DISC));
  DrawText(hdc, buf, -1, &rc, DT_LEFT | DT_VCENTER);
  SetTextColor(hdc, RGBA_W);
  sprintf(buf, "%2d:White %s", Discs(WHITE_DISC), bd.isHumanWhite ? y : t);
  DrawText(hdc, buf, -1, &rc, DT_RIGHT | DT_VCENTER);

  if(TRUE){
    char *s[] = {"silent", "all search"};
    char *m[] = {"novice", "easy", "medium", "hard"};
    RECT rc = {BOARD_WH, 0, BOARD_WH + SIZE_W, FONT_H * 3};
    SetBkColor(hdc, RGBA_G);
    SetTextColor(hdc, RGBA_LY);
    sprintf(buf, "show:%12s\nmode:%12s\nturnCount:%2d, bothPass:%2d",
      s[ow.show ? 1 : 0],
      m[(ow.mode > sizeof(m) / sizeof(m[0])) ? \
        sizeof(m) / sizeof(m[0]) - 1 : ow.mode],
      bd.turnCount, bd.bothPath);
    DrawText(hdc, buf, -1, &rc, DT_LEFT | DT_VCENTER);
  }
}

void DisplaySearch(BOOL done, int val, int x, int y)
{
  char buf[BUFMAX];
  RECT rc = {x * CELL_WH + FONT_WO, y * CELL_WH + FONT_HO,
    (x + 1) * CELL_WH + FONT_WO, (y + 1) * CELL_WH + FONT_HO};
  HDC hdc = GetDC(ow.hWnd);
  SetBkColor(hdc, RGBA_G);
  if(!done){
    SetTextColor(hdc, RGBA_LY);
    DrawText(hdc, "*", -1, &rc, DT_CENTER | DT_VCENTER);
  }else{
    SetTextColor(hdc, bd.isHumanWhite ? RGBA_B : RGBA_W);
    sprintf(buf, "%3d", val);
    DrawText(hdc, buf, -1, &rc, DT_CENTER | DT_VCENTER);
  }
  DisplayState(hdc);
  ReleaseDC(ow.hWnd, hdc);
}

void CheckGameFinished()
{
  if(bd.bothPath || bd.turnCount == TURN_MAX){
    int n = Discs(WHITE_DISC) - Discs(BLACK_DISC);
    if(!bd.isHumanWhite) n = -n;
    bd.inGame = FALSE;
    MessageBox(ow.hWnd,
      (n > 0) ? "You Win!" : ((n < 0) ? "You Lose!" : "Draw Game!"),
      "Finished", MB_OK);
    // must to reset menus
  }
}

int Discs(int disc)
{
  int x, y, count = 0;
  for(y = 0; y < CELL_RC; y++)
    for(x = 0; x < CELL_RC; x++)
      if(bd.board[y][x] == disc) count++;
  return count;
}

BOOL IsValidTurn(int x, int y)
{
  if(x >= CELL_RC || y >= CELL_RC) return FALSE; // always (x, y) >= 0
  if(bd.board[y][x] != NULL_DISC) return FALSE;
  if(IsReversibleDirection(x, y, -1,  0)) return TRUE; // left
  if(IsReversibleDirection(x, y,  0, -1)) return TRUE; // top
  if(IsReversibleDirection(x, y,  1,  0)) return TRUE; // right
  if(IsReversibleDirection(x, y,  0,  1)) return TRUE; // bottom
  if(IsReversibleDirection(x, y, -1, -1)) return TRUE; // left top
  if(IsReversibleDirection(x, y,  1, -1)) return TRUE; // right top
  if(IsReversibleDirection(x, y, -1,  1)) return TRUE; // left bottom
  if(IsReversibleDirection(x, y,  1,  1)) return TRUE; // right bottom
  return FALSE;
}

BOOL IsReversibleDirection(int x, int y, int dx, int dy)
{
  x += dx; y += dy;
  if(x < 0 || x >= CELL_RC || y < 0 || y >= CELL_RC) return FALSE;
  if(bd.board[y][x] == NULL_DISC) return FALSE;
  if(bd.board[y][x] == bd.turnDisc) return FALSE;
  while(TRUE){
    x += dx; y += dy;
    if(x < 0 || x >= CELL_RC || y < 0 || y >= CELL_RC) return FALSE;
    if(bd.board[y][x] == NULL_DISC) return FALSE;
    if(bd.board[y][x] == bd.turnDisc) return TRUE;
  }
}

void Reverse(UndoInfo *pUndo, int x, int y)
{
  pUndo->count = 0;
  pUndo->pos[0] = x + y * CELL_RC;
  if(IsReversibleDirection(x, y, -1,  0)) ReverseNeighbor(pUndo, -1,  0); // l
  if(IsReversibleDirection(x, y,  0, -1)) ReverseNeighbor(pUndo,  0, -1); // t
  if(IsReversibleDirection(x, y,  1,  0)) ReverseNeighbor(pUndo,  1,  0); // r
  if(IsReversibleDirection(x, y,  0,  1)) ReverseNeighbor(pUndo,  0,  1); // b
  if(IsReversibleDirection(x, y, -1, -1)) ReverseNeighbor(pUndo, -1, -1); // lt
  if(IsReversibleDirection(x, y,  1, -1)) ReverseNeighbor(pUndo,  1, -1); // rt
  if(IsReversibleDirection(x, y, -1,  1)) ReverseNeighbor(pUndo, -1,  1); // lb
  if(IsReversibleDirection(x, y,  1,  1)) ReverseNeighbor(pUndo,  1,  1); // rb
  bd.board[y][x] = bd.turnDisc;
  ReverseTurnDisc(bd.turnDisc);
}

void ReverseNeighbor(UndoInfo *pUndo, int dx, int dy)
{
  int x = pUndo->pos[0] % CELL_RC, y = pUndo->pos[0] / CELL_RC;
  while(bd.board[y += dy][x += dx] != bd.turnDisc){
    bd.board[y][x] = bd.turnDisc;
    pUndo->pos[++pUndo->count] = x + y * CELL_RC;
  }
}

void UnReverse(UndoInfo *pUndo)
{
  int i;
  for(i = pUndo->count; i > 0; --i){
    int *p = &bd.board[pUndo->pos[i] / CELL_RC][pUndo->pos[i] % CELL_RC];
    ReverseTurnDisc(*p);
  }
  bd.board[pUndo->pos[0] / CELL_RC][pUndo->pos[0] % CELL_RC] = NULL_DISC;
  ReverseTurnDisc(bd.turnDisc);
}

int Evaluation()
{
  int val = 0;
  if(bd.turnCount <= TURN_MIDDLE){
    if(ow.mode == 0) val += EvalDiscs();
    if(ow.mode != 1) val += EvalPosition();
    val += EvalReversiblePos();
  }else if(bd.turnCount <= TURN_FINISH){
    if(ow.mode != 1) val += EvalPosition();
    val += EvalReversiblePos();
  }else
    val += EvalDiscs();
  return bd.isHumanWhite ? -val : val;
}

int EvalDiscs()
{
  int x, y, val = 0;
  for(y = 0; y < CELL_RC; y++)
    for(x = 0; x < CELL_RC; x++)
      if(bd.board[y][x] != NULL_DISC)
        val += ((bd.board[y][x] == WHITE_DISC) ? 1 : -1);
  return val; // WhiteDiscs - BlackDiscs
}

int EvalPosition()
{
#if 0
  static const int weight[CELL_RC][CELL_RC] = {0};
  static const int weight[CELL_RC][CELL_RC] = {
    {   8, -12,   2,   0,   0,   2, -12,   8},
    { -12,  -8,   0,   0,   0,   0,  -8, -12},
    {   2,   0,   2,   1,   1,   2,   0,   2},
    {   0,   0,   1,   0,   0,   1,   0,   0},
    {   0,   0,   1,   0,   0,   1,   0,   0},
    {   2,   0,   2,   1,   1,   2,   0,   2},
    { -12,  -8,   0,   0,   0,   0,  -8, -12},
    {   8, -12,   2,   0,   0,   2, -12,   8}};
#endif
  static const int weight[CELL_RC][CELL_RC] = {
    {  45, -11,   4,  -1,  -1,   4, -11,  45},
    { -11, -16,  -1,  -3,  -3,  -1, -16, -11},
    {   4,  -1,   2,  -1,  -1,   2,  -1,   4},
    {  -1,  -3,  -1,   0,   0,  -1,  -3,  -1},
    {  -1,  -3,  -1,   0,   0,  -1,  -3,  -1},
    {   4,  -1,   2,  -1,  -1,   2,  -1,   4},
    { -11, -16,  -1,  -3,  -3,  -1, -16, -11},
    {  45, -11,   4,  -1,  -1,   4, -11,  45}};
  int x, y, val = 0;
  for(y = 0; y < CELL_RC; y++)
    for(x = 0; x < CELL_RC; x++)
      if(bd.board[y][x] != NULL_DISC)
        val += ((bd.board[y][x] == WHITE_DISC) ? 1 : -1) * weight[y][x];
  return val; // WhiteWeights - BlackWeights
}

int EvalReversiblePos()
{
  int x, y, val = 0;
  for(y = 0; y < CELL_RC; y++)
    for(x = 0; x < CELL_RC; x++)
      if(IsValidTurn(x, y)) val++;
  // return (((bd.turnDisc == WHITE_DISC) == bd.isHumanWhite) ? 3 : -3) * val;
  return ((bd.turnDisc == WHITE_DISC) ? 30 : -30) * val;
}

int MinMax(BOOL isAI, int depth, BOOL prevpass, int alpha, int beta)
{
  int x, y, best_x = -1, best_y = -1;
  BOOL pass = TRUE;
  if(!depth) return Evaluation();
  if(isAI) alpha = -9999;
  else beta = 9999;
  for(y = 0; y < CELL_RC; y++){
    for(x = 0; x < CELL_RC; x++){
      if(IsValidTurn(x, y)){
        int val;
        UndoInfo undo;
        Reverse(&undo, x, y);
//        if(ow.show || depth == bd.depth) RedrawOthelloWidget();
        if(ow.show || depth == bd.depth) DisplaySearch(FALSE, 0, x, y);
        val = MinMax(!isAI, depth - 1, pass = FALSE, alpha, beta);
        UnReverse(&undo);
//        if(ow.show || depth == bd.depth) RedrawOthelloWidget();
        if(ow.show || depth == bd.depth) DisplaySearch(TRUE, val, x, y);
        if(isAI){
          if(val >= alpha){ alpha = val; best_x = x; best_y = y; }
          if(alpha > beta) return alpha;
        }else{
          if(val <= beta){ beta = val; best_x = x; best_y = y; }
          if(beta < alpha) return beta;
        }
      }
    }
  }
  if(!pass){
    if(depth == bd.depth) return best_x + best_y * CELL_RC;
    return isAI ? alpha : beta;
  }else{ // pass
    int val;
    if(prevpass) return Evaluation(); // both pass
    ReverseTurnDisc(bd.turnDisc);
    val = MinMax(!isAI, depth - 1, TRUE, alpha, beta);
    ReverseTurnDisc(bd.turnDisc);
    if(depth == bd.depth) return best_x + best_y * CELL_RC; // pass
    return val;
  }
}

BOOL IsHumanPass()
{
  int x, y;
  for(y = 0; y < CELL_RC; y++)
    for(x = 0; x < CELL_RC; x++)
      if(IsValidTurn(x, y)) return FALSE;
  return TRUE;
}

void TurnCmp()
{
  int i = MinMax(TRUE,
    bd.depth = (bd.turnCount >= TURN_FINISH) ? SCAN_FINDEPTH : SCAN_DEPTH,
    FALSE, -9999, 9999);
  bd.depth = -1; // clear status (not thinking)
  if(i < 0 || i >= CELL_RC * CELL_RC){ // check AI pass
    ReverseTurnDisc(bd.turnDisc);
    if(IsHumanPass()){ // check Human pass
      bd.bothPath = TRUE;
      RedrawOthelloWidget();
    }
    return;
  }else{
    UndoInfo undo;
    assert(IsValidTurn(i % CELL_RC, i / CELL_RC));
    Reverse(&undo, i % CELL_RC, i / CELL_RC);
    bd.turnCount++;
    RedrawOthelloWidget();
    if(!bd.inGame) return;
  }
  if(IsHumanPass()){ // check Human pass and take one more AI turn
    ReverseTurnDisc(bd.turnDisc);
    TurnCmp(); // RunThread(TurnCmp);
  }
}

void RunThread(LPVOID pargs)
{
  /*uintptr_t h =*/ _beginthreadex(NULL, 0, DoThread, pargs, 0, NULL);
}

// _beginthreadex : __stdcall, _beginthread: __cdecl
UINT __stdcall DoThread(LPVOID pargs)
{
  void (*pTurnCmp)() = (void (*)())pargs;
  while(ow.threadRunning);
  ow.threadRunning = TRUE;
  pTurnCmp();
  ow.threadRunning = FALSE;
  _endthreadex(0);
  return 0;
}
