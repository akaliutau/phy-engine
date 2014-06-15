/* texture.h: Phy2 RGB texture objects */

#ifndef _PHY_TEXTURE_H_
#define _PHY_TEXTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef struct TEXTURE {
  int width, height, channels;
  unsigned char *data;  /* first bytes correspond to bottom-left pixel (as in OpenGL) */
} TEXTURE;

extern TEXTURE* CreateTexture(int width, int height, int channels, unsigned char* data);
extern void DestroyTexture(TEXTURE* texture);
extern void PrintTexture(FILE *out, TEXTURE* texture);

#include "color.h"
extern COLOR EvalTextureColor(TEXTURE *texture, float u, float v);

#ifdef __cplusplus
}
#endif

#endif /*_PHY_TEXTURE_H_*/


