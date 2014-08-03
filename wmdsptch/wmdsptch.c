/*
  wmdsptch.c

  WM_HOGE / WM_COMMAND dispatcher

  compile and link for 'Tiny C Compiler' http://bellard.org/tcc/
  tcc -shared -I../inc wmdsptch.c voidhash.c
*/

#include <windows.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "voidhash.h"

#define __WMDSPTCH_MAKE_DLL__
#include "wmdsptch.h"

#ifdef __DEBUG__
#pragma message("defined __DEBUG__")
#define WMD_BUF_MAX 16384
#endif
#define WMD_LOG "debug.log"

// #pragma comment(linker, "/section:.shared,rws")
// #pragma data_seg(".shared")
enum {WMD_w, WMD_m}; // 0: Dispatcher{WindowProc}, 1: CommandMenu{WindowProc}
static VoidHash *vh[] = {NULL, NULL};
// #pragma data_seg()
HINSTANCE hDLLInst;

// DLLEntryPoint #pragma comment(linker, "/ENTRY:\"DllEntryPoint\"")

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpvReserved)
{
  switch(dwReason){
  case DLL_PROCESS_ATTACH:
#ifdef __DEBUG__
    WMD_debug(NULL, "DLL_PROCESS_ATTACH: %08x\n", hInstDLL);
#endif
    hDLLInst = hInstDLL;
    if(!vh[WMD_w]) vh[WMD_w] = vh_alloc(15, 2); // Dispatcher{WindowProc}
    if(!vh[WMD_m]) vh[WMD_m] = vh_alloc(15, 2); // CommandMenu{WindowProc}
#ifdef __DEBUG__
    WMD_debug(NULL, "  vhw: %08x\n", vh[WMD_w]);
    WMD_debug(NULL, "  vhm: %08x\n", vh[WMD_m]);
    WMD_debug(NULL, "--DLL_PROCESS_ATTACH: done\n", hInstDLL);
#endif
    break;
  case DLL_PROCESS_DETACH:
#ifdef __DEBUG__
    WMD_debug(NULL, "DLL_PROCESS_DETACH: %08x\n", hInstDLL);
    if(TRUE){
      char buf[WMD_BUF_MAX];
      WMD_debug(NULL, "CM\n%s", vh_list(vh[WMD_m], buf, sizeof(buf), 0));
      assert(strlen(buf) < WMD_BUF_MAX);
      WMD_debug(NULL, "D\n%s", vh_list(vh[WMD_w], buf, sizeof(buf), 0));
      assert(strlen(buf) < WMD_BUF_MAX);
    }
#endif
    if(vh[WMD_m]) vh_free(vh[WMD_m]);
    if(vh[WMD_w]) vh_free(vh[WMD_w]);
#ifdef __DEBUG__
    WMD_debug(NULL, "--DLL_PROCESS_DETACH: done\n");
#endif
    break;
  case DLL_THREAD_ATTACH:
#ifdef __DEBUG__
    WMD_debug(NULL, "DLL_THREAD_ATTACH: %08x\n", hInstDLL);
#endif
    break;
  case DLL_THREAD_DETACH:
#ifdef __DEBUG__
    WMD_debug(NULL, "DLL_THREAD_DETACH: %08x\n", hInstDLL);
#endif
    break;
  default:
    break;
  }
  return TRUE;
}

__PORT void WMD_create(int wm, PWMHANDLER p)
{
  if(vh[wm]) vh_push(vh[wm], NULL, (uint32_t)p, vh_alloc(27, 0)); // Handler
}

__PORT void WMD_destroy(int wm, PWMHANDLER p)
{
  if(vh[wm]){
    VoidHashContainer *c = vh_pop(vh[wm], NULL, (uint32_t)p);
    if(c) vh_free_container(vh[wm], c);
  }
}

__PORT void WMD_push(int wm, PWMHANDLER p, UINT k, PWMHANDLER f)
{
  if(vh[wm]){
    VoidHashContainer *c = vh_ref(vh[wm], NULL, (uint32_t)p);
    if(c) vh_push((VoidHash *)c->value, NULL, (uint32_t)k, f);
  }
}

__PORT PWMHANDLER WMD_ref(int wm, PWMHANDLER p, UINT k)
{
  if(vh[wm]){
    VoidHashContainer *c = vh_ref(vh[wm], NULL, (uint32_t)p);
    if(c){
      VoidHashContainer *vhc = vh_ref((VoidHash *)c->value, NULL, (uint32_t)k);
      if(vhc) return (PWMHANDLER)vhc->value;
    }
  }
  return NULL;
}

