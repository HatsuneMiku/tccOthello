/*
  voidhash.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "voidhash.h"

uint32_t vh_hashstr(VoidHash *vh, char *str)
{
  return ((str[0] & (unsigned char)0xDF) - '@') % vh->len;
}

uint32_t vh_hashkey(VoidHash *vh, uint32_t key)
{
  return key % vh->len;
}

VoidHashContainer *vh_push(VoidHash *vh, char *str, uint32_t key, void *value)
{
  uint32_t h = str ? vh_hashstr(vh, str) : vh_hashkey(vh, key);
  VoidHashContainer *q = NULL;
  VoidHashContainer *p = vh->vhc[h];
  VoidHashContainer *c = (VoidHashContainer *)
    malloc(sizeof(VoidHashContainer));
  if(!c) return NULL;
  c->hash = h;
  if(str){
    size_t n = strlen(str);
    c->str = (char *)malloc(n + 1);
    if(!c->str){ free(c); return NULL; }
    strncpy(c->str, str, n);
    c->str[n] = '\0';
  }else{
    c->str = NULL;
  }
  c->key = key;
  c->value = value;
  c->prev = c->next = NULL;
  if(!p) return vh->vhc[h] = c;
  while(p){
    int cmp = str ? (p->str ? strcmp(str, p->str) : 1) : \
      (!p->str ? (key - p->key) : 1); // 1 means skip
    if(cmp > 0){
      q = p;
      p = p->next;
    }else if(cmp < 0){
      if(!p->prev) vh->vhc[h] = c;
      else (c->prev = p->prev)->next = c;
      return (c->next = p)->prev = c;
    }else{ // cmp == 0
      if(!p->prev) vh->vhc[h] = c;
      c->prev = p->prev;
      c->next = p->next;
      vh_free_container(vh, p); // no care prev/next links
      if(c->prev) c->prev->next = c;
      if(c->next) c->next->prev = c;
      return c;
    }
  }
  return (c->prev = q)->next = c;
}

VoidHashContainer *vh_ref(VoidHash *vh, char *str, uint32_t key)
{
  uint32_t h = str ? vh_hashstr(vh, str) : vh_hashkey(vh, key);
  VoidHashContainer *p = vh->vhc[h];
  if(str){
    while(p){
      if(p->str){
        int cmp = strcmp(str, p->str);
        if(!cmp) return p;
        else if(cmp < 0) return NULL;
      }
      p = p->next;
    }
  }else{
    while(p){
      if(!p->str){
        int cmp = key - p->key;
        if(!cmp) return p;
        else if(cmp < 0) return NULL;
      }
      p = p->next;
    }
  }
  return NULL;
}

VoidHashContainer *vh_pop(VoidHash *vh, char *str, uint32_t key) // need free
{
  VoidHashContainer *p = vh_ref(vh, str, key);
  if(!p) return NULL;
  if(!p->prev){
    uint32_t h = str ? vh_hashstr(vh, str) : vh_hashkey(vh, key);
    vh->vhc[h] = p->next;
    if(p->next) p->next->prev = NULL;
  }else{
    p->prev->next = p->next;
    if(p->next) p->next->prev = p->prev;
  }
  return p;
}

uint32_t vh_first(VoidHash *vh, VoidHashInfo *vhi)
{
  vhi->hash = 0;
  vhi->vhc = NULL;
  for( ; vhi->hash < vh->len; vhi->hash++)
    if(vhi->vhc = vh->vhc[vhi->hash]) return 1;
  return 0;
}

uint32_t vh_next(VoidHash *vh, VoidHashInfo *vhi)
{
  if(!vhi->vhc) return 0;
  if(vhi->vhc = vhi->vhc->next) return 1;
  for( ; ++vhi->hash < vh->len; )
    if(vhi->vhc = vh->vhc[vhi->hash]) return 1;
  return 0;
}

VoidHashContainer *vh_linearsearch(VoidHash *vh, void *value, uint32_t asstr)
{
  VoidHashInfo vhi;
  if(vh_first(vh, &vhi))
    do{
      if((asstr && value) ? \
      (vhi.vhc->value ? !strcmp((char *)vhi.vhc->value, (char *)value) : 0) \
      : (vhi.vhc->value == value)) // ? vh->freewithvalue 0:no 1:str 2:hash
      // (*(uint32_t *)vhi.vhc->value == *(uint32_t *)value))
        return vhi.vhc;
    }while(vh_next(vh, &vhi));
  return NULL;
}

uint32_t vh_len(VoidHash *vh)
{
  return vh->len;
}

uint32_t vh_count(VoidHash *vh)
{
  VoidHashInfo vhi;
  uint32_t count = 0;
  if(vh_first(vh, &vhi))
    do{
      ++count;
    }while(vh_next(vh, &vhi));
  return count;
}

char *vh_list(VoidHash *vh, char *buf, uint32_t len, uint32_t asstr) // value
{
  VoidHashInfo vhi;
  if(len <= 0) return NULL;
  buf[0] = '\0';
  if(vh_first(vh, &vhi))
    do{
      char sb[256];
      if(vh->freewithvalue == 2){
        VoidHash *p = (VoidHash *)vhi.vhc->value;
        sprintf(sb, "vh%4d/%2d", vh_count(p), vh_len(p));
      }
      sprintf(buf + strlen(buf), // ? vh->freewithvalue 0:no 1:str 2:hash
        "l:%2d, p:%08x, h:%2d, s:%-9s, k:%2d, v:%-9s, a:%08x\n",
        vh->len, vhi.vhc, vhi.vhc->hash, vhi.vhc->str, vhi.vhc->key,
        (vh->freewithvalue == 2) ? sb : \
          (asstr ? (char *)vhi.vhc->value : "notstring"),
        vhi.vhc->value);
    }while(vh_next(vh, &vhi));
  return buf;
}

VoidHash *vh_alloc(uint32_t len, uint32_t freewithvalue) // 0:no 1:str 2:hash
{
  uint32_t h;
  VoidHash *vh = (VoidHash *)malloc(sizeof(VoidHash));
  if(!vh) return NULL;
  vh->len = len;
  vh->freewithvalue = freewithvalue;
  vh->vhc = (VoidHashContainer **)malloc(sizeof(VoidHashContainer *) * len);
  if(!vh->vhc){ free(vh); return NULL; }
  for(h = 0; h < len; h++) vh->vhc[h] = NULL;
  return vh;
}

void vh_free_container(VoidHash *vh, VoidHashContainer *vhc) // pop released
{
  if(vhc->str) free(vhc->str);
  if(vh->freewithvalue && vhc->value){ // 0:no 1:str 2:hash
    if(vh->freewithvalue == 2) vh_free((VoidHash *)vhc->value);
    else free(vhc->value);
  }
  free(vhc);
}

void vh_flush(VoidHash *vh)
{
  if(vh && vh->vhc){
    uint32_t h;
    for(h = 0; h < vh->len; h++){
      VoidHashContainer *c = vh->vhc[h];
      while(c){
        VoidHashContainer *p = c->next;
        vh_free_container(vh, c); // no care prev/next links
        c = p;
      }
      vh->vhc[h] = NULL;
    }
  }
}

void vh_free(VoidHash *vh)
{
  vh_flush(vh);
  if(vh){
    if(vh->vhc) free(vh->vhc);
    free(vh);
  }
}

char *vh_randalnum(char *buf, uint32_t len)
{
  int i;
  for(i = 0; i < len; i++){
    int r = rand() % (26 + 26 + (i > 0 ? 10 : 0));
    if(r < 26) buf[i] = r + 'A';
    else if(r < (26 + 26)) buf[i] = r - 26 + 'a';
    else buf[i] = r - (26 + 26) + '0';
  }
  buf[i] = '\0';
  return buf;
}
