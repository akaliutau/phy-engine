

#include <string.h>

#include "options.h"
#include "Boolean.h"
#include "pools.h"
#include "vector.h"
#include "color.h"

static int *_argc;			
static char **_argv, 			
            **first_arg;		


#define init_arg(pargc, argv)	{_argc = pargc; _argv = first_arg = argv;}


#define args_remain()		(_argv - first_arg < *_argc)


#define next_arg()		{_argv++;}


static void PrintArglist(void)
{
  char **av = first_arg;
  while (av - first_arg < *_argc) {
    fprintf(stderr, "'%s' ", *av); av++;
  }
}


static void consume_arg(void) 
{
  char **av = _argv;
  while (av - first_arg < *_argc-1) {
    *av = *(av+1); av++;
  }
  *av = NULL;
  (*_argc)--;
}


#define get_argval(format, res)	(sscanf(*_argv, format, res) == 1)


static int getint(int *n, void *data)
{
  if (!get_argval("%d", n)) {
    fprintf(stderr, "'%s' is not a valid integer value\n", *_argv);
    return FALSE;
  }
  return TRUE;
}

static void printint(FILE *fp, int *n, void *data)
{
  fprintf(fp, "%d", *n);
}

static int dummy_int = 0;

CMDLINEOPTTYPE intTypeStruct = { 
  (int (*)(void *, void *))getint,
  (void (*)(FILE *, void *, void *))printint ,
  (void *)&dummy_int,
  NULL
};


static int getstring(char **s, void *data)
{
  *s = Alloc(strlen(*_argv)+1);
  sprintf(*s, "%s", *_argv);
  return TRUE;
}

static void printstring(FILE *fp, char **s, void *data)
{
  fprintf(fp, "'%s'", *s ? *s : "");
}

static char *dummy_string = NULL;

CMDLINEOPTTYPE stringTypeStruct = { 
  (int (*)(void *, void *))getstring,
  (void (*)(FILE *, void *, void *))printstring ,
  (void *)&dummy_string,
  NULL
};


int Tnstring_get(char *s, int n)
{
  if(s) {
    strncpy(s, *_argv, n);
    s[n-1] = '\0';  
  }
  
  return TRUE;
}

void Tnstring_print(FILE *fp, char *s, int n)
{
  fprintf(fp, "'%s'", s ? s : "");
}

char *Tnstring_dummy_val = NULL;



static void print_enum_vals(ENUMDESC *tab)
{
  while (tab && tab->name) {
    fprintf(stderr, "\t%s\n", tab->name);
    tab++;
  }
}

int Tenum_get(int *v, ENUMDESC *tab)
{
  ENUMDESC *tabsave = tab;
  while (tab && tab->name) {
    if (strncasecmp(*_argv, tab->name, tab->abbrev) == 0) {
      *v = tab->value;
      return TRUE;
    }
    tab++;
  }
  fprintf(stderr, "Invalid option argument '%s'. Should be one of:\n", *_argv);
  print_enum_vals(tabsave);
  return FALSE;
}

void Tenum_print(FILE *fp, int *v, ENUMDESC *tab)
{
  while (tab && tab->name) {
    if (*v == tab->value) {
      fprintf(fp, "%s", tab->name);
      return;
    }
    tab++;
  }
  fprintf(fp, "INVALID ENUM VALUE!!!");
}

int Tenum_dummy_val = 0;

static ENUMDESC Tenum_dummy_table[] = {
  { 0, NULL, 0 }
};

CMDLINEOPTTYPE enumTypeStruct = {
  (int (*)(void *, void *))Tenum_get,
  (void (*)(FILE *, void *, void *))Tenum_print,
  (void *)&Tenum_dummy_val,
  (void *)Tenum_dummy_table
};




static ENUMDESC boolTable[] = {
  { TRUE, "yes", 1 },
  { FALSE, "no", 1 },
  { TRUE, "true", 1 },
  { FALSE, "false", 1 },
  { 0, NULL, 0 }
};

CMDLINEOPTTYPE boolTypeStruct = {
  (int (*)(void *, void *))Tenum_get,
  (void (*)(FILE *, void *, void *))Tenum_print,
  (void *)&Tenum_dummy_val,
  (void *)boolTable
};



static int dummy_true  = TRUE;
static int dummy_false = FALSE;

static int set_true(int *x, void *data)
{
  

  *x = TRUE;
  return TRUE;
}

static int set_false(int *x, void *data)
{
  

  *x = FALSE;
  return TRUE;
}

static void print_other(FILE *fp, void *x, void *data)
{
  fprintf(fp, "other");
}


CMDLINEOPTTYPE setTrueTypeStruct = {
  (int (*)(void *, void *))set_true,
  (void (*)(FILE *, void *, void *))print_other,
  (void *)&dummy_true,
  (void *)NULL
};

CMDLINEOPTTYPE setFalseTypeStruct = {
  (int (*)(void *, void *))set_false,
  (void (*)(FILE *, void *, void *))print_other,
  (void *)&dummy_false,
  (void *)NULL
};



static int getfloat(float *x, void *data)
{
  if (!get_argval("%f", x)) {
    fprintf(stderr, "'%s' is not a valid floating point value\n", *_argv);
    return FALSE;
  }
  return TRUE;
}

