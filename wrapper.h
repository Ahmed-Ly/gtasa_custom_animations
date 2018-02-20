#include <Windows.h>
#include "GTASA_classes.h"
#include <vector>
#include <map>
#include <string>
#include <iterator>

VOID SHOW_DWORD_HEX_MsgBox(DWORD output);
VOID SHOW_OUTPUT_BOXW(LPCWSTR output);
VOID SHOW_OUTPUT_BOX(LPCSTR output);

// Animations = CAnimBlendHierarchy
#define ARRAY_CAnimManager_AnimBlocks                       0xb5d4a0
#define ARRAY_CAnimManager_Animations                       0xB4EA40
#define VAR_CAnimManager_NumAnimations                      0xb4ea2c
#define VAR_CAnimManager_NumAnimBlocks                      0xb4ea30

int GetNumAnimations ( void );
int GetNumAnimBlocks ( void );


typedef char * ( __cdecl* hAddAnimAssocDefinition )
(
    const char *   BlockName, 
    const char *   AnimName,
    unsigned long  AnimationGroup,
    unsigned long  AnimationID,
    int            Descriptor
);


typedef int ( __cdecl* hRegisterAnimBlock )
(
    const char * szName
);



typedef void(WINAPI* hCreateAssociations_Clump)
(
    char const* szIfpFile,
    void * pClump,
    char** pszAnimations,
    int NumAnimations
);

typedef void(WINAPI* hCreateAssociations)
(
    char const* szIfpFile,
    char const* arg2,
    char const* arg3,
    int arg4
);


typedef CAnimBlendAssociation * (__cdecl* hRpAnimBlendClumpGetAssociation)
(
    void * pClump,
    const char * szAnimName
);

typedef CAnimBlendAssociation * (__cdecl* hRpAnimBlendClumpGetAssociationAnimId)
(
    //int clump,
    void * clump,
    unsigned int animId
);



typedef CAnimBlendAssociation * (__cdecl* hGetAnimAssociationAnimId)
(
    DWORD animGroup,
    DWORD animID
);


typedef const char * (__cdecl* hGetAnimGroupName)
( 
    DWORD groupID
);


typedef CAnimBlendAssociation * (__cdecl* hCreateAnimAssociation)
(
    DWORD animGroup,
    DWORD animID
);

typedef CAnimBlendAssociation * (__cdecl* hAddAnimation)
(
    void * pClump,
    DWORD animGroup,
    DWORD animID
);

typedef void (WINAPI* hCAnimBlendStaticAssociation_Init)
(
    void* pClump,
    CAnimBlendHierarchy* pAnimBlendHierarchy
);

typedef signed int (__thiscall* hCAnimBlendAssociation_Init_reference)
(
    CAnimBlendStaticAssociation& ReferenceStaticAssoc
);


typedef int(__thiscall* hCAnimBlendAssociation_Init)
(
    void* pClump, 
    CAnimBlendHierarchy* pAnimBlendHier
);

typedef unsigned int(__thiscall* hCAnimBlendHierarchy_SetName)
(
    void * pThis,
    char * string
);

typedef unsigned int(__cdecl* hGetUppercaseKey)
(
    char const * str
);

typedef int(__cdecl * hLoadAnimFile_stream)
(
    int pStream,
    bool b1,
    const char * sz1
);

typedef const char *(__cdecl * hGetAnimBlockName)
(
    DWORD groupID
);

typedef CAnimBlock *(__cdecl * hGetAnimationBlock)
(
    const char * szName
);

typedef CAnimBlendHierarchy *(__cdecl * hGetAnimation)
(
    const char * szName,
    CAnimBlock * pBlock
);

typedef CAnimBlendAssociation *(__cdecl * hAddAnimation_hier)
(
    void * pClump, 
    CAnimBlendHierarchy *, 
    int ID
);


typedef WORD *(__thiscall * hCAnimBlendSequence_Constructor)(void);

typedef void(__thiscall * hCAnimBlendSequence_Deconstructor)(void);

typedef int(__thiscall * hCAnimBlendHierarchy_Constructor)(void);
//typedef int(WINAPI * hCAnimBlendHierarchy_Constructor)(void);

typedef unsigned int(__thiscall * hCAnimBlendSequence_SetName)
(
    void * pThis,
    char const* string
);

typedef int(__thiscall * hCAnimBlendSequence_SetBoneTag)
(
    void * pThis,
    int hash
);

typedef void *(__thiscall * hCAnimBlendSequence_SetNumFrames)
(
    void * pThis,
    size_t frameCount,
    bool isRoot, 
    bool compressed, 
    unsigned char* st
);


typedef int(__thiscall *  hCAnimBlendHierarchy_RemoveQuaternionFlips) ( void );

typedef int(__thiscall *  hCAnimBlendHierarchy_CalcTotalTime) (void);

typedef int(__cdecl *  hCreateAnimAssocGroups)
(
    void
);

typedef int(__cdecl *  hLoadPedAnimIFPFile)
(
    void
);

typedef int * (__cdecl *  hRwStreamOpen)
(int a1, int a2, DWORD *a3);

typedef BOOL(__cdecl *  hRwStreamClose)
(DWORD *a1, DWORD *a2);

typedef int(__cdecl *  hGetFirstAssocGroup)
(char *a1);



char * __cdecl NEW_AddAnimAssocDefinition
(
    const char *   BlockName,
    const char *   AnimName,
    unsigned long  AnimationGroup,
    unsigned long  AnimationID,
    int            Descriptor
);

int __cdecl NEW_RegisterAnimBlock
(
    const char * szName
);


void WINAPI NEW_CreateAssociations_Clump
(
    char const* szIfpFile,
    void * pClump, 
    char** pszAnimations,
    int NumAnimations
);

void WINAPI NEW_CreateAssociations
(
    char const* szIfpFile,
    char const* arg2, 
    char const* arg3, 
    int arg4
);

CAnimBlendAssociation * NEW_CreateAnimAssociation
(
    DWORD animGroup,
    DWORD animID
);

CAnimBlendAssociation * NEW_AddAnimation
(
    void * pClump,
    DWORD animGroup,
    DWORD animID
);

void WINAPI NEW_CAnimBlendStaticAssociation_Init
(
    void* pClump, 
    CAnimBlendHierarchy* pAnimBlendHierarchy
);

unsigned int WINAPI NEW_CAnimBlendHierarchy_SetName
(
    char * string
);

unsigned int __cdecl NEW_GetUppercaseKey
(
    char const * str
);


int __cdecl NEW_LoadAnimFile_stream
(
    int pStream,
    bool b1, 
    const char * sz1
);

int NEW_LoadPedAnimIFPFile
(
    void
);

const char * GetNameFromHash(DWORD Hash);
int GetNumAnimations(void);
void printHierarchies(void);
void printAnimBlocks(void);
bool isAnimationHierarchyLoaded(const char * AnimationName);
CAnimBlendHierarchy * GetAnimHierachyBy_GroupId_AnimId (DWORD GroupID, DWORD AnimID);
CAnimBlendStaticAssociation * GetAnimStaticAssocBy_GroupId_AnimId(DWORD GroupID, DWORD AnimID);
void ModifyAnimStaticAssocation(void * pClump, CAnimBlendStaticAssociation * pAnimStaticAssocToModify);
