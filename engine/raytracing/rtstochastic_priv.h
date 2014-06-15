/* rtstochastic_priv.h : Private declarations */

#ifndef _RTSTOCHASTIC_PRIV_H_
#define _RTSTOCHASTIC_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/* control panel */
extern void CreateRTStochasticControlPanel(void *parent_widget);
extern void RTStochastic_ShowControlPanel(void);
extern void RTStochastic_HideControlPanel(void);

extern void RTStochastic_Init(void);
extern void RTStochastic_Interrupt(void);
extern void RTStochastic_Trace(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /* _RTSTOCHASTIC_PRIV_H_ */ 

