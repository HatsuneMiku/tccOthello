/*
  wmdsptch.h

  WM_HOGE / WM_COMMAND dispatcher
*/

#ifndef __WMDSPTCH_H__
#define __WMDSPTCH_H__

#if defined(__cplusplus)
extern "C"{
#endif

#ifdef __WMDSPTCH_MAKE_DLL__
#define __PORT __declspec(dllexport) // make dll mode
#else
#define __PORT __declspec(dllimport) // use dll mode
#endif

typedef LRESULT (CALLBACK *PWMHANDLER)(HWND, UINT, WPARAM, LPARAM);

typedef struct _MenuInfo{
  UINT id;
  char *name;
  PWMHANDLER handler;
} MenuInfo;

__PORT void WMD_create(int wm, PWMHANDLER p);
__PORT void WMD_destroy(int wm, PWMHANDLER p);
__PORT void WMD_push(int wm, PWMHANDLER p, UINT k, PWMHANDLER f);
__PORT PWMHANDLER WMD_ref(int wm, PWMHANDLER p, UINT k);
__PORT PWMHANDLER WMD_pop(int wm, PWMHANDLER p, UINT k);
__PORT char *WMD_list(int wm, PWMHANDLER p, char *buf, int len);

__PORT void *WMD_malloc(size_t sz); // for another segment (using in DLL)
__PORT void WMD_free(void *p); // for another segment (using in DLL)
__PORT void WMD_debug(char *a, char *fmt, ...);

__PORT BOOL WMD_CreateDispatcher(PWMHANDLER p);
__PORT BOOL WMD_CreateCommandMenu(PWMHANDLER p);
__PORT BOOL WMD_DestroyDispatcher(PWMHANDLER p);
__PORT BOOL WMD_DestroyCommandMenu(PWMHANDLER p);
__PORT BOOL WMD_Bind(PWMHANDLER p, UINT id, PWMHANDLER f);
__PORT BOOL WMD_CMBind(PWMHANDLER p, UINT id, PWMHANDLER f);
__PORT BOOL WMD_UnBind(PWMHANDLER p, UINT id);
__PORT BOOL WMD_CMUnBind(PWMHANDLER p, UINT id);
__PORT PWMHANDLER WMD_Handler(PWMHANDLER p, UINT id, WPARAM w, LPARAM l);
__PORT HMENU WMD_CreateMenu(PWMHANDLER p, MenuInfo **pmi, UINT offset);
__PORT HMENU WMD_CreatePopupMenu(PWMHANDLER p, MenuInfo *mi, UINT offset);

__PORT void WMD_DarkMenu(HMENU hMenu, UINT id, BOOL stat);
__PORT BOOL WMD_ToggleMenu(HMENU hMenu, UINT id);
__PORT UINT WMD_RadioMenu(HMENU hMenu, UINT id, UINT offset, UINT *ids);
__PORT void WMD_CenterWindow(HWND hWnd);

#if defined(__cplusplus)
};
#endif

#endif // __WMDSPTCH_H__
