                                                                                                                                                                                                                                                                                                                                                                                                     #include <Windows.h>
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


CAnimBlendAssociation * CreateCustomAnimation ( CAnimBlendAssociation * pAnimAssocToSyncWith, void * pClump, DWORD animGroup, DWORD animID );

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
                ||
                (OLD_GetUppercaseKey("KO_shot_front") == pGatewayAnimHierarchy->m_hashKey)

            )
        {
        
            static bool isAllowedToPlayCustomAnim = false;
            isAllowedToPlayCustomAnim = !isAllowedToPlayCustomAnim;
            if (isAllowedToPlayCustomAnim)
            {
              // ofs << "isAllowedToPlayCustomAnim: true" << std::endl;
            }
            if (g_bAllowToPlayCustomAnimation ) //&& isAllowedToPlayCustomAnim)
            {
               
                ofs << "About to play custom Animation now " << std::endl;

                pAnimAssoc = CreateCustomAnimation ( nullptr, pClump, animGroup, animID );
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



CAnimBlendAssociation * NEW_AddAnimationAndSync
(
    void * pClump,
    CAnimBlendAssociation * pAnimAssociation,
    DWORD animGroup,
    DWORD animID
)
{

    ofs << "NEW_AddAnimationAndSync: " << std::endl;
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
                ||
                (OLD_GetUppercaseKey("KO_shot_front") == pGatewayAnimHierarchy->m_hashKey)
                ||
                (OLD_GetUppercaseKey("CAR_sit") == pGatewayAnimHierarchy->m_hashKey)
                ||
                (OLD_GetUppercaseKey("facgum") == pGatewayAnimHierarchy->m_hashKey)
                ||
                (OLD_GetUppercaseKey("factalk") == pGatewayAnimHierarchy->m_hashKey)

            )
        {
            if (g_bAllowToPlayCustomAnimation ) 
            {
               
                ofs << "About to play custom Animation now " << std::endl;

                pAnimAssoc = CreateCustomAnimation ( pAnimAssociation, pClump, animGroup, animID );
            }
            else
            { 
                pAnimAssoc =  OLD_AddAnimationAndSync ( pClump, pAnimAssociation, animGroup,  animID );
            } 
        }
        else
        {
            pAnimAssoc =  OLD_AddAnimationAndSync ( pClump, pAnimAssociation, animGroup,  animID );
        } 
    }
    else
    {
        ofs << "ERROR: pGatewayAnimHierarchy is nullptr. Calling OLD_AddAnimation " << std::endl;

        pAnimAssoc = OLD_AddAnimationAndSync ( pClump, pAnimAssociation, animGroup,  animID );
    }



    const char * AnimationName = GetNameFromHash(pAnimAssoc->m_pAnimBlendHierarchy->m_hashKey);
    if (AnimationName != nullptr)
    {
        ofs << std::endl << "AnimationName: " << AnimationName << std::endl;

        printf( "NEW_AddAnimationAndSync: pClump: %p  |  AnimationName: %s  | AnimGroup, AnimID: %d, %d \n ",pClump, AnimationName, animGroup, animID);
    }
    else
    {
        ofs << "GetNameFromHash: could not get animation name" << std::endl;
        ofs << "HashKey ( Not Found ) : " << pAnimAssoc->m_pAnimBlendHierarchy->m_hashKey << std::endl;
    }

    ofs << std::endl;

    return pAnimAssoc;
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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         