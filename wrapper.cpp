#include <Windows.h>
#include <fstream>
#include "rwcore.h"
#include "rpworld.h"
#include "wrapper.h"
#include "IFPLoader.h"
#pragma comment(lib, "rwcore.lib")
#pragma comment(lib, "rpworld.lib")

extern std::ofstream ofs;
extern std::wofstream ofsW;

extern bool g_StaticAssocsSet;
extern bool g_HashKeyModified;
extern CAnimBlendStaticAssociation * pAnimStaticAssoc1;
extern CAnimBlendStaticAssociation * pAnimStaticAssoc2;

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
        // Change the hierarchy
        if ( 
                (OLD_GetUppercaseKey("run_stopR") == pGatewayAnimHierarchy->m_hashKey)
                ||
                (OLD_GetUppercaseKey("run_stop") == pGatewayAnimHierarchy->m_hashKey)
                ||
                (OLD_GetUppercaseKey("muscleidle_csaw") ==pGatewayAnimHierarchy->m_hashKey)
                ||
                (OLD_GetUppercaseKey("SEAT_down") == pGatewayAnimHierarchy->m_hashKey)
            )
        {
            ofs << "run_stop anim!! " << std::endl;

            if (g_StaticAssocsSet ) //&& !g_HashKeyModified)
            {
                //g_HashKeyModified = true;

                CAnimBlendStaticAssociation * pAnimStaticAssocToModify = GetAnimStaticAssocBy_GroupId_AnimId(animGroup, animID);

                ofs << "Calling ModifyAnimStaticAssocation now" << std::endl;

                ModifyAnimStaticAssocation(pClump, pAnimStaticAssocToModify);
            }

            pAnimAssoc = OLD_AddAnimation(pClump, animGroup, animID);
        }
        else
        {
            pAnimAssoc = OLD_AddAnimation(pClump, animGroup, animID);
        }
    }
    else
    {
        ofs << "ERROR: pGatewayAnimHierarchy is nullptr. Calling OLD_AddAnimation instead of OLD_AddAnimation_hier" << std::endl;

        pAnimAssoc = OLD_AddAnimation(pClump, animGroup, animID);
    }
    

    const char * AnimationName = GetNameFromHash(pAnimAssoc->m_pAnimBlendHierarchy->m_hashKey);
    if (AnimationName != nullptr)
    {
        ofs << std::endl << "AnimationName: " << AnimationName << std::endl;
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

/*
    static bool isRunStopAnimReplaced = false;

    if (!isRunStopAnimReplaced && isAnimationHierarchyLoaded("run_stopR"))
    {
    
        ofs << "Yes, animation is loaded." << std::endl;

        CAnimBlock * pPedAnimBlock = OLD_GetAnimationBlock("ped");

        ofs << "pPedAnimBlock->m_name: " << pPedAnimBlock->m_name << std::endl;



        //CAnimBlock * pMuscularAnimBlock = OLD_GetAnimationBlock("muscular");
        //CAnimBlendHierarchy * pAnimRunStopHierarchy = OLD_GetAnimation("muscleidle_csaw", pMuscularAnimBlock);

        const char * AnimationToReplaceWithName = "run_wuzi";
        CAnimBlendHierarchy * pAnimSeatDownHierarchy = OLD_GetAnimation(AnimationToReplaceWithName, pPedAnimBlock);
        
        const char * AnimationName = GetNameFromHash(pAnimSeatDownHierarchy->m_hashKey);
        if (AnimationName != nullptr)
        {
            ofs << "AnimationName: " << AnimationName << std::endl;
        }
        else
        {
            ofs << "GetNameFromHash: could not get animation name" << std::endl;
        }


        CAnimBlendHierarchy * pAnimRunStopHierarchy = OLD_GetAnimation("run_stopR", pPedAnimBlock);

        pAnimRunStopHierarchy->m_pSequences          = pAnimSeatDownHierarchy->m_pSequences;
        pAnimRunStopHierarchy->m_nSeqCount           = pAnimSeatDownHierarchy->m_nSeqCount;
        pAnimRunStopHierarchy->m_bRunningCompressed  = pAnimSeatDownHierarchy->m_bRunningCompressed;
        pAnimRunStopHierarchy->field_B               = pAnimSeatDownHierarchy->field_B;
        pAnimRunStopHierarchy->m_fTotalTime          = pAnimSeatDownHierarchy->m_fTotalTime;
        //pAnimRunStopHierarchy->field_14                = pAnimSeatDownHierarchy->field_14;


        isRunStopAnimReplaced = true;
    } 
    

    ofs << std::endl; 
*/
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
