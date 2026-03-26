#ifndef f2p_h
#define f2p_h

#include "main.h"

void DMA2DICCF(DICCF_t *DICCF, uint32_t *buffer1, uint32_t *buffer2);
void DIG2DICCF(DICCF_t *DICCF);
void DICCF2DICCP(DICCF_t *DICCF, DICCP_t *DICCP);

#endif
