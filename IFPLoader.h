#pragma once
#ifndef IFPLOADER_H
#define IFPLOADER_H
#include "wrapper.h"
#include "FileLoader.h"
#include <vector>

#define IFP_TOTAL_SEQUENCES 32 // 32 because there are 32 bones in a ped model 


struct IFPHeaderV1
{
    int32_t   OffsetEOF;
    char      InternalFileName[24];
    int32_t   TotalAnimations;
};

struct IFPHeaderV2
{
    int32_t   OffsetEOF;
    char      InternalFileName[24];
    int32_t   TotalAnimations;
};

struct IFP : FileLoader
{   
    bool isVersion1;
    IFPHeaderV1 HeaderV1;
    IFPHeaderV2 HeaderV2;
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

enum BoneType 
{
    NORMAL      = 0, // Normal or Root, both are same
    PELVIS      = 1,
    SPINE       = 2,
    SPINE1      = 3,
    NECK        = 4,
    HEAD        = 5,
    JAW         = 8,
    L_BROW      = 6,
    R_BROW      = 7,
    L_CLAVICLE  = 31,
    L_UPPER_ARM = 32,
    L_FORE_ARM  = 33,
    L_HAND      = 34,
    L_FINGER    = 35,
    L_FINGER_01 = 36,
    R_CLAVICLE  = 21,
    R_UPPER_ARM = 22,
    R_FORE_ARM  = 23,
    R_HAND      = 24,
    R_FINGER    = 25,
    R_FINGER_01 = 26,
    L_BREAST    = 302,
    R_BREAST    = 301,
    BELLY       = 201,
    L_THIGH     = 41,
    L_CALF      = 42,
    L_FOOT      = 43,
    L_TOE_0     = 44,
    R_THIGH     = 51,
    R_CALF      = 52,
    R_FOOT      = 53,
    R_TOE_0     = 54    
};

typedef void *(__cdecl* hCMemoryMgr_Malloc)
(
    size_t TotalBytesToAllocate
);


void LoadIFPFile(const char * FilePath);
void ReadIFPVersion1(IFP * IFPElement);
void ReadIFPVersion2(IFP * IFPElement, bool anp3);

void insertAnimDummySequence(bool anp3, CAnimBlendHierarchy * pAnimHierarchy, size_t SequenceIndex);
int32_t getBoneIDFromName(std::string const& BoneName);
std::string getCorrectBoneNameFromName(std::string const& BoneName);
std::string getCorrectBoneNameFromID(int32_t & BoneID);
size_t getCorrectBoneIndexFromID(int32_t & BoneID);

void CAnimBlendHierarchy_Constructor(CAnimBlendHierarchy * pAnimHierarchy);
void CAnimBlendSequence_Constructor(CAnimBlendSequence * pSequence);




// --------------------------------------  For Hierarchy ----------------------------------------------------

void Call_CAnimBlendHierarchy_RemoveQuaternionFlips(CAnimBlendHierarchy * pAnimHierarchy);

void Call_CAnimBlendHierarchy_CalcTotalTime(CAnimBlendHierarchy * pAnimHierarchy);

#endif