#pragma once
#ifndef IFPLOADER_H
#define IFPLOADER_H
#include "wrapper.h"
#include "FileLoader.h"
#include <vector>

#define IFP_TOTAL_SEQUENCES 32 // 32 because there are 32 bones in a ped model 


struct IFPHeader
{
    // 4 + 4 + 24 + 4 = 36 bytes

    char      Version[4];
    int32_t   OffsetEOF;
    char      InternalFileName[24];
    int32_t   TotalAnimations;
};

struct IFP : FileLoader
{
    IFPHeader Header;
    std::vector <CAnimBlendHierarchy> AnimationHierarchies;
    std::vector <CAnimBlendSequence> AnimationSequences;
    unsigned char * KeyFramesArray;
};

struct Animation
{
    // 24 + 4 + 4 + 4 = 36 bytes

    char      Name[24];
    int32_t   TotalObjects;
    int32_t   FrameSize;
    int32_t   isCompressed; // The value is always 1
};


struct Object
{
    // 24 + 4 + 4 + 4 = 36 bytes

    char      Name[24];
    int32_t   FrameType;
    int32_t   TotalFrames;
    int32_t   BoneID;
};

struct IFP2_ChildFrame
{
    int16_t x, y, z, w, time;
};


typedef void *(__cdecl* hCMemoryMgr_Malloc)
(
    size_t TotalBytesToAllocate
);


void LoadIFPFile(const char * FilePath);
void ReadIfpAnp23(IFP * IFPElement, bool anp3);

void CAnimBlendHierarchy_Constructor(CAnimBlendHierarchy * pAnimHierarchy);
void CAnimBlendSequence_Constructor(CAnimBlendSequence * pSequence);




// --------------------------------------  For Hierarchy ----------------------------------------------------

void Call_CAnimBlendHierarchy_RemoveQuaternionFlips(CAnimBlendHierarchy * pAnimHierarchy);

void Call_CAnimBlendHierarchy_CalcTotalTime(CAnimBlendHierarchy * pAnimHierarchy);

#endif