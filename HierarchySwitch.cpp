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
extern hCAnimBlendStaticAssociation_Init OLD_CAnimBlendStaticAssociation_Init;
extern hCAnimBlendHierarchy_SetName OLD_CAnimBlendHierarchy_SetName;
extern hGetUppercaseKey OLD_GetUppercaseKey;

extern std::vector <CAnimBlendStaticAssociation *> g_StaticAssocations;

void WINAPI NEW_CAnimBlendStaticAssociation_Init
(
    void* pClump,
    CAnimBlendHierarchy* pAnimBlendHierarchy
)
{
    CAnimBlendStaticAssociation * pThis;

    __asm mov pThis, ecx;


    if (g_Clump == nullptr)
    {
        g_Clump = pClump;
        ofs << "NEW_CAnimBlendStaticAssociation_Init: Ok, g_Clump is now set" << std::endl;
    }


    __asm mov ecx, pThis;
    OLD_CAnimBlendStaticAssociation_Init(pClump, pAnimBlendHierarchy);

    //ofs << "NEW_CAnimBlendStaticAssociation_Init: " << std::endl << "Animation Name: " << GetNameFromHash(pAnimBlendHierarchy->m_hashKey) << std::endl;

    // Here pThis->m_nAnimGroup and pThis->m_nAnimID are -1 for every static assocation

    g_StaticAssocations.push_back(pThis);

}


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

void ModifyAnimStaticAssocation ( void * pClump, CAnimBlendStaticAssociation * pAnimStaticAssocToModify)
{

    CAnimBlendHierarchy* pAnimBlendHierarchy1 = pAnimStaticAssocToModify->m_pAnimBlendHier;
    
    // 241: run_wuzi from custom IFP
    // 242: SEAT_down from custom IFP
    // 127: facanger

    // GAT 3
    // 138: KO_shot_stom
    // 143: PHONE_in
    // 235: XPRESSscratch
    CAnimBlendHierarchy* pCustomAnimBlendHierarchy = &g_IFPs[0].AnimationHierarchies[138]; // 45 in car


    const char * AnimationName = GetNameFromHash(pCustomAnimBlendHierarchy->m_hashKey);
    if (AnimationName != nullptr)
    {
        ofs << std::endl << "(pCustomAnimBlendHierarchy)AnimationName: " << AnimationName << std::endl;
    }
    else
    {
        ofs << "GetNameFromHash: could not get animation name" << std::endl;
        ofs << "HashKey ( Not Found ) : " << pCustomAnimBlendHierarchy->m_hashKey << std::endl;
    }

    ofs << std::endl;

    pCustomAnimBlendHierarchy->m_hashKey = pAnimBlendHierarchy1->m_hashKey;
    pCustomAnimBlendHierarchy->m_nAnimBlockId = pAnimBlendHierarchy1->m_nAnimBlockId;

    __asm 
    {
        push ecx;
        mov ecx, pAnimStaticAssocToModify;
    };

    OLD_CAnimBlendStaticAssociation_Init(pClump, pCustomAnimBlendHierarchy);
    

    __asm pop ecx;

    printf(
        "pClump: %p\n \n\n", pClump);



    ofs << "modified pAnimStaticAssoc1 members and initialized pAnimStaticAssoc1 with pAnimBlendHierarchy2" << std::endl;
}


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
    for (size_t i = 0; i < g_StaticAssocations.size(); i++)
    {
        CAnimBlendStaticAssociation * pThis = g_StaticAssocations[i];

        if ((pThis->m_nAnimGroup == GroupID) && (pThis->m_nAnimID == AnimID))
        {
            return pThis->m_pAnimBlendHier;
        }
    }
    return nullptr;
}

CAnimBlendStaticAssociation * GetAnimStaticAssocBy_GroupId_AnimId(DWORD GroupID, DWORD AnimID)
{
    for (size_t i = 0; i < g_StaticAssocations.size(); i++)
    {
        CAnimBlendStaticAssociation * pThis = g_StaticAssocations[i];

        if ((pThis->m_nAnimGroup == GroupID) && (pThis->m_nAnimID == AnimID))
        {
            return pThis;
        }
    }
    return nullptr;
}