static void printfloat(FILE *fp, float *x, void *data)
{
  fprintf(fp, "%g", *x);
}

static float dummy_float = 0.;

CMDLINEOPTTYPE floatTypeStruct = { 
  (int (*)(void *, void *))getfloat,
  (void (*)(FILE *, void *, void *))printfloat ,
  (void *)&dummy_float,
  NULL
};


static int getvector(VECTOR *v, void *data)
{
  int ok = TRUE;
  ok = get_argval("%f", &v->x);
  if (ok) {
    consume_arg();
    ok &= args_remain() && get_argval("%f", &v->y);
  }
  if (ok) {
    consume_arg();
    ok &= args_remain() && get_argval("%f", &v->z);
  }
  if (!ok)
    fprintf(stderr, "invalid vector argument value");

  return ok;
}

static void printvector(FILE *fp, VECTOR *v, void *data)
{
  VectorPrint(fp, *v);
}

static VECTOR dummy_vector = {0., 0., 0.};

CMDLINEOPTTYPE vectorTypeStruct = {
  (int (*)(void *, void *))getvector,
  (void (*)(FILE *, void *, void *))printvector,
  (void *)&dummy_vector,
  NULL
};


static int getrgb(RGB *c, void *data)
{
  int ok = TRUE;
  ok = get_argval("%f", &c->r);
  if (ok) {
    consume_arg();
    ok &= args_remain() && get_argval("%f", &c->g);
  }
  if (ok) {
    consume_arg();
    ok &= args_remain() && get_argval("%f", &c->b);
  }
  if (!ok)
    fprintf(stderr, "invalid RGB color argument value");

  return ok;
}

static void printrgb(FILE *fp, RGB *v, void *data)
{
  RGBPrint(fp, *v);
}

static RGB dummy_rgb = {0., 0., 0.};

CMDLINEOPTTYPE rgbTypeStruct = {
  (int (*)(void *, void *))getrgb,
  (void (*)(FILE *, void *, void *))printrgb,
  (void *)&dummy_rgb,
  NULL
};


static int getxy(float *c, void *data)
{
  int ok = TRUE;
  ok = get_argval("%f", &c[0]);
  if (ok) {
    consume_arg();
    ok &= args_remain() && get_argval("%f", &c[1]);
  }
  if (!ok)
    fprintf(stderr, "invalid CIE xy color argument value");

  return ok;
}

static void printxy(FILE *fp, float *c, void *data)
{
  fprintf(fp, "%g %g", c[0], c[1]);
}

static float dummy_xy[2] = {0., 0.};

CMDLINEOPTTYPE xyTypeStruct = {
  (int (*)(void *, void *))getxy,
  (void (*)(FILE *, void *, void *))printxy,
  (void *)&dummy_xy,
  NULL
};


#ifndef MAX
#define MAX(a,b)	(((a)>(b)) ? (a) : (b))
#endif

static CMDLINEOPTDESC *LookupOption(char *s, CMDLINEOPTDESC *options)
{
  CMDLINEOPTDESC *opt = options;
  while (opt->name) {
    if (strncmp(s, opt->name, MAX(opt->abbrevlength>0 ? opt->abbrevlength : strlen(opt->name), strlen(s))) == 0) 
      return opt;
    opt++;
  }
  return (CMDLINEOPTDESC *)NULL;
}

static void PrintOption(FILE *fp, CMDLINEOPTDESC *opt)
{
  if (opt->description) {
    fprintf(fp, opt->description);
    fprintf(fp, " ");
    if (opt->type && opt->value &&
	(opt->type != &setTrueTypeStruct) &&
	(opt->type != &setFalseTypeStruct)) {
      fprintf(fp, "(default = ");
      opt->type->print(fp, opt->value, opt->type->data);
      fprintf(fp, ")");
    }
    fprintf(fp, "\n");
  }
}

static void process_arg(CMDLINEOPTDESC *options)
{
  CMDLINEOPTDESC *opt = LookupOption(*_argv, options);
  if (opt) {
    int ok = TRUE;
    if (opt->type) {
      if ((opt->type == &setTrueTypeStruct) ||
	  (opt->type == &setFalseTypeStruct)) {
	if (!opt->type->get(opt->value ? opt->value : opt->type->dummy, opt->type->data))
	  ok = FALSE;
      }
      else {
	consume_arg();
	if(args_remain()) {
	  if (!opt->type->get(opt->value ? opt->value : opt->type->dummy, opt->type->data))
	    ok = FALSE;
	} else {
	  fprintf(stderr, "Option argument missing.\n");
	  ok = FALSE;
	}
      }
      if (!ok)
	PrintOption(stderr, opt);
    }
    if (ok && opt->action) opt->action(opt->value ? opt->value : (opt->type ? opt->type->dummy : NULL));
    consume_arg();
  } else
    next_arg();
}

void ParseOptions(CMDLINEOPTDESC *options, int *argc, char **argv)
{
  init_arg(argc, argv);
  while (args_remain())
    process_arg(options);
}

void PrintOptions(FILE *fp, CMDLINEOPTDESC *opt)
{
  while (opt->name) {
    PrintOption(fp, opt);
    opt++;
  }
}
