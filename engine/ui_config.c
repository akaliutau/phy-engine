#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/RepType.h>

#include <error.h>

#include "ui.h"
#include "defaults.h"
#include "appdata.h"
#include "vector.h"
#include "tonemapping.h"



APPDATA appData;


#define XmNrecentFile "recentFile"
#define XmCrecentFile "recentFile"

#define SuffixSpec "_spec"
#define SuffixCamEye  "_cam_eye"
#define SuffixCamLook  "_cam_look"
#define SuffixCamUp  "_cam_up"
#define SuffixCamFOV  "_cam_fov"
#define SuffixACamEye	"_acam_eye"
#define SuffixACamLook	"_acam_look"
#define SuffixACamUp	"_acam_up"
#define SuffixACamFOV	"_acam_fov"

#define RedGamma "redgamma"
#define GreenGamma "greengamma"
#define BlueGamma "bluegamma"

static XtResource appResources[] =
{
  {
    XmNrecentFile "0" SuffixSpec,
    XmCrecentFile "0" SuffixSpec,
    XmRString,
    sizeof(char *),
    XtOffsetOf(APPDATA, recentFile[0].spec),
    XmRString,
    "-"
  },
  {
    XmNrecentFile "1" SuffixSpec,
    XmCrecentFile "1" SuffixSpec,
    XmRString,
    sizeof(char *),
    XtOffsetOf(APPDATA, recentFile[1].spec),
    XmRString,
    "-"
  },
  {
    XmNrecentFile "2" SuffixSpec,
    XmCrecentFile "2" SuffixSpec,
    XmRString,
    sizeof(char *),
    XtOffsetOf(APPDATA, recentFile[2].spec),
    XmRString,
    "-"
  },
  {
    XmNrecentFile "3" SuffixSpec,
    XmCrecentFile "3" SuffixSpec,
    XmRString,
    sizeof(char *),
    XtOffsetOf(APPDATA, recentFile[3].spec),
    XmRString,
    "-"
  },
  {
    XmNrecentFile "4" SuffixSpec,
    XmCrecentFile "4" SuffixSpec,
    XmRString,
    sizeof(char *),
    XtOffsetOf(APPDATA, recentFile[4].spec),
    XmRString,
    "-"
  },
  {
    XmNrecentFile "5" SuffixSpec,
    XmCrecentFile "5" SuffixSpec,
    XmRString,
    sizeof(char *),
    XtOffsetOf(APPDATA, recentFile[5].spec),
    XmRString,
    "-"
  },

  

  
  {
    XmNrecentFile "0" SuffixCamEye,
    XmCrecentFile "0" SuffixCamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[0].cam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "0" SuffixCamLook,
    XmCrecentFile "0" SuffixCamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[0].cam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "0" SuffixCamUp,
    XmCrecentFile "0" SuffixCamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[0].cam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "0" SuffixCamFOV,
    XmCrecentFile "0" SuffixCamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[0].cam.fov),
    XmRString,
    "22.5"
  },


  
  {
    XmNrecentFile "1" SuffixCamEye,
    XmCrecentFile "1" SuffixCamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[1].cam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "1" SuffixCamLook,
    XmCrecentFile "1" SuffixCamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[1].cam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "1" SuffixCamUp,
    XmCrecentFile "1" SuffixCamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[1].cam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "1" SuffixCamFOV,
    XmCrecentFile "1" SuffixCamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[1].cam.fov),
    XmRString,
    "22.5"
  },


  
  {
    XmNrecentFile "2" SuffixCamEye,
    XmCrecentFile "2" SuffixCamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[2].cam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "2" SuffixCamLook,
    XmCrecentFile "2" SuffixCamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[2].cam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "2" SuffixCamUp,
    XmCrecentFile "2" SuffixCamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[2].cam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "2" SuffixCamFOV,
    XmCrecentFile "2" SuffixCamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[2].cam.fov),
    XmRString,
    "22.5"
  },


  
  {
    XmNrecentFile "3" SuffixCamEye,
    XmCrecentFile "3" SuffixCamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[3].cam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "3" SuffixCamLook,
    XmCrecentFile "3" SuffixCamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[3].cam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "3" SuffixCamUp,
    XmCrecentFile "3" SuffixCamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[3].cam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "3" SuffixCamFOV,
    XmCrecentFile "3" SuffixCamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[3].cam.fov),
    XmRString,
    "22.5"
  },

  
  {
    XmNrecentFile "4" SuffixCamEye,
    XmCrecentFile "4" SuffixCamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[4].cam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "4" SuffixCamLook,
    XmCrecentFile "4" SuffixCamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[4].cam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "4" SuffixCamUp,
    XmCrecentFile "4" SuffixCamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[4].cam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "4" SuffixCamFOV,
    XmCrecentFile "4" SuffixCamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[4].cam.fov),
    XmRString,
    "22.5"
  },

  
  {
    XmNrecentFile "5" SuffixCamEye,
    XmCrecentFile "5" SuffixCamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[5].cam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "5" SuffixCamLook,
    XmCrecentFile "5" SuffixCamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[5].cam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "5" SuffixCamUp,
    XmCrecentFile "5" SuffixCamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[5].cam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "5" SuffixCamFOV,
    XmCrecentFile "5" SuffixCamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[5].cam.fov),
    XmRString,
    "22.5"
  },

  

  
  {
    XmNrecentFile "0" SuffixACamEye,
    XmCrecentFile "0" SuffixACamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[0].acam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "0" SuffixACamLook,
    XmCrecentFile "0" SuffixACamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[0].acam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "0" SuffixACamUp,
    XmCrecentFile "0" SuffixACamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[0].acam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "0" SuffixACamFOV,
    XmCrecentFile "0" SuffixACamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[0].acam.fov),
    XmRString,
    "22.5"
  },


  
  {
    XmNrecentFile "1" SuffixACamEye,
    XmCrecentFile "1" SuffixACamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[1].acam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "1" SuffixACamLook,
    XmCrecentFile "1" SuffixACamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[1].acam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "1" SuffixACamUp,
    XmCrecentFile "1" SuffixACamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[1].acam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "1" SuffixACamFOV,
    XmCrecentFile "1" SuffixACamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[1].acam.fov),
    XmRString,
    "22.5"
  },


  
  {
    XmNrecentFile "2" SuffixACamEye,
    XmCrecentFile "2" SuffixACamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[2].acam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "2" SuffixACamLook,
    XmCrecentFile "2" SuffixACamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[2].acam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "2" SuffixACamUp,
    XmCrecentFile "2" SuffixACamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[2].acam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "2" SuffixACamFOV,
    XmCrecentFile "2" SuffixACamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[2].acam.fov),
    XmRString,
    "22.5"
  },


  
  {
    XmNrecentFile "3" SuffixACamEye,
    XmCrecentFile "3" SuffixACamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[3].acam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "3" SuffixACamLook,
    XmCrecentFile "3" SuffixACamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[3].acam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "3" SuffixACamUp,
    XmCrecentFile "3" SuffixACamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[3].acam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "3" SuffixACamFOV,
    XmCrecentFile "3" SuffixACamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[3].acam.fov),
    XmRString,
    "22.5"
  },

  
  {
    XmNrecentFile "4" SuffixACamEye,
    XmCrecentFile "4" SuffixACamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[4].acam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "4" SuffixACamLook,
    XmCrecentFile "4" SuffixACamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[4].acam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "4" SuffixACamUp,
    XmCrecentFile "4" SuffixACamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[4].acam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "4" SuffixACamFOV,
    XmCrecentFile "4" SuffixACamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[4].acam.fov),
    XmRString,
    "22.5"
  },

  
  {
    XmNrecentFile "5" SuffixACamEye,
    XmCrecentFile "5" SuffixACamEye,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[5].acam.eyep),
    XmRString,
    "10 0 0"
  },
  {
    XmNrecentFile "5" SuffixACamLook,
    XmCrecentFile "5" SuffixACamLook,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[5].acam.lookp),
    XmRString,
    "0 0 0"
  },
  {
    XmNrecentFile "5" SuffixACamUp,
    XmCrecentFile "5" SuffixACamUp,
    XmRVector,
    sizeof(VECTOR),
    XtOffsetOf(APPDATA, recentFile[5].acam.updir),
    XmRString,
    "0 0 1"
  },
  {
    XmNrecentFile "5" SuffixACamFOV,
    XmCrecentFile "5" SuffixACamFOV,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, recentFile[5].acam.fov),
    XmRString,
    "22.5"
  },

  {
    RedGamma,
    RedGamma,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, redgamma),
    XmRString,
    "0.0"
  },
  {
    GreenGamma,
    GreenGamma,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, greengamma),
    XmRString,
    "0.0"
  },
  {
    BlueGamma,
    BlueGamma,
    XmRFloat,
    sizeof(float),
    XtOffsetOf(APPDATA, bluegamma),
    XmRString,
    "0.0"
  }
};




