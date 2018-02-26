                                                                                                                                                                                                                                                                                                                                                                                                     #include <Windows.h>
#include <fstream>
#include "rwcore.h"
#include "rpworld.h"
#include "wrapper.h"
#include "IFPLoader.h"
#pragma comment(lib, "rwcore.lib")
#pragma comment(lib, "rpworld.lib")

extern std::vector <IFP> g_IFPs;

extern std::ofstream ofs;
extern std::wofstream ofsW;

extern bool g_PlayCustomAnimations;

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
extern hGetAnimGroupName OLD_GetAnimGroupName;

extern hCAnimBlendAssociation_Init_reference OLD_CAnimBlendAssociation_Init_reference;
extern hCAnimBlendAssociation_Init OLD_CAnimBlendAssociation_Init;

extern hLoadAnimFile_stream OLD_LoadAnimFile_stream;
extern hAddAnimation_hier OLD_AddAnimation_hier;

extern hCAnimBlendStaticAssociation_Constructor OLD_CAnimBlendStaticAssociation_Constructor;
extern hCAnimBlendStaticAssociation_Init OLD_CAnimBlendStaticAssociation_Init;


char * __cdecl NEW_AddAnimAssocDefinition 
(
    const char *   BlockName,
    const char *   AnimName,
    unsigned long  AnimationGroup,
    unsigned long  AnimationID,
    int            Descriptor
)
{
    /*
    ofs << "NEW_AddAnimAssocDefinition: called, arguements: " << std::endl 
        << "BlockName = "<< BlockName << ", " <<  "AnimName = " << AnimName << ", "
        << "AnimationGroup = " << AnimationGroup << ", " << "AnimationID = " << AnimationID << std::endl << std::endl;
        */
  // MessageBox(0, "NEW_AddAnimAssocDefinition: called!!!\n", "Hi", MB_ICONINFORMATION);

    return OLD_AddAnimAssocDefinition (BlockName, AnimName, AnimationGroup, AnimationID, Descriptor);
}



int __cdecl NEW_RegisterAnimBlock
(
    const char * szName
)
{
    
   // ofs << "NEW_RegisterAnimBlock: called, arguements: " << std::endl
    //    << "szName = " << szName << std::endl << std::endl;
      
    return OLD_RegisterAnimBlock ( szName );
}


void WINAPI NEW_CreateAssociations_Clump
(
    char const* szIfpFile,
    void * pClump,
    char** pszAnimations,
    int NumAnimations
)
{
    void * pThis;
    __asm mov pThis, ecx;


    __asm mov ecx, pThis;
    OLD_CreateAssociations_Clump(szIfpFile, pClump, pszAnimations, NumAnimations);

    ofs << "NEW_CreateAssociations_Clump: Called" << "   |  szIfpFile:  " << szIfpFile << std::endl;
    
}

void WINAPI NEW_CreateAssociations
(
    char const* szIfpFile,
    char const* arg2,
    char const* arg3,
    int arg4
)
{
    void * pThis;
    __asm mov pThis, ecx;


    ofs << "NEW_CreateAssociations: " << std::endl
        << "szIfpFile: " << szIfpFile << std::endl
        << "arg2: " << arg2 << std::endl << "arg3: " << arg3 << std::endl
        << "arg4: " << arg4  << std::endl << std::endl;
    
    __asm mov ecx, pThis;
    return OLD_CreateAssociations ( szIfpFile, arg2, arg3, arg4 );
}


CAnimBlendAssociation * NEW_CreateAnimAssociation
(
    DWORD animGroup,
    DWORD animID
)
{
    ofs << "NEW_CreateAnimAssociation: " << std::endl;


    ofs << "animGroup: "
        << animGroup << std::endl << "animID: " << animID << std::endl;

    CAnimBlendAssociation * pAnimAssoc = OLD_CreateAnimAssociation(animGroup, animID);

    ofs << std::endl;


    return pAnimAssoc;
}


hCAnimBlendAssocGroup_CopyAnimation CAnimBlendAssocGroup_CopyAnimation   = (hCAnimBlendAssocGroup_CopyAnimation)0x004CE130;
hCAnimBlendAssociation_SyncAnimation CAnimBlendAssociation_SyncAnimation = (hCAnimBlendAssociation_SyncAnimation)0x004CEB40;
hCAnimBlendAssociation_Start         CAnimBlendAssociation_Start         = (hCAnimBlendAssociation_Start)0x004CEB70;
hUncompressAnimation                 UncompressAnimation                 = (hUncompressAnimation)0x4d41c0;
hCAnimBlendAssociation_Constructor_staticAssocRef OLD_CAnimBlendAssociation_Constructor_staticAssocRef = (hCAnimBlendAssociation_Constructor_staticAssocRef)0x4CF080;

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



