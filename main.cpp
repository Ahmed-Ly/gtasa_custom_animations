#include <windows.h>
#include <fstream>
#include <stdio.h>
#include <ctime>
#include "detours.h"
#include "wrapper.h"
#include "IFPLoader.h"

#pragma comment(lib, "detours.lib")

void PrepareOutputStreams();
inline bool isKeyPressed(unsigned int keyCode);
 

extern std::ofstream ofs;
extern std::wofstream ofsW;

std::map<DWORD, std::string> g_mapOfAnimHierarchyHashes;

void * g_Clump = nullptr;
bool g_PlayCustomAnimations = false;

std::vector <CAnimBlendStaticAssociation *> g_StaticAssocations;

CAnimBlendStaticAssociation * pAnimStaticAssoc1 = nullptr;
CAnimBlendStaticAssociation * pAnimStaticAssoc2 = nullptr;


hGetAnimationBlock        OLD_GetAnimationBlock = (hGetAnimationBlock)0x4d3940;
hGetAnimation             OLD_GetAnimation = (hGetAnimation)0x4d42f0;
hGetAnimBlockName         OLD_GetAnimBlockName                     =  (hGetAnimBlockName)              0x4d3a30;
hAddAnimAssocDefinition   OLD_AddAnimAssocDefinition               =  (hAddAnimAssocDefinition)        0x4D3BA0;
hRegisterAnimBlock        OLD_RegisterAnimBlock                    =  (hRegisterAnimBlock)             0x4D3E50;
hCreateAssociations_Clump OLD_CreateAssociations_Clump             =  (hCreateAssociations_Clump)      0x4CE6E0;
hCreateAssociations       OLD_CreateAssociations                   =  (hCreateAssociations)            0x4CE3B0;

hRpAnimBlendClumpGetAssociation OLD_RpAnimBlendClumpGetAssociation = (hRpAnimBlendClumpGetAssociation) 0x4d6870;
hRpAnimBlendClumpGetAssociationAnimId OLD_RpAnimBlendClumpGetAssociationAnimId = (hRpAnimBlendClumpGetAssociationAnimId) 0x4D68B0;

hGetAnimAssociationAnimId OLD_GetAnimAssociationAnimId = (hGetAnimAssociationAnimId) 0x4d3a60;
hCreateAnimAssociation OLD_CreateAnimAssociation = (hCreateAnimAssociation)0x4d3a40;
hAddAnimation OLD_AddAnimation = (hAddAnimation)0x4d3aa0;  
hGetAnimGroupName OLD_GetAnimGroupName = (hGetAnimGroupName)0x4d3a20;

hCAnimBlendStaticAssociation_Init OLD_CAnimBlendStaticAssociation_Init = (hCAnimBlendStaticAssociation_Init)0x004CEC20;
hCAnimBlendAssociation_Init_reference OLD_CAnimBlendAssociation_Init_reference = (hCAnimBlendAssociation_Init_reference)0x4CEEC0;
hCAnimBlendAssociation_Init OLD_CAnimBlendAssociation_Init = (hCAnimBlendAssociation_Init)0x4CED50;
hCAnimBlendHierarchy_SetName OLD_CAnimBlendHierarchy_SetName = (hCAnimBlendHierarchy_SetName)0x4CF2D0;
hGetUppercaseKey OLD_GetUppercaseKey = (hGetUppercaseKey)0x53CF30; 

hLoadAnimFile_stream OLD_LoadAnimFile_stream = (hLoadAnimFile_stream)0x4d47f0;
hAddAnimation_hier OLD_AddAnimation_hier = (hAddAnimation_hier)0x4d4330;

hLoadPedAnimIFPFile OLD_LoadPedAnimIFPFile = (hLoadPedAnimIFPFile)0x004D5620;

void HookFunctions()
{
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach ( &(PVOID&) OLD_AddAnimAssocDefinition, NEW_AddAnimAssocDefinition );
    DetourAttach ( &(PVOID&) OLD_RegisterAnimBlock     , NEW_RegisterAnimBlock );


    //DetourAttach ( &(PVOID&) OLD_CreateAssociations_Clump, NEW_CreateAssociations_Clump );

    //DetourAttach ( &(PVOID&) OLD_CreateAssociations        , NEW_CreateAssociations );
    
    //DetourAttach(&(PVOID&)OLD_CreateAnimAssociation, NEW_CreateAnimAssociation);
    DetourAttach(&(PVOID&)OLD_AddAnimation, NEW_AddAnimation);
    
    DetourAttach(&(PVOID&)OLD_CAnimBlendStaticAssociation_Init, NEW_CAnimBlendStaticAssociation_Init); 
    DetourAttach(&(PVOID&)OLD_GetUppercaseKey, NEW_GetUppercaseKey);
    
    //DetourAttach(&(PVOID&)OLD_LoadAnimFile_stream, NEW_LoadAnimFile_stream);

    //DetourAttach(&(PVOID&)OLD_LoadPedAnimIFPFile, NEW_LoadPedAnimIFPFile);
    

    
    DetourTransactionCommit(); 
}

DWORD WINAPI Main_thread(LPVOID lpParam)
{

    if (AllocConsole())
    {
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }


    PrepareOutputStreams();

    /*
    ofs << "GetNumAnimations = " << GetNumAnimations() << std::endl << std::endl;

    ofs << "GetNumAnimBlocks = " << GetNumAnimBlocks() << std::endl << std::endl;
    */


    HookFunctions();

    
    printf("calling LoadIFPFile now\n");

    LoadIFPFile("C:\\Users\\danish\\Desktop\\gtaTxdDff\\ped.ifp"); // ANP3
  // LoadIFPFile ("C:\\Program Files (x86)\\Rockstar Games\\GTA3\\anim\\ped.ifp"); // ANPK
    

    ofs << "finished calling LoadIFPFile " << std::endl;

    printf("finished calling LoadIFPFile\n");
    

    clock_t OnePressTMR = clock();

    while (1)
    {
        if (clock() - OnePressTMR > 400)
        {
            if (isKeyPressed('6') &&  (clock() - OnePressTMR > 1000))
            {
                OnePressTMR = clock();
                printf("calling LoadIFPFile now\n");

                LoadIFPFile("C:\\Users\\danish\\Desktop\\gtaTxdDff\\ped.ifp");

                ofs << "finished calling LoadIFPFile " << std::endl;

                printf("finished calling LoadIFPFile\n");

                clock_t OnePressTMR = clock();
            }

            if (isKeyPressed('0'))
            {
                OnePressTMR = clock();

                printAnimBlocks();

                // Print the hierarchies information to file
                printHierarchies();
            }

            

            if (isKeyPressed('5'))
            {
                OnePressTMR = clock();
                ofs << std::endl << "Pressed Key '5'" << std::endl << std::endl;

               g_PlayCustomAnimations = true;
         
            }
        }
    } 

     // delete pBlock;

    return S_OK;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        CreateThread(0, NULL, &Main_thread, 0, 0, NULL);
        
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        break;
    }
    case DLL_THREAD_ATTACH:
    {
        break;
    }
    case DLL_THREAD_DETACH:
    {
        break;
    }
    }

    /* Return TRUE on success, FALSE on failure */
    return TRUE;
}


inline bool isKeyPressed(unsigned int keyCode) {
    return (GetKeyState(keyCode) & 0x8000) != 0;
}