#define XmNphyrcFilename "phyrcFilename"
#define XmCphyrcFilename "phyrcFilename"

static XtResource appFilenameResource[] =
{
  {
    XmNphyrcFilename,
    XmCphyrcFilename,
    XmRString,
    sizeof(char *),
    XtOffsetOf(APPDATA, phyrcFilename),
    XmRString,
    "%H/.phyrc"
  }
};




#define SUBST_INDEX_H 0

static SubstitutionRec substitutions[] =
{
  {
    'H',
    NULL
  }
};






static Boolean ConvertStringToVector(Display *display,
				     XrmValuePtr args,
				     Cardinal *num_args,
				     XrmValuePtr fromVal,
				     XrmValuePtr toVal,
				     XtPointer *ptr)
{
  static VECTOR vec;
  

  if(*num_args != 0)
  {
    XtWarningMsg("wrongParameters", "ConvertStringToVector",
		 "Xt error", "String to Vector conversion"
		 "needs no extra parameters", (String *)NULL,
		 (Cardinal *)NULL);
  }

  if(sscanf((char *)fromVal->addr, "%f %f %f", &(vec.x), &(vec.y), &(vec.z)) == 3)
  {
    if(toVal->addr)
    {
      if(toVal->size < sizeof(VECTOR))
      {
	toVal->size = sizeof(VECTOR);
	return False;
      }
      else
      {
	*((VECTOR *)toVal->addr) = vec;
      }
    }
    else
    {
      (*toVal).size = sizeof(VECTOR);
      (*toVal).addr = (XtPointer) &vec;
    }
    return TRUE;
  }
  else
  {
    XtDisplayStringConversionWarning(display,
				     (char *)fromVal->addr, XmRVector);
    return(FALSE);
  }
}






