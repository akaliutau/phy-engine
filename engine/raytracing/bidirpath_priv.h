/* bidirpath_priv.h */

#ifndef _BIDIRPATH_PRIV_H_
#define _BIDIRPATH_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/* control panel */
extern void CreateBidirPathControlPanel(void *parent_widget);
extern void BidirPathShowControlPanel(void);
extern void BidirPathHideControlPanel(void);


extern void BidirPathInterrupt(void);
extern void BidirPathTrace(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /* _BIDIRPATH_PRIV_H_ */ 

