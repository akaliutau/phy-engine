
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include "extmath.h"

#include "ui.h"
#include "ui_sdk.h"
#include "ui_dump.h"
#include "scene.h"
#include "vertex.h"
#include "render.h"
#include "error.h"

typedef enum DUMPTYPE {DT_RADIANCE, DT_RGB, DT_FALSECOLOR} DUMPTYPE;
typedef enum FCDUMPTYPE {FC_LINEAR, FC_LOG} FCDUMPTYPE;

#define NRKEYS	5

typedef struct FCKEY {
  float val;
  RGB col;
} FCKEY;

static struct {
  DUMPTYPE type;
  int radFields[SPECTRUM_CHANNELS];
  int rgbFields[3];
  int fcField;
  int idField;
  FCDUMPTYPE fcdumptype;
  FCKEY key[NRKEYS];
  COLOR radiance;
  RGB rgb;
  float val, fltid;
} dump;

static void DumpDefaults(void)
{
  int i;
  double min = 0.1, max = 10.;
  RGB rgb[5];
  rgb[0] = Blue;
  rgb[1] = Turquoise;
  rgb[2] = Green;
  rgb[3] = Yellow;
  rgb[4] = Red;

  dump.type = DT_RADIANCE;
  dump.idField = 1;

  for (i=0; i<SPECTRUM_CHANNELS; i++) {
    dump.radFields[i] = i+2;
  }

  dump.fcField = SPECTRUM_CHANNELS+2;

  dump.rgbFields[0] = 2;
  dump.rgbFields[1] = 3;
  dump.rgbFields[2] = 4;

  dump.fcdumptype = FC_LOG;
  if (NRKEYS != 5)
    Error("DumpDefault", "Check default false color key value/color pairs");
  for (i=0; i<NRKEYS && i<5; i++) {
    double f = (double)i/(double)(NRKEYS-1);
    dump.key[i].val = pow(10., (1.-f) * log10(min) + f * log10(max));
    dump.key[i].col = rgb[i];
  }
}

static void DumpTypeCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int set = (((XmToggleButtonCallbackStruct *)call_data)->set == XmSET);
  if (set) {
    dump.type = (DUMPTYPE)client_data;
  }
}

static Widget CreateDumpTypeBox(Widget parent)
{
  Widget frame = CreateFrame(parent, "dumpTypeFrame", "Dump Type");
  Widget form = CreateRadioBox(frame, "dumpTypeBox");

  CreateToggleButton(form, "Radiance Dump", 
		     dump.type == DT_RADIANCE ? True : False,
		     DumpTypeCallback, (XtPointer)DT_RADIANCE);

  CreateToggleButton(form, "RGB Dump", 
		     dump.type == DT_RGB ? True : False,
		     DumpTypeCallback, (XtPointer)DT_RGB);

  CreateToggleButton(form, "False Color Dump", 
		     dump.type == DT_FALSECOLOR ? True : False,
		     DumpTypeCallback, (XtPointer)DT_FALSECOLOR);

  XtManageChild(form);
  return frame;
}

static Widget CreateIdFieldBox(Widget parent)
{
  Widget frame = CreateFrame(parent, "dumpIdFieldFrame", "Patch ID Field");
  CreateFormEntry(frame, "Column", "dumpIdTextf",
		  FET_INTEGER, (XtPointer)&dump.idField, NULL, 0);
  return frame;
}

static Widget CreateRadianceDumpOptionsBox(Widget parent)
{
  int i;

  Widget frame = CreateFrame(parent, "radDumpOptsFrame", "Radiance Dump Options");
  Widget form = CreateRowColumn(frame, "radDumpBox");
  XtVaSetValues(form, 
		XmNorientation, XmHORIZONTAL,
		NULL);

  CreateLabel(form, "Fields");
  for (i=0; i<SPECTRUM_CHANNELS; i++) {
    char name[100];
    sprintf(name, " Chan %d", i+1);
    CreateFormEntry(form, name, "radDumpTextf",
		    FET_INTEGER, (XtPointer)&dump.radFields[i], NULL, 0);
  }

  XtManageChild(form);
  return frame;
}

static Widget CreateRGBDumpOptionsBox(Widget parent)
{
  Widget frame = CreateFrame(parent, "rgbDumpOptsFrame", "RGB Dump Options");
  Widget form = CreateRowColumn(frame, "rgbDumpBox");

  XtVaSetValues(form, 
		XmNorientation, XmHORIZONTAL,
		NULL);

  CreateLabel(form, "Fields");
  CreateFormEntry(form, " R", "rgbDumpTextf",
		  FET_INTEGER, (XtPointer)&dump.rgbFields[0], NULL, 0);
  CreateFormEntry(form, " G", "rgbDumpTextf",
		  FET_INTEGER, (XtPointer)&dump.rgbFields[1], NULL, 0);
  CreateFormEntry(form, " B", "rgbDumpTextf",
		  FET_INTEGER, (XtPointer)&dump.rgbFields[2], NULL, 0);

  XtManageChild(form);
  return frame;
}

