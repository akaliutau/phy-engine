

#include <math.h>
#include "ui.h"
#include "ui_sdk.h"
#include "render.h"
#include "error.h"
#include "statistics.h"
#include "radiance.h"
#include "raytracing.h"


#define BACKFACE_CULLING	1
#define OUTLINE_DRAWING		2
#define BOUNDING_BOX_DRAWING	3
#define CLUSTER_DRAWING		4
#define SMOOTH_SHADING		5
#define DISPLAY_LISTS		6
#define FRUSTUM_CULLING		7
#define USE_BACKGROUND          8
#define NO_SHADING              9

static void RenderToggleCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int set = (((XmToggleButtonCallbackStruct *)call_data)->set == XmSET);

  switch((int)client_data) {
  case BACKFACE_CULLING:
    RenderSetBackfaceCulling(set);
    break;
  case OUTLINE_DRAWING:
    RenderNewDisplayList();
    RenderSetOutlineDrawing(set);
    break;
  case BOUNDING_BOX_DRAWING:
    RenderSetBoundingBoxDrawing(set);
    break;
  case CLUSTER_DRAWING:
    RenderSetClusterDrawing(set);
    break;
  case SMOOTH_SHADING:
    RenderNewDisplayList();
    RenderSetSmoothShading(set);
    break;
  case NO_SHADING:
    RenderNewDisplayList();
    RenderSetNoShading(set);
    break;
  case DISPLAY_LISTS:
    RenderNewDisplayList();
    RenderUseDisplayLists(set);
    break;
  case FRUSTUM_CULLING:
    RenderNewDisplayList();
    RenderUseFrustumCulling(set);
    break;
  case USE_BACKGROUND:
    RenderSetUseBackground(set);
    break;
  default:
    Fatal(1, "RenderToggleCallback", "invalid toggle options value %d", (int)client_data);
  }

  RenderScene();
}


static void OutlineColorOKCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget outlineColorBox = w;
  RGB *outlinecolor = (RGB *)client_data;

  XtUnmanageChild(outlineColorBox);

  RenderNewDisplayList();
  RenderSetOutlineColor(outlinecolor);
  RenderScene();
}

static void OutlineColorCancelCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget outlineColorBox = w;
  XtUnmanageChild(outlineColorBox);
}

static Widget CreateOutlineColorDialog(Widget parent, char *name)
{
  Widget outlineColorBox = CreateDialog(parent, name);
  Widget outlineColorForm, outlineColorFrame;

  outlineColorFrame = CreateFrame(outlineColorBox, "outlineColorFrame", "outlineColorTitle");
  outlineColorForm = CreateRowColumn(outlineColorFrame, "outlineColorForm");

  CreateFormEntry(outlineColorForm, "redLabel", "redTextf",
		  FET_FLOAT, (XtPointer)&renderopts.outline_color.r, NULL, 0);
  CreateFormEntry(outlineColorForm, "greenLabel", "greenTextf",
		  FET_FLOAT, (XtPointer)&renderopts.outline_color.g, NULL, 0);
  CreateFormEntry(outlineColorForm, "blueLabel", "blueTextf",
		  FET_FLOAT, (XtPointer)&renderopts.outline_color.b, NULL, 0);

  XtManageChild(outlineColorForm);

  XtAddCallback(outlineColorBox, XmNokCallback,
		OutlineColorOKCallback, (XtPointer)&renderopts.outline_color);

  XtAddCallback(outlineColorBox, XmNcancelCallback,
		OutlineColorCancelCallback, (XtPointer)NULL);

  return outlineColorBox;
}


static void BoundingboxColorOKCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget boundingboxColorBox = w;
  RGB *boundingboxcolor = (RGB *)client_data;
  
  XtUnmanageChild(boundingboxColorBox);
  
  RenderSetBoundingBoxColor(boundingboxcolor);
  RenderScene();
}

static void BoundingboxColorCancelCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget boundingboxColorBox = w;
  XtUnmanageChild(boundingboxColorBox);
}

static Widget CreateBoundingboxColorDialog(Widget parent, char *name)
{
  Widget boundingboxColorBox = CreateDialog(parent, name);
  Widget boundingboxColorForm, boundingboxColorFrame;

  boundingboxColorFrame = CreateFrame(boundingboxColorBox, "boundingboxColorFrame", "boundingboxColorTitle");
  boundingboxColorForm = CreateRowColumn(boundingboxColorFrame, "boundingboxColorForm");

  CreateFormEntry(boundingboxColorForm, "redLabel", "redTextf",
		  FET_FLOAT, (XtPointer)&renderopts.bounding_box_color.r, NULL, 0);
  CreateFormEntry(boundingboxColorForm, "greenLabel", "greenTextf",
		  FET_FLOAT, (XtPointer)&renderopts.bounding_box_color.g, NULL, 0);
  CreateFormEntry(boundingboxColorForm, "blueLabel", "blueTextf",
		  FET_FLOAT, (XtPointer)&renderopts.bounding_box_color.b, NULL, 0);

  XtManageChild(boundingboxColorForm);

  XtAddCallback(boundingboxColorBox, XmNokCallback,
		BoundingboxColorOKCallback, (XtPointer)&renderopts.bounding_box_color);

  XtAddCallback(boundingboxColorBox, XmNcancelCallback,
		BoundingboxColorCancelCallback, (XtPointer)NULL);

  return boundingboxColorBox;
}


