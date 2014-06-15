

#include "rtdummy.h"
#include "options.h"
#include "render.h"

static void Defaults(void)
{
}

static void ParseRTOptions(int *argc, char **argv)
{
}

static void PrintRTOptions(FILE *fp)
{
}

static void Initialize(void)
{
}

static void RayCast(ImageOutputHandle *ip)
{
}


static int Redisplay(void)
{
  return TRUE;
}


static int SaveImage(ImageOutputHandle *ip)
{
  return TRUE;
}

static void Interrupt(void)
{
}

static void Terminate(void)
{
}


static void CreateControlPanel(void *parent_widget)
{
}

static void ShowControlPanel(void)
{
}

static void HideControlPanel(void)
{
}

RAYTRACINGMETHOD rtdummy = {
  "RTDummy", 4,
  "Ray Tracing Template",
  "rtdummyButton",
  Defaults,
  CreateControlPanel,
  ParseRTOptions,
  PrintRTOptions,
  Initialize,
  RayCast,
  Redisplay,
  SaveImage,
  Interrupt,
  ShowControlPanel,
  HideControlPanel,
  Terminate
};