static void SetLogLinDumpCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int set = (((XmToggleButtonCallbackStruct *)call_data)->set == XmSET);
  if (set) {
    dump.fcdumptype = (FCDUMPTYPE)client_data;
  }
}

static Widget CreateFalseColorDumpOptionsBox(Widget parent)
{
  int i;

  Widget frame = CreateFrame(parent, "radFalseColorOptsFrame", "False Color Dump Options");
  Widget form = CreateRowColumn(frame, "fcDumpBox");
  Widget subform;
  
  subform = CreateRadioBox(form, "loglinDumpBox");
  CreateToggleButton(subform, "Linear",
		     dump.fcdumptype == FC_LINEAR ? True : False,
		     SetLogLinDumpCallback, (XtPointer)FC_LINEAR);
  CreateToggleButton(subform, "Logarithmic",
		     dump.fcdumptype == FC_LOG ? True : False,
		     SetLogLinDumpCallback, (XtPointer)FC_LOG);
  XtManageChild(subform);

  CreateFormEntry(form, "Value field", "fcdumpTextf",
		  FET_INTEGER, (XtPointer)&dump.fcField, NULL, 0);

  CreateLabel(form, "Key Value/Color pairs (min value to max value)");
  for (i=0; i<NRKEYS; i++) {
    subform = CreateRowColumn(form, "keyBox");
    CreateFormEntry(subform, "Value", "fcvalTextf",
		    FET_FLOAT, (XtPointer)&dump.key[i].val, NULL, 0);
    CreateFormEntry(subform, " R", "fccolTextf",
		    FET_FLOAT, (XtPointer)&dump.key[i].col.r, NULL, 0);
    CreateFormEntry(subform, " G", "fccolTextf",
		    FET_FLOAT, (XtPointer)&dump.key[i].col.g, NULL, 0);
    CreateFormEntry(subform, " B", "fccolTextf",
		    FET_FLOAT, (XtPointer)&dump.key[i].col.b, NULL, 0);
    XtManageChild(subform);
  }

  XtManageChild(form);
  return frame;
}

Widget CreateDumpConfigureDialog(Widget parent, char *name)
{
  Widget dialog = CreateDialog(parent, name);
  Widget form = CreateRowColumn(dialog, "dumpConfigureForm");

  DumpDefaults();

  CreateDumpTypeBox(form);
  CreateIdFieldBox(form);
  CreateRadianceDumpOptionsBox(form);
  CreateRGBDumpOptionsBox(form);
  CreateFalseColorDumpOptionsBox(form);

  XtManageChild(form);
  return dialog;
}

static int lineno;

static int process_radiance(PATCH *P)
{
  COLORABS(dump.radiance, dump.radiance);
  RadianceToRGB(dump.radiance, &P->color);
  return TRUE;
}

static int process_rgb(PATCH *P)
{
  P->color = dump.rgb;
  return TRUE;
}

static double ident(double x)
{
  return x;
}

static void mapval(int *interval, float *fraction, double (*f)(double))
{
  int i;
  if (f(dump.val) <= f(dump.key[0].val)) {
    *interval = 0;
    *fraction = 0.;
  } else if (f(dump.val) >= f(dump.key[NRKEYS-1].val)) {
    *interval = NRKEYS-2;
    *fraction = 1.;
  } else {
    for (i=1; i<NRKEYS-1; (i)++) {
      if (f(dump.val) <= f(dump.key[i].val))
	break;
    }
    i = i-1;
    *interval = i;
    *fraction = (f(dump.val) - f(dump.key[i].val)) / (f(dump.key[i+1].val) - f(dump.key[i].val));
  }
}

static int do_process_falsecolor(PATCH *P, double (*f)(double x))
{
  int i; float x;
  RGB col1, col2;

  errno = 0;
  f(dump.val);
  if (errno != 0)
    return FALSE;

  mapval(&i, &x, f);
  col1 = dump.key[i].col;
  col2 = dump.key[i+1].col;
  RGBSET(P->color,
	 (1.-x) * col1.r + x * col2.r,
	 (1.-x) * col1.g + x * col2.g,
	 (1.-x) * col1.b + x * col2.b);

  return TRUE;
}

