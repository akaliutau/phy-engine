/* rtstochastic_priv.h : Private declarations */

#ifndef _RAYMATTING_PRIV_H_
#define _RAYMATTING_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/* control panel */
extern void CreateRMControlPanel(void *parent_widget);
extern void RM_ShowControlPanel(void);
extern void RM_HideControlPanel(void);

extern void RM_Init(void);
extern void RM_Interrupt(void);
extern void RM_Trace(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /* _RAYMATTING_PRIV_H_ */ 
