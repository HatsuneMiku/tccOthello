/*
  tccOthello.c

  compile and link for 'Tiny C Compiler' http://bellard.org/tcc/
  tcc -I../inc tccOthello.c OthelloWidget.c wmdsptch.def
*/

#include <windows.h>

#ifdef _MSC_VER
#if _MSC_VER >= 1500
// 1200 VC6.0
// 1300 VC7.0 VC2003
// 1310 VC7.1 VC2003
// 1400 VC8.0 VC2005
// 1500 VC9.0 VC2008
// 1600 VC10.0 VC2010
#define USE_DWM
#endif
#endif
#ifdef USE_DWM
#include <dwmapi.h>
// use LoadLibrary to run safety on old version OSs
typedef HRESULT (*tDwmIsCompositionEnabled)(BOOL *);
typedef HRESULT (*tDwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND *);
typedef HRESULT (*tDwmExtendFrameIntoClientArea)(HWND, const MARGINS *);
#endif

#include "wmdsptch.h"
#include "OthelloWidget.h"

#define APPNAME "tccOthello"
#define MENU_00 0x0800

char szClsName[] = APPNAME;
char szAppName[] = APPNAME;
HINSTANCE g_hInst;

BOOL RegWndCls(HINSTANCE inst, char *cls, WNDPROC proc)
{
  WNDCLASS wc;
  ZeroMemory(&wc, sizeof(wc));
  wc.hInstance     = inst;
  wc.lpszClassName = cls;
  wc.lpfnWndProc   = proc;
  wc.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
  wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  return RegisterClass(&wc);
}

HWND CreateWnd(char *cls, char *app, int w, int h, HWND parent, HINSTANCE inst)
{
  HWND hWnd = CreateWindow(cls, app,
    WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT,
    w + GetSystemMetrics(SM_CXEDGE) * 2
      + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
    h + GetSystemMetrics(SM_CYEDGE) * 2
      + GetSystemMetrics(SM_CYFIXEDFRAME) * 2
      + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU),
    parent, NULL, inst, NULL);
  return hWnd;
}

HWND RegCreateWnd(HINSTANCE inst, char *cls, WNDPROC proc,
  char *app, int w, int h, HWND parent)
{
  if(RegWndCls(inst, cls, proc)){
    HWND hw = CreateWnd(cls, app, w, h, parent, inst);
    if(hw) return hw;
  }
  return NULL;
}

WINAPI main_OnMenuStart(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if(LOWORD(wParam) - MENU_00 - 1){
    OnUpdateStartWhite();
    OnStartWhite();
  }else{
    OnUpdateStartBlack();
    OnStartBlack();
  }
  return FALSE;
}

WINAPI main_OnMenuToggle(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  SetShowOthelloWidget(WMD_ToggleMenu(GetMenu(hWnd), LOWORD(wParam)));
  return FALSE;
}

WINAPI main_OnMenuLevel(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static UINT ids[] = {12, 14, 16, 18, 0};
  SetModeOthelloWidget(
    WMD_RadioMenu(GetMenu(hWnd), LOWORD(wParam), MENU_00, ids));
  return FALSE;
}

WINAPI main_OnMenuClose(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PostMessage(hWnd, WM_CLOSE, 0, 0);
  return FALSE;
}

static MenuInfo main_mi0[] = {
  {0, "&Game", NULL}, // menu name
  {1, "start &Black", main_OnMenuStart},
  {2, "start &White", main_OnMenuStart},
  {0, "", NULL}, // separator
  {9, "E&xit", main_OnMenuClose},
  {0, NULL, NULL} // terminator
};

static MenuInfo main_mi1sub0[] = {
  {0, "&Level", NULL}, // menu name
  {12, "&Novice", main_OnMenuLevel},
  {14, "&Easy", main_OnMenuLevel},
  {16, "&Medium", main_OnMenuLevel},
  {18, "&Hard", main_OnMenuLevel},
  {0, NULL, NULL} // terminator
};

static MenuInfo main_mi1[] = {
  {0, "&Options", NULL}, // menu name
  {0, "&Level", (PWMHANDLER)main_mi1sub0}, // submenu
  {0, "", NULL}, // separator
  {10, "&Show all search", main_OnMenuToggle},
  {0, NULL, NULL} // terminator
};

