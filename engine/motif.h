/* motif.h: solves some incompatibilities between Motif 1.2 and Motif 2.0 */

#ifndef _PHY_MOTIF_H_
#define _PHY_MOTIF_H_

/* toggle buttons can have three states in Motif 2.0: set, unset and
 * undetermined, while they can only have to states in Motif 1.2:
 * set and unset. The symbols XmSET and XmUNSET are defined in Motif 2.0 
 * only */
#ifndef XmSET
#define XmSET	True
#endif /*XmSET*/

#ifndef XmUNSET
#define XmUNSET	False
#endif /*XmUNSET*/

#endif /*_PHY_MOTIF_H_*/