static Boolean CheckIfFileCreatable(String filespec)
{
  FILE *fp;

  fp = fopen(filespec, "w");

  if(fp)
  {
    fclose(fp);
    return(True);
  }
  else
  {
    return(False);
  }
}





extern void LoadUserOptions(void)
{
  Display *display;
  XrmDatabase rdb;
  char *filename;

  

  XtSetTypeConverter(XmRString, XmRVector,
		     ConvertStringToVector,
		     (XtConvertArgList) NULL,
		     0,
		     XtCacheNone,
		     NULL);

  

  

  substitutions[SUBST_INDEX_H].substitution = getenv("HOME");

  
  XtVaGetApplicationResources(topLevel,
			      &appData,
			      appFilenameResource,
			      XtNumber(appFilenameResource),
			      NULL);
			      
  

  display = XtDisplay(topLevel);
  rdb = XtDatabase(display);

  filename = XtFindFile(appData.phyrcFilename, substitutions, 
			XtNumber(substitutions), NULL);

  if(!(filename && filename[0]))
  {
    

    filename = XtFindFile(appData.phyrcFilename, substitutions,
			  XtNumber(substitutions), CheckIfFileCreatable);

    if(!(filename && filename[0]))
    {
      Error("LoadUserOptions", "Cannot create a user resource file");
    }
  }

  
  appData.phyrcFilename = filename;

  if(filename && filename[0])
  {
    
    
    XrmCombineFileDatabase(filename, &rdb, False);
  }

  

  XtVaGetApplicationResources(topLevel,
			      &appData,
			      appResources,
			      XtNumber(appResources),
			      NULL);

  
  if (appData.redgamma != 0.0 ||
      appData.greengamma != 0.0 ||
      appData.bluegamma != 0.0) {
    RGBSET(tmopts.gamma, appData.redgamma, appData.greengamma, appData.bluegamma);
  }

  return;
}







extern void SaveUserOptions(void)
{
  FILE *fp;
  int i;

  

  fp = fopen(appData.phyrcFilename, "w");

  if(!fp)
  {
    Error("SaveUserOptions", "Can't open phyrc file");
    return;
  }

  

  fprintf(fp, "!phyrc : resource file for Phy2\n\n");

  

  for(i = 0; i < NUM_RECENT_FILES; i++)
  {
    if(appData.recentFile[i].spec != NULL)
    {
      CAMERA *cam = &(appData.recentFile[i].cam);
      CAMERA *acam = &(appData.recentFile[i].acam);

      fprintf(fp, "*recentFile%i%s: %s\n", i, SuffixSpec,
	      appData.recentFile[i].spec);
      
      fprintf(fp, "*recentFile%i%s: %f %f %f\n", i, SuffixCamEye,
	      cam->eyep.x, cam->eyep.y, cam->eyep.z);
      fprintf(fp, "*recentFile%i%s: %f %f %f\n", i, SuffixCamLook,
	      cam->lookp.x, cam->lookp.y, cam->lookp.z);
      fprintf(fp, "*recentFile%i%s: %f %f %f\n", i, SuffixCamUp,
	      cam->updir.x, cam->updir.y, cam->updir.z);
      fprintf(fp, "*recentFile%i%s: %f\n", i, SuffixCamFOV,
	      cam->fov);

      fprintf(fp, "*recentFile%i%s: %f %f %f\n", i, SuffixACamEye,
	      acam->eyep.x, acam->eyep.y, acam->eyep.z);
      fprintf(fp, "*recentFile%i%s: %f %f %f\n", i, SuffixACamLook,
	      acam->lookp.x, acam->lookp.y, acam->lookp.z);
      fprintf(fp, "*recentFile%i%s: %f %f %f\n", i, SuffixACamUp,
	      acam->updir.x, acam->updir.y, acam->updir.z);
      fprintf(fp, "*recentFile%i%s: %f\n", i, SuffixACamFOV,
	      acam->fov);

      fprintf(fp, "\n");
    }
  }

  fprintf(fp, "*redgamma: %g\n", tmopts.gamma.r);
  fprintf(fp, "*greengamma: %g\n", tmopts.gamma.g);
  fprintf(fp, "*bluegamma: %g\n", tmopts.gamma.b);

  fclose(fp);
}
