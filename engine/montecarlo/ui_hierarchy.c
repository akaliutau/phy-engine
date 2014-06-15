



#ifndef lint
static const char vcid[] = "$Id: ui_hierarchy.c,v 1.2 Exp $";
#endif 

#include "ui.h"
#include "ui_sdk.h"
#include <Xm/MessageB.h>
#include "hierarchy.h"
#include "error.h"


static void 
ClusteringCallback(Widget w,
		   XtPointer client_data,
		   XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set)
    hierarchy.clustering = (CLUSTERING_MODE)client_data;
}

void
CreateClusteringMenu(Widget parent)
{
  Widget frame = CreateFrame(parent,
			     "hierClusteringFrame", "hierClusteringTitle");
  Widget toggleBox = CreateRadioBox(frame, "hierClusteringToggleBox");

  CreateToggleButton(toggleBox, "hierNoClusteringButton",
		     hierarchy.clustering == NO_CLUSTERING,
		     ClusteringCallback, (XtPointer)NO_CLUSTERING);

  CreateToggleButton(toggleBox, "hierIsotropicClusteringButton",
		     hierarchy.clustering == ISOTROPIC_CLUSTERING,
		     ClusteringCallback, (XtPointer)ISOTROPIC_CLUSTERING);

  CreateToggleButton(toggleBox, "hierOrientedClusteringButton",
		     hierarchy.clustering == ORIENTED_CLUSTERING,
		     ClusteringCallback, (XtPointer)ORIENTED_CLUSTERING);

  XtManageChild(toggleBox);
}


static void
SetHMeshingCallback(Widget w,
		    XtPointer client_data,
		    XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set == XmSET;
  if (hierarchy.do_h_meshing != set && Working()) 
    Warning(NULL, "Interrupt computations first");
  else
    hierarchy.do_h_meshing = set;
}

void
CreateAccuracyBox(Widget parent)
{
  Widget frame = CreateFrame(parent, "hierAccuracyFrame", "hierAccuracyTitle");
  Widget subform = CreateRowColumn(frame, "hierAccuracyForm");

  CreateToggleButton(subform, "hierHMeshingButton", 
		     hierarchy.do_h_meshing ? True : False,
		     SetHMeshingCallback, NULL);

  CreateFormEntry(subform, "hierEpsilon", "hierEpsilonTextf",
		  FET_FLOAT, (XtPointer)&hierarchy.epsilon, NULL, 0);

  CreateFormEntry(subform, "hierMinArea", "hierMinAreaTextf",
		  FET_FLOAT, (XtPointer)&hierarchy.minarea, NULL, 0);

  XtManageChild(subform);
}

#ifdef NEVER

static void
SetPdrSubdivideCallback(Widget w,
			XtPointer client_data,
			XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set == XmSET;
  if (hierarchy.pdr_subdivide != set && Working()) 
    Warning(NULL, "Interrupt computations first");
  else
    hierarchy.pdr_subdivide = set;
}

void
CreatePdrSubdivideToggleButton(Widget parent)
{
  CreateToggleButton(parent, "hierPdrSubdivideButton", 
		     hierarchy.pdr_subdivide ? True : False,
		     SetPdrSubdivideCallback, NULL);
}


static void
SetIdrRefinementCallback(Widget w, 
			 XtPointer client_data,
			 XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set == XmSET;
  if (hierarchy.imp_refinement != set && Working()) 
    Warning(NULL, "Interrupt computations first");
  else
    hierarchy.imp_refinement = set;
}

void
CreateImpRefinementToggleButton(Widget parent)
{
  CreateToggleButton(parent, "hierIdrRefinementButton", 
		     hierarchy.imp_refinement ? True : False,
		     SetIdrRefinementCallback, NULL);
}



static void
SetIdrHierarchicalCallback(Widget w,
			   XtPointer client_data,
			   XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set == XmSET;
  if (hierarchy.do_h_importance != set && Working()) 
    Warning(NULL, "Interrupt computations first");
  else
    hierarchy.do_h_importance = set;
}

void
CreateImpHierarchicalToggleButton(Widget parent)
{
  CreateToggleButton(parent, "hierIdrHierarchicalButton", 
		     hierarchy.do_h_importance ? True : False,
		     SetIdrHierarchicalCallback, NULL);
}
#endif 


void
CreateHMeshingMenu(Widget parent)
{
  CreateAccuracyBox(parent);
  CreateClusteringMenu(parent);
}
