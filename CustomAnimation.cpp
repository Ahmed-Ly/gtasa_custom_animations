#include <fstream>
#include "wrapper.h"
#include "IFPLoader.h"

extern std::vector <IFP> g_IFPs;

extern std::ofstream ofs;
extern std::wofstream ofsW;

extern bool g_bAllowToPlayCustomAnimation;

extern DWORD g_CurrentHierarchyAnimationHash;

extern hGetUppercaseKey OLD_GetUppercaseKey;

extern hGetAnimationBlock        OLD_GetAnimationBlock;
extern hGetAnimation             OLD_GetAnimation;

extern hGetAnimBlockName OLD_GetAnimBlockName;
extern hAddAnimAssocDefinition OLD_AddAnimAssocDefinition;
extern hRegisterAnimBlock OLD_RegisterAnimBlock;
extern hCreateAssociations_Clump OLD_CreateAssociations_Clump;
extern hCreateAssociations OLD_CreateAssociations;

extern hRpAnimBlendClumpGetAssociation OLD_RpAnimBlendClumpGetAssociation;
extern hRpAnimBlendClumpGetAssociationAnimId OLD_RpAnimBlendClumpGetAssociationAnimId;

extern hGetAnimAssociationAnimId OLD_GetAnimAssociationAnimId;

extern hCreateAnimAssociation OLD_CreateAnimAssociation;
extern hAddAnimation OLD_AddAnimation;
extern hAddAnimationAndSync OLD_AddAnimationAndSync;
extern hGetAnimGroupName OLD_GetAnimGroupName;

extern hCAnimBlendAssociation_Init_reference OLD_CAnimBlendAssociation_Init_reference;
extern hCAnimBlendAssociation_Init OLD_CAnimBlendAssociation_Init;

extern hLoadAnimFile_stream OLD_LoadAnimFile_stream;
extern hAddAnimation_hier OLD_AddAnimation_hier;

extern hCAnimBlendStaticAssociation_Constructor OLD_CAnimBlendStaticAssociation_Constructor;
extern hCAnimBlendStaticAssociation_Init OLD_CAnimBlendStaticAssociation_Init;

extern hCAnimBlendAssocGroup_CopyAnimation CAnimBlendAssocGroup_CopyAnimation  ;
extern hCAnimBlendAssociation_SyncAnimation CAnimBlendAssociation_SyncAnimation;
extern hCAnimBlendAssociation_Start         CAnimBlendAssociation_Start  ;
extern hUncompressAnimation                 UncompressAnimation;
extern hCAnimBlendAssociation_Constructor_staticAssocRef OLD_CAnimBlendAssociation_Constructor_staticAssocRef ;

CAnimBlendAssociation *__cdecl CustomAddAnimation (int pClump, CAnimBlendStaticAssociation & CustomAnimStaticAssoc);
CAnimBlendAssociation * __cdecl CustomAddAnimationAndSync (int pClump, CAnimBlendAssociation * pAnimAssocToSyncWith, CAnimBlendStaticAssociation & CustomAnimStaticAssoc);


CAnimBlendAssociation * CreateCustomAnimation ( CAnimBlendAssociation * pAnimAssocToSyncWith, void * pClump, DWORD animGroup, DWORD animID )
{
    CAnimBlendStaticAssociation * pAnimStaticAssocToModify = GetAnimStaticAssocBy_GroupId_AnimId(animGroup, animID);

    CAnimBlendHierarchy* pAnimBlendHierarchy1       = pAnimStaticAssocToModify->m_pAnimBlendHier;
                
    CAnimBlendHierarchy* pCustomAnimBlendHierarchy = &g_IFPs[0].AnimationHierarchies[145];
    pCustomAnimBlendHierarchy->m_hashKey      = pAnimBlendHierarchy1->m_hashKey;
    pCustomAnimBlendHierarchy->m_nAnimBlockId = pAnimBlendHierarchy1->m_nAnimBlockId;
               
                
    CAnimBlendStaticAssociation CustomAnimStaticAssoc;
    OLD_CAnimBlendStaticAssociation_Constructor ( &CustomAnimStaticAssoc );

    CustomAnimStaticAssoc.m_nFlags = pAnimStaticAssocToModify->m_nFlags;

    OLD_CAnimBlendStaticAssociation_Init ( &CustomAnimStaticAssoc, pClump, pCustomAnimBlendHierarchy);

    CustomAnimStaticAssoc.m_nAnimGroup = pAnimStaticAssocToModify->m_nAnimGroup;
    CustomAnimStaticAssoc.m_nAnimID    = pAnimStaticAssocToModify->m_nAnimID;

    if (pAnimAssocToSyncWith != nullptr)
    {
        return CustomAddAnimationAndSync ( (int)pClump, pAnimAssocToSyncWith, CustomAnimStaticAssoc );
    }
    else
    {
        return CustomAddAnimation ( (int)pClump, CustomAnimStaticAssoc );
    }
}