static void ClusterColorOKCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget clusterColorBox = w;
  RGB *clustercolor = (RGB *)client_data;
  
  XtUnmanageChild(clusterColorBox);
  
  RenderSetClusterColor(clustercolor);
  RenderScene();
}

static void ClusterColorCancelCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget clusterColorBox = w;
  XtUnmanageChild(clusterColorBox);
}

static Widget CreateClusterColorDialog(Widget parent, char *name)
{
  Widget clusterColorBox = CreateDialog(parent, name);
  Widget clusterColorForm, clusterColorFrame;

  clusterColorFrame = CreateFrame(clusterColorBox, "clusterColorFrame", "clusterColorTitle");
  clusterColorForm = CreateRowColumn(clusterColorFrame, "clusterColorForm");

  CreateFormEntry(clusterColorForm, "redLabel", "redTextf",
		  FET_FLOAT, (XtPointer)&renderopts.cluster_color.r, NULL, 0);
  CreateFormEntry(clusterColorForm, "greenLabel", "greenTextf",
		  FET_FLOAT, (XtPointer)&renderopts.cluster_color.g, NULL, 0);
  CreateFormEntry(clusterColorForm, "blueLabel", "blueTextf",
		  FET_FLOAT, (XtPointer)&renderopts.cluster_color.b, NULL, 0);

  XtManageChild(clusterColorForm);

  XtAddCallback(clusterColorBox, XmNokCallback,
		ClusterColorOKCallback, (XtPointer)&renderopts.cluster_color);

  XtAddCallback(clusterColorBox, XmNcancelCallback,
		ClusterColorCancelCallback, (XtPointer)NULL);

  return clusterColorBox;
}


static void SetLineWidthCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  float lwidth = *(float *)client_data;
  RenderSetLineWidth(lwidth);
  RenderNewDisplayList();
}

static Widget CreateLineWidthDialog(Widget parent, char *name)
{
  Widget box = CreatePromptDialog(parent, name, FET_FLOAT, (XtPointer)&renderopts.linewidth,
				  NULL, 0);
  XtAddCallback(box, XmNokCallback, SetLineWidthCallback, (XtPointer)&renderopts.linewidth);
  return box;
}


void CreateRenderMenu(Widget menuBar)
{
  Widget renderMenu = CreateSubMenu(menuBar, "renderButton", "renderMenu");

  CreateToggleButton(renderMenu, "displayListButton", renderopts.use_display_lists,
		     RenderToggleCallback, (XtPointer)DISPLAY_LISTS);
  CreateToggleButton(renderMenu, "frustumCullingButton", renderopts.frustum_culling,
		     RenderToggleCallback, (XtPointer)FRUSTUM_CULLING);

  CreateSeparator(renderMenu, "renderSeparator");
  CreateToggleButton(renderMenu, "smoothShadingButton", renderopts.smooth_shading,
		     RenderToggleCallback, (XtPointer)SMOOTH_SHADING);
  CreateToggleButton(renderMenu, "backfacecullButton", renderopts.backface_culling,
		     RenderToggleCallback, (XtPointer)BACKFACE_CULLING);

  CreateSeparator(renderMenu, "renderSeparator");
  CreateToggleButton(renderMenu, "drawoutlineButton", renderopts.draw_outlines,
		     RenderToggleCallback, (XtPointer)OUTLINE_DRAWING);
  CreateCascadeDialog(renderMenu, "outlineColorButton", 
		      CreateOutlineColorDialog, "outlineColorBox", NULL, NULL);
  CreateCascadeDialog(renderMenu, "lineWidthButton",
		      CreateLineWidthDialog, "lineWidthBox", NULL, NULL);
  CreateToggleButton(renderMenu, "noShadingButton", renderopts.no_shading,
		     RenderToggleCallback, (XtPointer)NO_SHADING);

  CreateSeparator(renderMenu, "renderSeparator");
  CreateToggleButton(renderMenu, "drawboundingboxButton", renderopts.draw_bounding_boxes,
		     RenderToggleCallback, (XtPointer)BOUNDING_BOX_DRAWING);
  CreateCascadeDialog(renderMenu, "boundingboxColorButton", 
		      CreateBoundingboxColorDialog, "boundingboxColorBox", NULL, NULL);

  CreateSeparator(renderMenu, "renderSeparator");
  CreateToggleButton(renderMenu, "drawclusterButton", renderopts.draw_clusters, 
		     RenderToggleCallback, (XtPointer)CLUSTER_DRAWING);
  CreateCascadeDialog(renderMenu, "clusterColorButton", 
		      CreateClusterColorDialog, "clusterColorBox", NULL, NULL);

  CreateSeparator(renderMenu, "renderSeparator");
  CreateToggleButton(renderMenu, "useBackgroundButton", renderopts.use_background,
		     RenderToggleCallback, (XtPointer)USE_BACKGROUND);
}