CAnimBlendAssociation * NEW_AddAnimation
(
    void * pClump,
    DWORD animGroup,
    DWORD animID
)
{


    ofs << "NEW_AddAnimation: " << std::endl;


    ofs << "animGroup: "<< animGroup << std::endl 
        << "animID: " << animID << std::endl 
        <<"GroupName: " << OLD_GetAnimGroupName(animGroup) << std::endl;

    ofs << "BlockName: " << OLD_GetAnimBlockName(animGroup) << std::endl;
    

    CAnimBlendAssociation * pAnimAssoc;
    CAnimBlendHierarchy * pGatewayAnimHierarchy = GetAnimHierachyBy_GroupId_AnimId (animGroup, animID);


    if (pGatewayAnimHierarchy != nullptr)
    {
        if (
                (OLD_GetUppercaseKey("muscleidle_csaw") == pGatewayAnimHierarchy->m_hashKey)
                ||
                (OLD_GetUppercaseKey("run_stopR") == pGatewayAnimHierarchy->m_hashKey)
            )
        {

            if (g_PlayCustomAnimations)
            {
                ofs << "About to play custom Animation now " << std::endl;

                CAnimBlendStaticAssociation * pAnimStaticAssocToModify = GetAnimStaticAssocBy_GroupId_AnimId(animGroup, animID);

                CAnimBlendHierarchy* pAnimBlendHierarchy1       = pAnimStaticAssocToModify->m_pAnimBlendHier;
                
                CAnimBlendHierarchy* pCustomAnimBlendHierarchy = &g_IFPs[0].AnimationHierarchies[241];
                pCustomAnimBlendHierarchy->m_hashKey      = pAnimBlendHierarchy1->m_hashKey;
                pCustomAnimBlendHierarchy->m_nAnimBlockId = pAnimBlendHierarchy1->m_nAnimBlockId;
               
                
                CAnimBlendStaticAssociation CustomAnimStaticAssoc;
                OLD_CAnimBlendStaticAssociation_Constructor ( &CustomAnimStaticAssoc );

                CustomAnimStaticAssoc.m_nFlags = pAnimStaticAssocToModify->m_nFlags;

                OLD_CAnimBlendStaticAssociation_Init ( &CustomAnimStaticAssoc, pClump, pCustomAnimBlendHierarchy);

                CustomAnimStaticAssoc.m_nAnimGroup = pAnimStaticAssocToModify->m_nAnimGroup;
                CustomAnimStaticAssoc.m_nAnimID    = pAnimStaticAssocToModify->m_nAnimID;

                pAnimAssoc = CustomAddAnimation ( (int)pClump, CustomAnimStaticAssoc );
            }
            else
            { 
                pAnimAssoc = OLD_AddAnimation(pClump, animGroup, animID);
            } 
        }
        else
        {
            pAnimAssoc = OLD_AddAnimation(pClump, animGroup, animID);
        } 
    }
    else
    {
        ofs << "ERROR: pGatewayAnimHierarchy is nullptr. Calling OLD_AddAnimation " << std::endl;

        pAnimAssoc = OLD_AddAnimation(pClump, animGroup, animID);
    }
    

    const char * AnimationName = GetNameFromHash(pAnimAssoc->m_pAnimBlendHierarchy->m_hashKey);
    if (AnimationName != nullptr)
    {
        ofs << std::endl << "AnimationName: " << AnimationName << std::endl;

        printf(
            "pClump: %p  |  AnimationName: %s\n ",
            pClump, AnimationName);
    }
    else
    {
        ofs << "GetNameFromHash: could not get animation name" << std::endl;
        ofs << "HashKey ( Not Found ) : " << pAnimAssoc->m_pAnimBlendHierarchy->m_hashKey << std::endl;
    }



    ofs << std::endl;

    return  pAnimAssoc;
}


int __cdecl NEW_LoadAnimFile_stream
(
    int pStream,
    bool b1,
    const char * sz1
)
{
    int result;

    
    ofs << "NEW_LoadAnimFile_stream: called" << std::endl;


    
    result = OLD_LoadAnimFile_stream(pStream, b1, sz1);

    return result;
}

int GetNumAnimations (void)
{
    return *(int *)(VAR_CAnimManager_NumAnimations);
}

int GetNumAnimBlocks (void)
{
    return *(int *)(VAR_CAnimManager_NumAnimBlocks);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         