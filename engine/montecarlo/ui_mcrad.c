

#include "ui.h"
#include "ui_sdk.h"
#include "error.h"
#include "render.h"

#include "mcrad.h"
#include "mcradP.h"
#include "ui_mcrad.h"

static void SamplingCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  mcr.sequence = (SEQ4D)client_data;
}

void McrCreateSamplingMenu(Widget parent)
{
  Widget subform = CreateRadioBox(parent, "samplingForm");

  CreateToggleButton(subform, "samplingRandomButton",
		     mcr.sequence == S4D_RANDOM ? True : False,
		     SamplingCallback, (XtPointer)S4D_RANDOM);

  CreateToggleButton(subform, "samplingHaltonButton",
		     mcr.sequence == S4D_HALTON ? True : False,
		     SamplingCallback, (XtPointer)S4D_HALTON);

  CreateToggleButton(subform, "samplingNiederreiterButton",
		     mcr.sequence == S4D_NIEDERREITER ? True : False,
		     SamplingCallback, (XtPointer)S4D_NIEDERREITER);

  XtManageChild(subform);
}

static void BasisCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  mcr.approx_type = (APPROX_TYPE)client_data;
}

void McrCreateBasisMenu(Widget parent)
{
  Widget subform = CreateRadioBox(parent, "basisForm");

  CreateToggleButton(subform, "constantBasisButton",
		     mcr.approx_type == AT_CONSTANT ? True : False,
		     BasisCallback, (XtPointer)AT_CONSTANT);

  CreateToggleButton(subform, "linearBasisButton",
		     mcr.approx_type == AT_LINEAR ? True : False,
		     BasisCallback, (XtPointer)AT_LINEAR);

  CreateToggleButton(subform, "quadraticBasisButton",
		     mcr.approx_type == AT_QUADRATIC ? True : False,
		     BasisCallback, (XtPointer)AT_QUADRATIC);

  CreateToggleButton(subform, "cubicBasisButton",
		     mcr.approx_type == AT_CUBIC ? True : False,
		     BasisCallback, (XtPointer)AT_CUBIC);

  XtManageChild(subform);
}

