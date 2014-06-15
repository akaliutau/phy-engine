/* ui_phong.h: Phong EDF... editor creation methods. */

#ifndef _UI_PHONG_H_
#define _UI_PHONG_H_

#include "phong.h"

/* methods for creating a Phong edf... editor */
extern void *CreatePhongEdfEditor(void *parent, PHONG_EDF *edf);
extern void *CreatePhongBrdfEditor(void *parent, PHONG_BRDF *brdf);
extern void *CreatePhongBtdfEditor(void *parent, PHONG_BTDF *btdf);

#endif /*_UI_PHONG_H_*/