__PORT PWMHANDLER WMD_pop(int wm, PWMHANDLER p, UINT k)
{
  if(vh[wm]){
    VoidHashContainer *c = vh_ref(vh[wm], NULL, (uint32_t)p);
    if(c){
      VoidHashContainer *vhc = vh_pop((VoidHash *)c->value, NULL, (uint32_t)k);
      if(vhc){
        PWMHANDLER f = (PWMHANDLER)vhc->value;
        vh_free_container((VoidHash *)c->value, vhc);
        return f;
      }
    }
  }
  return NULL;
}

__PORT char *WMD_list(int wm, PWMHANDLER p, char *buf, int len)
{
  if(len <= 0) return NULL;
  buf[0] = '\0';
  if(vh[wm]){
    VoidHashContainer *c = vh_ref(vh[wm], NULL, (uint32_t)p);
    if(c) vh_list((VoidHash *)c->value, buf, len, 0);
  }
  return buf;
}

__PORT void *WMD_malloc(size_t sz) // for another segment (using in DLL)
{
  return malloc(sz);
}

__PORT void WMD_free(void *p) // for another segment (using in DLL)
{
  free(p);
}

__PORT void WMD_debug(char *a, char *fmt, ...)
{
  FILE *fp;
  int len = strlen(fmt);
  static int debugmode = TRUE;
  va_list arg;
  va_start(arg, fmt);
  if(!a && !len) debugmode = !debugmode;
  if(debugmode && (fp = fopen(WMD_LOG, !len ? "wb" : "ab"))){
    time_t t;
    struct tm *p;
    time(&t);
    p = localtime(&t);
    fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d [%s] ",
      p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
      p->tm_hour, p->tm_min, p->tm_sec, a ? a : "");
    vfprintf(fp, !len ? "\n" : fmt, arg);
    fclose(fp);
  }
  va_end(arg);
}

__PORT BOOL WMD_CreateDispatcher(PWMHANDLER p)
{
  WMD_create(WMD_w, p);
#ifdef __DEBUG__
  if(TRUE){
    char buf[WMD_BUF_MAX];
    WMD_debug(NULL, "D\n%s", vh_list(vh[WMD_w], buf, sizeof(buf), 0));
    assert(strlen(buf) < WMD_BUF_MAX);
  }
#endif
  return TRUE;
}

__PORT BOOL WMD_CreateCommandMenu(PWMHANDLER p)
{
  WMD_create(WMD_m, p);
#ifdef __DEBUG__
  if(TRUE){
    char buf[WMD_BUF_MAX];
    WMD_debug(NULL, "CM\n%s", vh_list(vh[WMD_m], buf, sizeof(buf), 0));
    assert(strlen(buf) < WMD_BUF_MAX);
  }
#endif
  return TRUE;
}

__PORT BOOL WMD_DestroyDispatcher(PWMHANDLER p)
{
  WMD_destroy(WMD_w, p);
#ifdef __DEBUG__
  if(TRUE){
    char buf[WMD_BUF_MAX];
    WMD_debug(NULL, "D\n%s", vh_list(vh[WMD_w], buf, sizeof(buf), 0));
    assert(strlen(buf) < WMD_BUF_MAX);
  }
#endif
  return TRUE;
}

__PORT BOOL WMD_DestroyCommandMenu(PWMHANDLER p)
{
  WMD_destroy(WMD_m, p);
#ifdef __DEBUG__
  if(TRUE){
    char buf[WMD_BUF_MAX];
    WMD_debug(NULL, "CM\n%s", vh_list(vh[WMD_m], buf, sizeof(buf), 0));
    assert(strlen(buf) < WMD_BUF_MAX);
  }
#endif
  return TRUE;
}

__PORT BOOL WMD_Bind(PWMHANDLER p, UINT id, PWMHANDLER f)
{
  WMD_push(WMD_w, p, id, f);
#ifdef __DEBUG__
  if(TRUE){
    char buf[WMD_BUF_MAX];
    WMD_debug(NULL, "==Bind\n%s", WMD_list(WMD_w, p, buf, sizeof(buf)));
    assert(strlen(buf) < WMD_BUF_MAX);
  }
#endif
  return TRUE;
}

__PORT BOOL WMD_CMBind(PWMHANDLER p, UINT id, PWMHANDLER f)
{
  WMD_push(WMD_m, p, id, f);
#ifdef __DEBUG__
  if(TRUE){
    char buf[WMD_BUF_MAX];
    WMD_debug(NULL, "==CMBind\n%s", WMD_list(WMD_m, p, buf, sizeof(buf)));
    assert(strlen(buf) < WMD_BUF_MAX);
  }
#endif
  return TRUE;
}

__PORT BOOL WMD_UnBind(PWMHANDLER p, UINT id)
{
  WMD_pop(WMD_w, p, id);
#ifdef __DEBUG__
  if(TRUE){
    char buf[WMD_BUF_MAX];
    WMD_debug(NULL, "==Bind\n%s", WMD_list(WMD_w, p, buf, sizeof(buf)));
    assert(strlen(buf) < WMD_BUF_MAX);
  }
#endif
  return TRUE;
}

