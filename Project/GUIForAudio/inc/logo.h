#ifndef _LOGO_H_
#define _LOGO_H_
#include <stdint.h>

void LOGOCoordsInit(void);
int32_t LOGODraw(void);
int32_t LOGODrawFlush(void);
int32_t LOGODrawNoFlush(void);
int32_t LOGODrawStop(void);

#endif // _LOGO_H_
