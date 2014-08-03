/*
  voidhash.h
*/

#ifndef __VOIDHASH_H__
#define __VOIDHASH_H__

#ifdef _MSC_VER
#if _MSC_VER < 1600
// 1200 VC6.0
// 1300 VC7.0 VC2003
// 1310 VC7.1 VC2003
// 1400 VC8.0 VC2005
// 1500 VC9.0 VC2008
// 1600 VC10.0 VC2010
typedef unsigned int uint32_t;
#endif
#else
#ifdef __GNUC__
typedef unsigned int uint32_t;
#else
// typedef unsigned int32_t uint32_t;
typedef int32_t uint32_t;
#endif
#endif

typedef struct _VoidHashContainer{
  uint32_t hash;
  char *str;
  uint32_t key;
  void *value;
  struct _VoidHashContainer *prev;
  struct _VoidHashContainer *next;
} VoidHashContainer;

typedef struct _VoidHash{
  uint32_t len;
  uint32_t freewithvalue;
  VoidHashContainer **vhc;
} VoidHash;

typedef struct _VoidHashInfo{
  uint32_t hash;
  VoidHashContainer *vhc;
} VoidHashInfo;

uint32_t vh_hashstr(VoidHash *vh, char *str);
uint32_t vh_hashkey(VoidHash *vh, uint32_t key);
VoidHashContainer *vh_push(VoidHash *vh, char *str, uint32_t key, void *value);
VoidHashContainer *vh_ref(VoidHash *vh, char *str, uint32_t key);
VoidHashContainer *vh_pop(VoidHash *vh, char *str, uint32_t key); // need free
uint32_t vh_first(VoidHash *vh, VoidHashInfo *vhi);
uint32_t vh_next(VoidHash *vh, VoidHashInfo *vhi);
VoidHashContainer *vh_linearsearch(VoidHash *vh, void *value, uint32_t asstr);
uint32_t vh_len(VoidHash *vh);
uint32_t vh_count(VoidHash *vh);
char *vh_list(VoidHash *vh, char *buf, uint32_t len, uint32_t asstr); // value

VoidHash *vh_alloc(uint32_t len, uint32_t freewithvalue); // 0:no 1:str 2:hash
void vh_free_container(VoidHash *vh, VoidHashContainer *vhc); // pop released
void vh_flush(VoidHash *vh);
void vh_free(VoidHash *vh);

char *vh_randalnum(char *buf, uint32_t len);

#endif // __VOIDHASH_H__