static MenuInfo *main_mi[] = {main_mi0, main_mi1, NULL};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WINAPI main_OnCreate(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  HMENU hMenu = WMD_CreateMenu(WndProc, main_mi, MENU_00);
  if(hMenu) SetMenu(hWnd, hMenu);
  DrawMenuBar(hWnd);
  WMD_CenterWindow(hWnd);
  WMD_DarkMenu(hMenu, MENU_00 + 18, FALSE);
  CreateOthelloWidget(hWnd);
#ifdef USE_DWM
  if(TRUE){
    OSVERSIONINFO osVer;
    HMODULE hDwmAPI;
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osVer);
    if(osVer.dwMajorVersion < 6) return FALSE; // Vista
    if(hDwmAPI = LoadLibrary("dwmapi.dll")){
      tDwmIsCompositionEnabled pDwmIsCompositionEnabled =
        (tDwmIsCompositionEnabled)GetProcAddress(
          hDwmAPI, "DwmIsCompositionEnabled");
      tDwmEnableBlurBehindWindow pDwmEnableBlurBehindWindow =
        (tDwmEnableBlurBehindWindow)GetProcAddress(
          hDwmAPI, "DwmEnableBlurBehindWindow");
      tDwmExtendFrameIntoClientArea pDwmExtendFrameIntoClientArea =
        (tDwmExtendFrameIntoClientArea)GetProcAddress(
          hDwmAPI, "DwmExtendFrameIntoClientArea");
      if(pDwmIsCompositionEnabled
      && pDwmEnableBlurBehindWindow
      && pDwmExtendFrameIntoClientArea){
        BOOL fDwmEnabled = FALSE;
#if 0
        DWM_BLURBEHIND bb = {0}; // HRGN hRgnBlur = NULL
        MARGINS margins = {
          GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CXEDGE),
          GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CXEDGE)
            + RIGHT_W,
          GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYEDGE)
            + GetSystemMetrics(SM_CYMENU),
          GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYEDGE)};
#endif
        MARGINS margins = {-1}; // for all client area
        // must receive WM_DWMCOMPOSITIONCHANGED
        pDwmIsCompositionEnabled(&fDwmEnabled);
        if(fDwmEnabled){
#if 0
          // DWM_BB_ENABLE, DWM_BB_BLURREGION, DWM_BB_TRANSITIONONMAXIMIZED
          bb.dwFlags = DWM_BB_ENABLE;
          bb.fEnable = TRUE;
          pDwmEnableBlurBehindWindow(hWnd, &bb);
#endif
          pDwmExtendFrameIntoClientArea(hWnd, &margins);
        }
      }
      FreeLibrary(hDwmAPI);
    }
  }
#endif
  return FALSE;
}

WINAPI main_OnDestroy(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DestroyOthelloWidget();
  PostQuitMessage(0);
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

WINAPI main_OnLButtonDown(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  OnLClicked(LOWORD(lParam), HIWORD(lParam));
  return FALSE;
}

WINAPI main_OnPaint(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  RECT rc;
  HDC hdc = BeginPaint(hWnd, &ps);
  DisplayBoard(hdc);
  EndPaint(hWnd, &ps);
  return DefWindowProc(hWnd, msg, wParam, lParam);
  // return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static BOOL w = FALSE, m = FALSE;
  if(!w){
    w = WMD_CreateDispatcher(WndProc);
    WMD_Bind(WndProc, WM_CREATE, main_OnCreate);
    WMD_Bind(WndProc, WM_DESTROY, main_OnDestroy);
    WMD_Bind(WndProc, WM_LBUTTONDOWN, main_OnLButtonDown);
    WMD_Bind(WndProc, WM_PAINT, main_OnPaint);
  }
  if(!m){
    m = WMD_CreateCommandMenu(WndProc);
  }
  return WMD_Handler(WndProc, msg, wParam, lParam)(hWnd, msg, wParam, lParam);
}

int APIENTRY WinMain(
  HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
  if(RegCreateWnd(g_hInst = hInst, szClsName, (WNDPROC)WndProc,
    szAppName, SIZE_W, SIZE_H, NULL)){
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    return msg.wParam;
  }
  return FALSE;
}
