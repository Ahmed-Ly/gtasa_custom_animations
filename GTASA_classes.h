#ifndef GTASA_CLASSES_H
#define GTASA_CLASSES_H

#include <Windows.h>
#include <stdint.h>

class CAnimBlock
{
public:
    const char* GetName();
    bool IsLoaded();
    uint16_t GetReferenceCount();
    int GetIdOffset();
    size_t GetAnimationCount();
    DWORD GetFirstAssocGroup();

    void SetAnimationCount(size_t numAnims);
    void SetName(const char* name);
    void SetIdOffset(int idOffset);
    void SetFirstAssocGroup(DWORD assocGroup);
    void SetLoaded(bool loaded);
    void SetReferenceCount(uint16_t refs);

    char m_name[16];
    bool m_loaded;
    char pad1;
    uint16_t m_refs;
    int m_idOffset;
    size_t m_numAnims;
    DWORD m_assocGroup;
};

struct CAnimBlendSequence
{
    unsigned int m_nHash;
    unsigned short m_nFlags;
    unsigned short m_nFrameCount;
    void *m_pFrames;
};
/*
class CAnimBlendSequence {
public:
    unsigned int m_nHash;
    unsigned short m_nFlags;
    unsigned short m_nFrameCount;
    void *m_pFrames; 

    //funcs
    //CAnimBlendSequence();
    //~CAnimBlendSequence();
    
    void CompressKeyframes(unsigned char* arg1);
    int GetDataSize(bool arg1);
    bool MoveMemorY();
    int RemoveQuaternionFlips();
    void RemoveUncompressedData(unsigned char* arg1);
    void SetBoneTag(int hash);
    void SetName(char const* string);
    void SetNumFrames(int count, bool arg2, bool arg3, unsigned char* arg4);
    void Uncompress(unsigned char* arg1);
    

};
*/

class CAnimBlendHierarchy;

class CAnimBlendStaticAssociation {
public:
    unsigned short m_nNumBlendNodes;      
    short m_nAnimID;                      
    short m_nAnimGroup;                   
    short m_nFlags;                  
    int* m_pAnimBlendNodesSequenceArray;   
    CAnimBlendHierarchy* m_pAnimBlendHier; 

                                          

    void AllocateSequenceArray(int size);
    CAnimBlendStaticAssociation();
    void Init(void* pClump, CAnimBlendHierarchy* pAnimBlendHier);
    virtual ~CAnimBlendStaticAssociation() = 0;
};


class CAnimBlendHierarchy {
public:
    unsigned int m_hashKey;
    CAnimBlendSequence *m_pSequences;
    unsigned short m_nSeqCount; // pAnimHierarchy + 8
    bool m_bRunningCompressed;
    char field_B;
    int m_nAnimBlockId;
    float m_fTotalTime;
    int field_14;

    //funcs

    void* AllocSequenceBlock(bool arg1);
    //CAnimBlendHierarchy();
    void CalcTotalTime();
    void CalcTotalTimeCompressed();
    void RemoveAnimSequences();
    void RemoveQuaternionFlips();
    void RemoveUncompressedData();
    void SetName(char const* string);
    void Shutdown();
    void Uncompress();

};

//4 + 4 + 4 + 4      +  2 + 2     + 4 + 4 + 4 + 4 + 4 + 4 + 4   + 2 + 2 + 4 + 4
class CAnimBlendAssociation {
protected:
    void *vtable;
public:
    void *m_pPrev;
    void *m_pAnimBlendClumpData;
    unsigned short m_nNumBlendNodes;
    short m_nAnimGroup;
    void *m_pAnimBlendNodeArray;
    CAnimBlendHierarchy *m_pAnimBlendHierarchy;
    float m_fBlendAmount;
    float m_fBlendDelta;
    float m_fCurrentTime;
    float m_fSpeed;
    float m_fTimeStep;
    short m_nwAnimID;
    unsigned short m_nFlags;
    unsigned int m_nCallbackType;
    void *m_pCallbackFunc;
    void *m_pCallbackData;

    //funcs
    void* AllocateAnimBlendNodeArray(int numBlendNodes);
    CAnimBlendAssociation(void *& arg1_CAnimBlendStaticAssociation);
    CAnimBlendAssociation(void* pClump, CAnimBlendHierarchy* pAnimBlendHier);
    void* GetNode(int index);
    void Init(void* & arg1_CAnimBlendStaticAssociation);
    void Init(void* pClump, CAnimBlendHierarchy* pAnimBlendHier);
    void ReferenceAnimBlock();
    void SetBlend(float fBlendAmount, float fBlendDelta);
    void SetCurrentTime(float currentTime);
    void SetDeleteCallback(void* func, void* data);
    void SetFinishCallback(void* func, void* data);
    void Start(float currentTime);
    void SyncAnimation(CAnimBlendAssociation* arg1);
    bool UpdateBlend(float BlendDeltaMult);
    void UpdateTime();
    //~CAnimBlendAssociation();
    //virtual ~CAnimBlendAssociation();
};

#endif