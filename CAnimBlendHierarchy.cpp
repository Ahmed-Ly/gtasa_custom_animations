#include "wrapper.h"

hCAnimBlendHierarchy_Constructor OLD_CAnimBlendHierarchy_Constructor = (hCAnimBlendHierarchy_Constructor)0x4CF270;

void CAnimBlendHierarchy_Constructor ( CAnimBlendHierarchy * pAnimHierarchy )
{
    int pThis = (int)pAnimHierarchy;


    *(DWORD *)(pThis + 4) = 0;
    *(WORD *)(pThis + 8) = 0;
    *(BYTE *)(pThis + 10) = 0;
    *(BYTE *)(pThis + 11) = 0;
    *(DWORD *)(pThis + 12) = -1;
    *(DWORD *)(pThis + 16) = 0;
    *(DWORD *)(pThis + 20) = 0;
    
}

