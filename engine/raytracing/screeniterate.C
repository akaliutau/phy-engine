

#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

#include "ui.h"

#include "raytracing.h"
#include "screeniterate.H"
#include "camera.h"
#include "render.h"
#include "color.h"
#include "raytools.H"



#define WAKE_UP_PROCESS ((unsigned char)1<<0)
#define WAKE_UP_RENDER  ((unsigned char)1<<1)

#define WAKE_UP_ALL 0xff

typedef struct SCREENITERATESTATE
{
  clock_t lastTime;
  unsigned char wake_up;
} SCREENITERATESTATE;

static SCREENITERATESTATE iState;


static void UpdateCpuSecs(void)
{
  rt_total_time += clock() - iState.lastTime;
  iState.lastTime = clock();
}



static void wake_up(int )
{
  iState.wake_up = WAKE_UP_ALL;
  signal(SIGALRM, wake_up);
  alarm( 1 );

  UpdateCpuSecs();
}

static void (*prev_alrm_handler)(int signr);
static unsigned prev_alarm_left;

// ScreenIterateInit : initialise statistics and timers
void ScreenIterateInit(void)
{
  interrupt_raytracing = false;

#ifndef NO_EVENT_TIMER
  
  prev_alrm_handler = signal(SIGALRM, wake_up);
  prev_alarm_left = alarm( 1 );
  iState.wake_up = 0;
#endif

  
  iState.lastTime = clock(); 
  rt_total_time = 0.;
  rt_raycount = rt_pixcount = 0;
}

void ScreenIterateFinish(void)
{
  UpdateCpuSecs();

#ifndef NO_EVENT_TIMER
  
  signal(SIGALRM, prev_alrm_handler);
  alarm(prev_alarm_left);
#endif

}

void ScreenIterateSequential(SCREENITERATECALLBACK callback, void *data)
{
  int i,j, width, height;
  COLOR col;
  RGB *rgb;

  ScreenIterateInit();

  width = Camera.hres;
  height = Camera.vres;
  rgb = new RGB[width];

  
 
  for (j=0; j<height && ! interrupt_raytracing ; j++) 
  {  
    fprintf(stderr,"%d", j);
    for (i=0; i<width && ! interrupt_raytracing; i++) 
    {
      col = callback(i, j, data);
      RadianceToRGB(col, &rgb[i]);

      rt_pixcount++;

#ifndef NO_EVENT_TIMER
      if(iState.wake_up & WAKE_UP_PROCESS)
#else
      if(rt_pixcount & 0x0F)
#endif
      {
	ProcessWaitingEvents();
	iState.wake_up &= ~WAKE_UP_PROCESS;
      }

    }
    
    
    
    RenderPixels(0, j, width, 1, rgb);
  }

  delete[] rgb;

  ScreenIterateFinish();
}





static inline void FillRect(int x0, int y0, int x1, int y1, RGB col,RGB *rgb)
{
  int x,y;

  for(x = x0; x < x1; x++)
  {
    for(y = y0; y < y1; y++)
    {
      rgb[y * Camera.hres + x] = col;
    }
  }
}


void ScreenIterateProgressive(SCREENITERATECALLBACK callback, void *data)
{
  int i, width, height;
  COLOR col;
  RGB pixelRGB;
  RGB *rgb;
  int x0,y0,x1,y1,stepsize,xsteps,ysteps;
  int xstep_done, ystep_done, skip;
  int ymin, ymax;

  ScreenIterateInit();

  width = Camera.hres;
  height = Camera.vres;
  rgb = new RGB[width * height]; // We need a full screen !

  for(i = 0; i < width * height; i++)
  {
    rgb[i] = Black;
  }

  stepsize = 64;
  skip = false;  // First iteration all squares need to be filled
  ymin = height + 1;
  ymax = -1;
  
  while((stepsize > 0) && (!interrupt_raytracing))
  {
    y0 = 0;
    ysteps = 0;
    ystep_done = false;
    
    while(!ystep_done && (!interrupt_raytracing))
    {
      y1 = y0 + stepsize;
      if(y1 >= height)
      {
	y1 = height;
	ystep_done = true;
      }
      
      ymin = MIN(y0, ymin);
      ymax = MAX(y1, ymax);
      
      x0 = 0;
      xsteps = 0;
      xstep_done = false;
      
      while(!xstep_done && (!interrupt_raytracing))
      {
	x1 = x0 + stepsize;
	
	if(x1 >= width)
	{
	  x1 = width;
	  xstep_done = true;
	}
	
	if(!skip || (ysteps & 1) || (xsteps & 1))
	{	    
	  col = callback(x0, height - y0 - 1, data);
	  RadianceToRGB(col, &pixelRGB);
	  FillRect(x0, y0, x1, y1, pixelRGB, rgb);

	  rt_pixcount++;
	  
#ifndef NO_EVENT_TIMER
	  if(iState.wake_up & WAKE_UP_PROCESS)
#else
	  if(rt_pixcount & 0x0F)
#endif
	  {
	    ProcessWaitingEvents();
	    iState.wake_up &= ~WAKE_UP_PROCESS;
	  }
	  
#ifndef NO_EVENT_TIMER
	  if(iState.wake_up & WAKE_UP_RENDER)
#else
	  if(rt_pixcount & 0x7F)
#endif
	  {
	    iState.wake_up &= ~WAKE_UP_RENDER;
	    if((ymax > 0) && (ymax > ymin))
		RenderPixels(0,ymin, width, ymax - ymin, 
			     rgb + ymin * width);

	    ymin = MAX(0, ymax - stepsize);
	    ymax = ymax;	    
	  }
	  
	} // Skiptest
	
	x0 = x1;
	xsteps++;
      } // while xstep

      if(ymax >= height)
      {
	if((ymax > ymin))
	  RenderPixels(0,ymin, width, ymax - ymin, 
		       rgb + ymin * width);
	ymax = -1;
      }
      
      y0 = y1;
      ysteps++;
    } // while ystep
    
    skip = true;
    stepsize /= 2;
    
  } // while not done

  delete[] rgb;

  ScreenIterateFinish();
}
