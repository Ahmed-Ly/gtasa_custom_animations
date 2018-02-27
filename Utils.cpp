#include "wrapper.h"
#include <fstream>
#include "IFPLoader.h"

extern std::ofstream ofs;
extern std::wofstream ofsW;

extern std::vector <IFP> g_IFPs;

extern DWORD g_CurrentHierarchyAnimationHash;
extern char * AnimationToReplaceWithName;
extern std::map<DWORD, std::string> g_mapOfAnimHierarchyHashes;
extern void * g_Clump;
extern hCAnimBlendStaticAssociation_Constructor OLD_CAnimBlendStaticAssociation_Constructor;
extern hCAnimBlendStaticAssociation_Init OLD_CAnimBlendStaticAssociation_Init;
extern hCAnimBlendHierarchy_SetName OLD_CAnimBlendHierarchy_SetName;
extern hGetUppercaseKey OLD_GetUppercaseKey;



const char * GetNameFromHash ( DWORD Hash )
{
    // Check if the element exists
    if (g_mapOfAnimHierarchyHashes.find(Hash) != g_mapOfAnimHierarchyHashes.end())
    {
        return g_mapOfAnimHierarchyHashes[Hash].c_str();
    }
    return nullptr;
}

/*
unsigned int WINAPI NEW_CAnimBlendHierarchy_SetName ( char * string )
{
    CAnimBlendHierarchy* pThis;
    __asm mov pThis, ecx;

    g_mapOfAnimHierarchyHashes[pThis->m_hashKey] = std::string(string);

    ofs << "NEW_CAnimBlendHierarchy_SetName: " << GetNameFromHash(pThis->m_hashKey) << std::endl;

    __asm mov ecx, pThis;
    return OLD_CAnimBlendHierarchy_SetName ( string );
}
*/


unsigned int __cdecl NEW_GetUppercaseKey
(
    char const * str
)
{
    unsigned int HashKey = OLD_GetUppercaseKey ( str );

    g_mapOfAnimHierarchyHashes[HashKey] = std::string(str);

    return HashKey;
}

    // 241: run_wuzi from custom IFP
    // 242: SEAT_down from custom IFP
    // 127: facanger

    // GAT 3
    // 138: KO_shot_stom
    // 143: PHONE_in
    // 235: XPRESSscratch
    // 120: idle_stance

    // GTA VC 
    // 145: KO_shot_stom  NOPE
    // 151: Phone_in      GOOD
    // 233: XpressScratch GOOD
    // 127: IDLE_STANCE   GOOD
    // 177: run_stopR 
    //  14: bomber
    //   1: ARREST_GUN

void printHierarchies(void)
{
    CAnimBlendHierarchy* FirstAnimHierarchy = (CAnimBlendHierarchy*)ARRAY_CAnimManager_Animations;

    for (int i = 0; i < GetNumAnimations(); i++)
    {
        CAnimBlendHierarchy * AnimHierarchy = (CAnimBlendHierarchy*)((BYTE*)FirstAnimHierarchy + sizeof(CAnimBlendHierarchy) * i);

        const char * AnimationName = GetNameFromHash(AnimHierarchy->m_hashKey);
        if (AnimationName != nullptr)
        {
            ofs << "Index: " << i << "  |   AnimationName: " << AnimationName << std::endl;
        }
        else
        {
            ofs << "GetNameFromHash: could not get animation name" << std::endl;
        }
    }

}

void printAnimBlocks(void)
{
    CAnimBlock* FirstAnimBlock = (CAnimBlock*)ARRAY_CAnimManager_AnimBlocks;

    ofs << std::endl;

    for (int i = 0; i < GetNumAnimBlocks(); i++)
    {
        CAnimBlock * AnimBlock = (CAnimBlock*)((BYTE*)FirstAnimBlock + sizeof(CAnimBlock) * i);

    
        ofs << "Animation Block Name: " << AnimBlock->m_name << "  |  AssocGroup: " << AnimBlock->m_assocGroup << std::endl;
    }

    ofs << std::endl;
}


bool isAnimationHierarchyLoaded(const char * AnimationName)
{
    CAnimBlendHierarchy* FirstAnimHierarchy = (CAnimBlendHierarchy*)ARRAY_CAnimManager_Animations;

    for (int i = 0; i < GetNumAnimations(); i++)
    {
        CAnimBlendHierarchy * AnimHierarchy = (CAnimBlendHierarchy*)((BYTE*)FirstAnimHierarchy + sizeof(CAnimBlendHierarchy) * i);

        if ( OLD_GetUppercaseKey (AnimationName) == AnimHierarchy->m_hashKey )
        {
            return true;
        }
    }

    return false;
}


CAnimBlendHierarchy * GetAnimHierachyBy_GroupId_AnimId(DWORD GroupID, DWORD AnimID)
{

    DWORD * pAssocGroup = (DWORD *) (*(DWORD*)0x00B4EA34) + 5 * GroupID;

    CAnimBlendStaticAssociation * pStaticAssoc = (CAnimBlendStaticAssociation *)( pAssocGroup[1] + 20 * (AnimID - pAssocGroup[3]) );

    return pStaticAssoc->m_pAnimBlendHier; 
}

CAnimBlendStaticAssociation * GetAnimStaticAssocBy_GroupId_AnimId(DWORD GroupID, DWORD AnimID)
{
    DWORD * pAssocGroup = (DWORD *) (*(DWORD*)0x00B4EA34) + 5 * GroupID;

    CAnimBlendStaticAssociation * pStaticAssoc = (CAnimBlendStaticAssociation *)( pAssocGroup[1] + 20 * (AnimID - pAssocGroup[3]) );

    return pStaticAssoc;
}