CAnimBlendAssociation *__cdecl CustomAddAnimation(int pClump, CAnimBlendStaticAssociation & CustomAnimStaticAssoc)
{
   //ofs << "OriginalAddAnimation called" << std::endl;



 CAnimBlendAssociation * pAnimAssoc; // esi
  int *clumpData; // edi
  int *next; // eax
  DWORD *tempAssoc; // eax
  int *nextAssoc; // ecx

  // We need to remove this line and add some code here for running animations simultaneously
  //pAnimAssoc = CAnimBlendAssocGroup_CopyAnimation((DWORD *) (*(DWORD*)0x00B4EA34) + 5 * GroupID, AnimID);

    UncompressAnimation ( CustomAnimStaticAssoc.m_pAnimBlendHier );
    pAnimAssoc = (CAnimBlendAssociation *)malloc(sizeof(CAnimBlendAssociation));
    OLD_CAnimBlendAssociation_Constructor_staticAssocRef ( pAnimAssoc, CustomAnimStaticAssoc);

  //ofs << "Done calling  CAnimBlendAssocGroup_CopyAnimation " << std::endl;

  clumpData = *(int **)(   (*(DWORD*)0xB5F878) + pClump);

  if ( !((*((BYTE *)pAnimAssoc + 46) >> 5) & 1) )
    goto LABEL_5;
  next = (int *)*clumpData;
  if ( !*clumpData )
    goto LABEL_5;
  while ( !((*((BYTE *)next + 42) >> 5) & 1) )
  {
    next = (int *)*next;
    if ( !next )
      goto LABEL_5;
  }
  if ( next )
  {
    CAnimBlendAssociation_SyncAnimation(pAnimAssoc, (CAnimBlendAssociation *)(next - 1));
    *((BYTE *)pAnimAssoc + 46) |= 1u;
  }
  else
  {
LABEL_5:
    CAnimBlendAssociation_Start(pAnimAssoc, 0);
  }

  tempAssoc = ((DWORD*)pAnimAssoc) + 1;

  if ( *clumpData )                             // clumpData->nextAssoc
    *(DWORD *)(*clumpData + 4) = (DWORD)tempAssoc;

  nextAssoc = (int *)*clumpData;

  DWORD * dwpAnimAssoc = (DWORD*) pAnimAssoc;

  dwpAnimAssoc[2] = (DWORD)clumpData;
  //pAnimAssoc[2] = clumpData;

  *tempAssoc = (DWORD)nextAssoc;

  *clumpData = (int)tempAssoc;                  // clumpData->nextAssoc = (int)tempAssoc

  //ofs << "Exiting OriginalAddAnimation " << std::endl;

  return pAnimAssoc;

}


CAnimBlendAssociation * __cdecl CustomAddAnimationAndSync(int pClump, CAnimBlendAssociation * pAnimAssocToSyncWith, CAnimBlendStaticAssociation & CustomAnimStaticAssoc)
{
  CAnimBlendAssociation *pAnimAssoc; // esi
  int *pClumpData; // edi
  DWORD *tempAssoc; // eax
  int nextAssoc; // ecx

  // We need to remove this line and add some code here for running animations simultaneously
  //pAnimAssoc = CAnimBlendAssocGroup_CopyAnimation((DWORD *) (*(DWORD*)0x00B4EA34) + 5 * GroupID, AnimID);

    UncompressAnimation ( CustomAnimStaticAssoc.m_pAnimBlendHier );
    pAnimAssoc = (CAnimBlendAssociation *)malloc(sizeof(CAnimBlendAssociation));
    OLD_CAnimBlendAssociation_Constructor_staticAssocRef ( pAnimAssoc, CustomAnimStaticAssoc);

  pClumpData = *(int **)(   (*(DWORD*)0xB5F878) + pClump);
  if ( (*((BYTE *)pAnimAssoc + 46) >> 5) & 1 && pAnimAssocToSyncWith )
  {
    CAnimBlendAssociation_SyncAnimation ( pAnimAssoc, pAnimAssocToSyncWith);
    *((BYTE *)pAnimAssoc + 46) |= 1u;
  }
  else
  {
    CAnimBlendAssociation_Start ( pAnimAssoc, 0);
  }

  tempAssoc = ((DWORD*)pAnimAssoc) + 1;

  if ( *pClumpData )
    *(DWORD *)(*pClumpData + 4) = (DWORD)tempAssoc;

  nextAssoc = *pClumpData;

   DWORD * dwpAnimAssoc = (DWORD*) pAnimAssoc;

  dwpAnimAssoc[2] = (DWORD)pClumpData;

  *tempAssoc = nextAssoc;

  *pClumpData = (int)tempAssoc;

  return pAnimAssoc;
}





