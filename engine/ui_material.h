/* ui_material.h: material editor */

#ifndef _UI_MATERIAL_H_
#define _UI_MATERIAL_H_

#include "ui.h"
#include "material.h"

extern Widget CreateMaterialEditor(Widget parent, char *name);
extern void EditMaterial(MATERIAL *mat);

#endif /*_UI_MATERIAL_H_*/
