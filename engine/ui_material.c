

#include "ui.h"
#include "ui_sdk.h"
#include "ui_material.h"
#include "render.h"
#include "scene.h"
#include "radiance.h"
#include "vertex.h"

static Widget materialEditorDialog, matSelectionLabel,
              edfEditorFrame, edfEditorForm,
              bsdfEditorFrame, bsdfEditorForm;

static MATERIAL *the_mat=(MATERIAL *)NULL, *the_dupl=(MATERIAL *)NULL;

static void CreateMaterialSelectionPanel(Widget parent)
{
  matSelectionLabel = CreateLabel(parent, "matSelectionLabel");
}

static void DismissCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild(materialEditorDialog);
}

static void UpdateCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  if (!Radiance) {
    COLOR rho = BsdfDiffuseReflectance(the_dupl->bsdf);
    RGB rgb;
    ColorToRGB(rho, &rgb);
    ForAllPatches(p, Patches) {
      if (p->surface->material == the_mat) {
	p->color = rgb;
	PatchComputeVertexColors(p);
      }
    } EndForAll;
    RenderNewDisplayList();
    RenderScene();
  } else {
    if (Radiance->UpdateMaterial) {
      Radiance->UpdateMaterial(the_mat, the_dupl);
    }
  }

  if (the_mat->edf) {EdfDestroy(the_mat->edf); the_mat->edf = EdfDuplicate(the_dupl->edf);}
  if (the_mat->bsdf) {BsdfDestroy(the_mat->bsdf); the_mat->bsdf = BsdfDuplicate(the_dupl->bsdf);}
}

static Widget CreateEdfEditor(Widget parent, EDF *edf)
{
  Widget form = CreateRowColumn(parent, "edfEditorForm");
  if (!edf) {
    CreateLabel(form, "noEdfLabel");
  } else {
    EdfCreateEditor(form, edf);
  }
  return form;
}

static Widget CreateBsdfEditor(Widget parent, BSDF *bsdf)
{
  Widget form = CreateRowColumn(parent, "bsdfEditorForm");
  if (!bsdf) {
    CreateLabel(form, "noBsdfLabel");
  } else {
    BsdfCreateEditor(form, bsdf);
  }
  return form;
}

static void SelectMaterial(MATERIAL *mat)
{
  the_mat = mat;

  if (the_dupl) MaterialDestroy(the_dupl);
  the_dupl = MaterialDuplicate(mat);

  XtVaSetValues(matSelectionLabel,
		XtVaTypedArg,
		   XmNlabelString, XmRString, mat->name, strlen(mat->name) + 1,
		NULL);

  XtUnmanageChild(edfEditorForm);
  XtDestroyWidget(edfEditorForm);
  edfEditorForm = CreateEdfEditor(edfEditorFrame, the_dupl->edf);
  XtManageChild(edfEditorForm);

  XtUnmanageChild(bsdfEditorForm);
  XtDestroyWidget(bsdfEditorForm);
  bsdfEditorForm = CreateBsdfEditor(bsdfEditorFrame, the_dupl->bsdf);
  XtManageChild(bsdfEditorForm);
}

void EditMaterial(MATERIAL *mat)
{
  SelectMaterial(mat);
  XtManageChild(materialEditorDialog);
}

Widget CreateMaterialEditor(Widget parent, char *name)
{  
  Widget matPanel = CreateDialog(parent, name);
  Widget form, frame;
  {
    static int wgiv=0; if (!wgiv) {
      Error("CreateMaterialEditor", "to be revised");
      wgiv = 1;
    }
  }

  materialEditorDialog = matPanel;

  form = CreateRowColumn(matPanel, "matForm");

  CreateLabel(form, "matTitle");

  frame = CreateFrame(form, "matSelectionFrame", "matSelectionTitle");
  CreateMaterialSelectionPanel(frame);

  edfEditorFrame = CreateFrame(form, "edfEditorFrame", "edfEditorTitle");
  edfEditorForm = CreateRowColumn(edfEditorFrame, "edfEditorForm");
  XtManageChild(edfEditorForm);

  bsdfEditorFrame = CreateFrame(form, "bsdfEditorFrame", "bsdfEditorTitle");
  bsdfEditorForm = CreateRowColumn(bsdfEditorFrame, "bsdfEditorForm");
  XtManageChild(bsdfEditorForm);

  XtManageChild(form);

  XtAddCallback(matPanel, XmNcancelCallback, DismissCallback, (XtPointer)NULL);
  XtAddCallback(matPanel, XmNokCallback, UpdateCallback, (XtPointer)NULL);

  return matPanel;
}