CAnimBlendAssociation *__cdecl OriginalAddAnimation(int pClump, int GroupID, int AnimID)
{
   //ofs << "OriginalAddAnimation called" << std::endl;



 CAnimBlendAssociation * pAnimAssoc; // esi
  int *clumpData; // edi
  int *next; // eax
  DWORD *tempAssoc; // eax
  int *nextAssoc; // ecx
                                                // We need to remove this line and add some code here for running animations simultaneously
  pAnimAssoc = CAnimBlendAssocGroup_CopyAnimation((DWORD *) (*(DWORD*)0x00B4EA34) + 5 * GroupID, AnimID);

  //ofs << "Done calling  CAnimBlendAssocGroup_CopyAnimation " << std::endl;

  clumpData = *(int **)(   (*(DWORD*)0xB5F878) + pClump);

  if ( !((*((BYTE *)pAnimAssoc + 46) >> 5) & 1) )
    goto LABEL_5;
  next = (int *)*clumpData;
  if ( !*clumpData )
    goto LABEL_5;
  while ( !((*((BYTE *)next + 42) >> 5) & 1) )
  {
    next = (int *)*next;
    if ( !next )
      goto LABEL_5;
  }
  if ( next )
  {
    CAnimBlendAssociation_SyncAnimation(pAnimAssoc, (CAnimBlendAssociation *)(next - 1));
    *((BYTE *)pAnimAssoc + 46) |= 1u;
  }
  else
  {
LABEL_5:
    CAnimBlendAssociation_Start(pAnimAssoc, 0);
  }

  tempAssoc = ((DWORD*)pAnimAssoc) + 1;

  if ( *clumpData )                             // clumpData->nextAssoc
    *(DWORD *)(*clumpData + 4) = (DWORD)tempAssoc;

  nextAssoc = (int *)*clumpData;

  DWORD * dwpAnimAssoc = (DWORD*) pAnimAssoc;

  dwpAnimAssoc[2] = (DWORD)clumpData;
  //pAnimAssoc[2] = clumpData;

  *tempAssoc = (DWORD)nextAssoc;

  *clumpData = (int)tempAssoc;                  // clumpData->nextAssoc = (int)tempAssoc

  //ofs << "Exiting OriginalAddAnimation " << std::endl;

  return pAnimAssoc;

}




CAnimBlendAssociation * __cdecl OriginalAddAnimationAndSync(int pClump, CAnimBlendAssociation * pAnimAssocToSyncWith, int GroupID, int AnimID)
{
  CAnimBlendAssociation *pAnimAssoc; // esi
  int *pClumpData; // edi
  DWORD *tempAssoc; // eax
  int nextAssoc; // ecx

  pAnimAssoc = CAnimBlendAssocGroup_CopyAnimation((DWORD *) (*(DWORD*)0x00B4EA34) + 5 * GroupID, AnimID);
  pClumpData = *(int **)(   (*(DWORD*)0xB5F878) + pClump);
  if ( (*((BYTE *)pAnimAssoc + 46) >> 5) & 1 && pAnimAssocToSyncWith )
  {
    CAnimBlendAssociation_SyncAnimation ( pAnimAssoc, pAnimAssocToSyncWith);
    *((BYTE *)pAnimAssoc + 46) |= 1u;
  }
  else
  {
    CAnimBlendAssociation_Start ( pAnimAssoc, 0);
  }

  tempAssoc = ((DWORD*)pAnimAssoc) + 1;

  if ( *pClumpData )
    *(DWORD *)(*pClumpData + 4) = (DWORD)tempAssoc;

  nextAssoc = *pClumpData;

   DWORD * dwpAnimAssoc = (DWORD*) pAnimAssoc;

  dwpAnimAssoc[2] = (DWORD)pClumpData;

  *tempAssoc = nextAssoc;

  *pClumpData = (int)tempAssoc;

  return pAnimAssoc;
}