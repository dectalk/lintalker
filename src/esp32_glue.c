#include "Macintosh.h"
#include <stdint.h>

/* Global state variables used by the speech engine, 
 * originally defined in main.c/Wavinout.c which were excluded from the build. 
 */

shellVarPtr gInstanceStorage = NULL;
uintptr_t   gComponentRefcon = 0;
short       gSpeechIsDone    = 1;