static int process_falsecolor(PATCH *P)
{
  double (*map)(double) = ident;

  switch (dump.fcdumptype) {
  case FC_LINEAR: map = ident; break;
  case FC_LOG:    map = log10; break;
  default:
    Fatal(-1, "process_falsecolor", "Invalid falsecolor dump type %d", dump.fcdumptype);
  }

  if (!do_process_falsecolor(P, map)) {
    Warning(NULL, "Math error on line %d", lineno);
    P->color = Black;
  }
  return True;
}

#define MAXFIELDS 100
#define MAXBUF 1000

static void check_need_field(float *fields[MAXFIELDS+1], int i, float *valp)
{
  if (i > MAXFIELDS)
    Error(NULL, "Too many fields in file: limit dump files to %d fields maximum",
	  MAXFIELDS);
  else if (fields[i] != (float *)NULL)
    Error(NULL, "Field %d is used multiple times", i);
  else
    fields[i] = valp;
}

static int check_what_fields(float *fields[MAXFIELDS+1])
{
  int i;
  for (i=0; i<=MAXFIELDS; i++) fields[i] = (float *)NULL;

  check_need_field(fields, dump.idField, &dump.fltid);
  switch (dump.type) {
  case DT_RADIANCE:
    for (i=0; i<SPECTRUM_CHANNELS; i++)
      check_need_field(fields, dump.radFields[i], &dump.radiance.spec[i]);
    break;
  case DT_RGB:
    check_need_field(fields, dump.rgbFields[0], &dump.rgb.r);
    check_need_field(fields, dump.rgbFields[1], &dump.rgb.g);
    check_need_field(fields, dump.rgbFields[2], &dump.rgb.b);
    break;
  case DT_FALSECOLOR:
    check_need_field(fields, dump.fcField, &dump.val);
    break;
  default:
    Fatal(-1, "check_what_fields", "Invalid dump type %d", dump.type);
  }

  for (i=MAXFIELDS; i>=0; i--) {
    if (fields[i] != (float *)NULL) return i;
  }

  return 0;
}

static void init_dump_vals(void)
{
  COLORCLEAR(dump.radiance);
  dump.rgb = Black;
  dump.val = 0.;
  dump.fltid = 0.;

  
}

static int parse_fields(char *buf, int *id, float *fields[MAXFIELDS+1], int nfields)
{
  int i;
  char *p = buf;
  for (i=1; i<=nfields; i++) {
    int n;
    float v;
    if (sscanf(p, "%f%n", &v, &n) == 0) {
      Error(NULL, "Too few fields on line %d", lineno);
      return FALSE;
    }
    p += n;

    if (fields[i] != (float *)NULL) {
      *fields[i] = v;
    }
  }

  *id = (int)dump.fltid;

  return TRUE;
}

static int read_dump(char *fname, FILE *fp, int ispipe, Widget fsbox)
{
  PATCH **id2patch;
  int nrids;
  int i, id, nrfields;
  float *fields[MAXFIELDS+1];
  char buf[MAXBUF];

  init_dump_vals();
  nrfields = check_what_fields(fields);
#ifdef DEBUG
  fprintf(stderr, "File needs to contain at least %d fields\n", nrfields);
  fprintf(stderr, "Required fields: ");
  for (i=1; i<=nrfields; i++)
    if (fields[i]) fprintf(stderr, "%d ", i);
  fprintf(stderr, "\n");
#endif

  
  nrids = PatchGetNextID();
  id2patch = (PATCH **)Alloc(nrids * sizeof(PATCH *));
  for (i=0; i<nrids; i++) id2patch[i] = (PATCH *)NULL;
  ForAllPatches(P, Patches) {
    id2patch[P->id] = P;
  } EndForAll;

  lineno = 0;
  while (fgets(buf, MAXBUF, fp)) {
    PATCH *P;
    int ok;
    lineno++;

    if (buf[0] == '#')
      continue;  

    if (!parse_fields(buf, &id, fields, nrfields))
      break;

    if (id >= nrids || id<0 || !(P = id2patch[id])) {
      Error(NULL, "There is no patch with ID %d (on line %d)",
	    id, lineno);
      break;
    }

    ok = TRUE;
    switch (dump.type) {
    case DT_RADIANCE:   ok = process_radiance(P); break;
    case DT_RGB:        ok = process_rgb(P); break;
    case DT_FALSECOLOR: ok = process_falsecolor(P); break;
    default:
      Fatal(-1, "read_dump", "Invalid dump type %d", dump.type);
    }
    if (!ok)
      break;    
  }

  ForAllPatches(P, Patches) {
    PatchComputeVertexColors(P);
  } EndForAll;

  RenderNewDisplayList();
  RenderScene();

  Free((char *)id2patch, nrids * sizeof(PATCH *));

  return TRUE;
}

Widget CreateReadDumpDialog(Widget parent, char *name)
{
  return CreateFileSelectionDialog(parent, name, read_dump, "r");
}