__PORT BOOL WMD_CMUnBind(PWMHANDLER p, UINT id)
{
  WMD_pop(WMD_m, p, id);
#ifdef __DEBUG__
  if(TRUE){
    char buf[WMD_BUF_MAX];
    WMD_debug(NULL, "==CMBind\n%s", WMD_list(WMD_m, p, buf, sizeof(buf)));
    assert(strlen(buf) < WMD_BUF_MAX);
  }
#endif
  return TRUE;
}

__PORT PWMHANDLER WMD_Handler(PWMHANDLER p, UINT id, WPARAM w, LPARAM l)
{
  PWMHANDLER f = WMD_ref((id == WM_COMMAND) ? WMD_m : WMD_w, p,
    (id == WM_COMMAND) ? LOWORD(w) : id);
  if(f) return f;
  return DefWindowProc;
}

__PORT HMENU WMD_CreateMenu(PWMHANDLER p, MenuInfo **pmi, UINT offset)
{
  int i;
  HMENU menu = CreateMenu();
  for(i = 0; pmi[i]; i++){
    MenuInfo *mi = pmi[i];
    AppendMenu(menu,
      MF_STRING | (mi[0].id ? 0 : MF_POPUP),
      mi[0].id ? (offset + mi[0].id) :
        (UINT_PTR)WMD_CreatePopupMenu(p, mi, offset),
      mi[0].name);
    if(mi[0].id && mi[0].handler)
      WMD_CMBind(p, offset + mi[0].id, mi[0].handler);
  }
  return menu;
}

__PORT HMENU WMD_CreatePopupMenu(PWMHANDLER p, MenuInfo *mi, UINT offset)
{
  int i;
  HMENU menu = CreatePopupMenu();
  for(i = 1; mi[i].id != 0 || mi[i].name != NULL; i++){
    BOOL sep = (!strlen(mi[i].name) || !mi[i].handler);
    InsertMenu(menu, i - 1,
      MF_BYPOSITION | (sep ? MF_SEPARATOR : \
        (MF_STRING | (mi[i].id ? 0 : MF_POPUP))),
      sep ? offset : (mi[i].id ? (offset + mi[i].id) : \
        (UINT_PTR)WMD_CreatePopupMenu(p, (MenuInfo *)mi[i].handler, offset)),
      mi[i].name);
    if(mi[i].id && mi[i].handler)
      WMD_CMBind(p, offset + mi[i].id, mi[i].handler);
  }
  return menu;
}

__PORT void WMD_DarkMenu(HMENU hMenu, UINT id, BOOL stat)
{
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  GetMenuItemInfo(hMenu, id, FALSE, &mii);
  mii.fState = stat ? MFS_ENABLED : MFS_DISABLED;
  SetMenuItemInfo(hMenu, id, FALSE, &mii);
}

__PORT BOOL WMD_ToggleMenu(HMENU hMenu, UINT id)
{
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  GetMenuItemInfo(hMenu, id, FALSE, &mii);
  mii.fState = (mii.fState == MFS_CHECKED) ? MFS_UNCHECKED : MFS_CHECKED;
  SetMenuItemInfo(hMenu, id, FALSE, &mii);
  return mii.fState == MFS_CHECKED;
}

__PORT UINT WMD_RadioMenu(HMENU hMenu, UINT id, UINT offset, UINT *ids)
{
  UINT i, r = 0;
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  for(i = 0; ids[i]; i++){
    UINT j = offset + ids[i];
    GetMenuItemInfo(hMenu, j, FALSE, &mii);
    if(mii.fState != MFS_DISABLED){
      mii.fState = (j == id) ? ((r = i), MFS_CHECKED) : MFS_UNCHECKED;
      SetMenuItemInfo(hMenu, j, FALSE, &mii);
    }
  }
  return r;
}

__PORT void WMD_CenterWindow(HWND hWnd)
{
  RECT rw_self, rc_parent, rw_parent;
  HWND hWnd_parent = GetParent(hWnd);
  if(NULL == hWnd_parent) hWnd_parent = GetDesktopWindow();
  GetWindowRect(hWnd_parent, &rw_parent);
  GetClientRect(hWnd_parent, &rc_parent);
  GetWindowRect(hWnd, &rw_self);
  SetWindowPos(hWnd, NULL,
    rw_parent.left + (rc_parent.right + rw_self.left - rw_self.right) / 2,
    rw_parent.top + (rc_parent.bottom + rw_self.top - rw_self.bottom) / 2,
    0, 0,
    SWP_NOSIZE | SWP_NOZORDER|SWP_NOACTIVATE);
}
