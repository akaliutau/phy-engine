

#include "ui.h"
#include "ui_sdk.h"
#include "ui_phong.h"
#include "error.h"

static void CreateColorForm(Widget parent, char *formname, char *labelname, COLOR *col)
{
  Widget subform;
#ifdef RGBCOLORS
  char *label[3] = {"phongRLabel", "phongGLabel", "phongBLabel"};
#else
  char *label[3] = {"phongXLabel", "phongYLabel", "phongZLabel"};
#endif

#if SPECTRUM_CHANNELS != 3
  Error("CreateColorForm", "Doesn't work for more than 3 spectral channels");
#endif
  subform = CreateRowColumn(parent, formname);
  CreateLabel(subform, labelname);
  CreateFormEntry(subform, label[0], "phongRTextf", FET_FLOAT, (XtPointer)&col->spec[0], NULL, 0);
  CreateFormEntry(subform, label[1], "phongGTextf", FET_FLOAT, (XtPointer)&col->spec[1], NULL, 0);
  CreateFormEntry(subform, label[2], "phongBTextf", FET_FLOAT, (XtPointer)&col->spec[2], NULL, 0);
  XtManageChild(subform);
}

void *CreatePhongEdfEditor(void *parent, PHONG_EDF *edf)
{
  Widget form = (Widget)parent;

  CreateColorForm(form, "phongKdForm", "phongKdLabel", &edf->Kd);
  CreateColorForm(form, "phongKsForm", "phongKsLabel", &edf->Ks);
  CreateFormEntry(form, "phongNsLabel", "phongNsTextf", FET_FLOAT, (XtPointer)&edf->Ns, NULL, 0);

  return form;
}

void *CreatePhongBrdfEditor(void *parent, PHONG_BRDF *brdf)
{
  Widget form = (Widget)parent;

  CreateColorForm(form, "phongKdForm", "phongKdLabel", &brdf->Kd);
  CreateColorForm(form, "phongKsForm", "phongKsLabel", &brdf->Ks);
  CreateFormEntry(form, "phongNsLabel", "phongNsTextf", FET_FLOAT, (XtPointer)&brdf->Ns, NULL, 0);

  return form;
}

void *CreatePhongBtdfEditor(void *parent, PHONG_BTDF *btdf)
{
  Widget form = (Widget)parent;
  Widget subform;

  CreateColorForm(form, "phongKdForm", "phongKdLabel", &btdf->Kd);
  CreateColorForm(form, "phongKsForm", "phongKsLabel", &btdf->Ks);
  CreateFormEntry(form, "phongNsLabel", "phongNsTextf", FET_FLOAT, (XtPointer)&btdf->Ns, NULL, 0);

  subform = CreateRowColumn(form, "phongNrNiForm");
  CreateFormEntry(subform, "phongNrLabel", "phongNrTextf", FET_FLOAT, (XtPointer)&btdf->refrIndex.nr, NULL, 0);
  CreateFormEntry(subform, "phongNiLabel", "phongNiTextf", FET_FLOAT, (XtPointer)&btdf->refrIndex.ni, NULL, 0);
  XtManageChild(subform);

  return form;
}

