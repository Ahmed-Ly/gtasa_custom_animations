#include "wrapper.h"

hCAnimBlendSequence_Constructor  OLD_CAnimBlendSequence_Constructor = (hCAnimBlendSequence_Constructor)0x4D0C10;
hCAnimBlendSequence_Deconstructor OLD_CAnimBlendSequence_Deconstructor = (hCAnimBlendSequence_Deconstructor)0x4D0C30; 

void CAnimBlendSequence_Constructor (CAnimBlendSequence * pSequence)
{
	WORD * pThis = (WORD *)pSequence;

	pThis[2] = 0;
	pThis[3] = 0;
	*((DWORD *)pThis + 2) = 0;
	*pThis = -1;
}

/*
// The keyframes will not be freed if the deconstructor is not there, but
// it would be better to put the frames in a vector container for better control.
// I prefer to not use the deconstructor, so I guess we don't need to uncomment this

CAnimBlendSequence::~CAnimBlendSequence()
{
	OLD_CAnimBlendSequence_Deconstructor();
}
*/