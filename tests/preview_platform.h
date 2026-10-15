/* Host capture of the actual native rendering functions; never linked on 3DS. */
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
typedef uint32_t u32;typedef uint8_t u8;typedef uint64_t u64;
typedef void C3D_RenderTarget;typedef void *C2D_TextBuf;
typedef struct {int cellHeight;} TGLP_s;
typedef struct {int height;TGLP_s *tglp;} FINF_s;
typedef struct {int charWidth;} charWidthInfo_s;
typedef struct {unsigned char *data;FINF_s info;TGLP_s glyph;int id;} PreviewFont;
typedef PreviewFont *C2D_Font;
typedef struct {C2D_Font font;const char *s;} C2D_Text;
#define C2D_WithColor 1
#define C2D_AtBaseline 2
static unsigned read16(const unsigned char *p){return p[0]|p[1]<<8;}
static unsigned read32(const unsigned char *p){return read16(p)|(read16(p+2)<<16);}
static C2D_Font preview_font(const char *path,int id){
 FILE *f=fopen(path,"rb");if(!f)abort();fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);
 C2D_Font font=calloc(1,sizeof(*font));font->data=malloc(n);if(fread(font->data,1,n,f)!=(size_t)n)abort();fclose(f);
 font->id=id;font->info.height=font->data[48];font->glyph.cellHeight=font->data[read32(font->data+36)+1];font->info.tglp=&font->glyph;return font;
}
static FINF_s *C2D_FontGetInfo(C2D_Font font){return &font->info;}
static int C2D_FontGlyphIndexFromCodePoint(C2D_Font font,u32 cp){
 const unsigned char *d=font->data;unsigned at=read32(d+44);
 while(at){unsigned first=read16(d+at),last=read16(d+at+2),method=read16(d+at+4);
  if(cp>=first&&cp<=last){if(method==0)return read16(d+at+12)+cp-first;
   if(method==1)return read16(d+at+12+2*(cp-first));
   if(method==2){int count=read16(d+at+12);for(int i=0;i<count;i++)if(read16(d+at+14+i*4)==cp)return read16(d+at+16+i*4);}}
  at=read32(d+at+8);
 }return read16(d+30);
}
static charWidthInfo_s *C2D_FontGetCharWidthInfo(C2D_Font font,int index){
 static charWidthInfo_s result;const unsigned char *d=font->data;unsigned at=read32(d+40);result.charWidth=d[34];
 while(at){unsigned first=read16(d+at),last=read16(d+at+2);if(index>=(int)first&&index<=(int)last){result.charWidth=d[at+8+3*(index-first)+2];break;}at=read32(d+at+4);}return &result;
}
static u32 C2D_Color32(unsigned r,unsigned g,unsigned b,unsigned a){return (r&255)|((g&255)<<8)|((b&255)<<16)|((a&255)<<24);}
static void C2D_DrawRectSolid(float x,float y,float z,float w,float h,u32 c){(void)z;printf("R\t%f\t%f\t%f\t%f\t%u\n",x,y,w,h,c);}
static void C2D_DrawEllipseSolid(float x,float y,float z,float w,float h,u32 c){(void)z;printf("E\t%f\t%f\t%f\t%f\t%u\n",x,y,w,h,c);}
static void C2D_TextFontParse(C2D_Text *t,C2D_Font font,C2D_TextBuf buf,const char *s){(void)buf;t->font=font;t->s=s;}
static void C2D_DrawText(C2D_Text *t,int flags,float x,float y,float z,float sx,float sy,...){
 (void)flags;(void)z;(void)sy;va_list ap;va_start(ap,sy);u32 c=va_arg(ap,u32);va_end(ap);
 printf("T\t%f\t%f\t%f\t%d\t%u\t%s\n",x,y,sx*30/t->font->glyph.cellHeight,t->font->id,c,t->s);
}
