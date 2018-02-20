#include "IFPLoader.h"
#include <algorithm>

extern std::ofstream ofs;
extern std::wofstream ofsW;

std::vector <IFP> g_IFPs;

extern hCAnimBlendHierarchy_SetName OLD_CAnimBlendHierarchy_SetName;

hCMemoryMgr_Malloc               OLD_CMemoryMgr_Malloc = (hCMemoryMgr_Malloc)0x0072F420;
hCAnimBlendSequence_SetName      OLD_CAnimBlendSequence_SetName = (hCAnimBlendSequence_SetName)0x4D0C50;
hCAnimBlendSequence_SetBoneTag   OLD_CAnimBlendSequence_SetBoneTag = (hCAnimBlendSequence_SetBoneTag)0x4D0C70;
hCAnimBlendSequence_SetNumFrames OLD_CAnimBlendSequence_SetNumFrames = (hCAnimBlendSequence_SetNumFrames)0x4D0CD0;

hCAnimBlendHierarchy_RemoveQuaternionFlips OLD_CAnimBlendHierarchy_RemoveQuaternionFlips = (hCAnimBlendHierarchy_RemoveQuaternionFlips)0x4CF4E0;
hCAnimBlendHierarchy_CalcTotalTime         OLD_CAnimBlendHierarchy_CalcTotalTime = (hCAnimBlendHierarchy_CalcTotalTime)0x4CF2F0;


extern DWORD BoneIds[];
extern char BoneNames[][24];


void LoadIFPFile(const char * FilePath)
{
    IFP IFPElement;

    IFPElement.createLoader(FilePath);

    if (IFPElement.loadFile())
    {
        printf("IfpLoader: File loaded. Parsing it now.\n");

        char Version[4];
       
        IFPElement.readBytes ( Version, sizeof(Version) );

        if (strncmp(Version, "ANP2", sizeof(Version)) == 0 || strncmp(Version, "ANP3", sizeof(Version)) == 0)
        {
            IFPElement.isVersion1 = false;

            bool anp3 = strncmp(Version, "ANP3", sizeof(Version)) == 0;

            ReadIFPVersion2 ( &IFPElement, anp3 );
        }
        else
        {
            IFPElement.isVersion1 = true;

            ReadIFPVersion1 ( &IFPElement );
        }

        // We are unloading the data because we don't need to read it anymore. 
        // This function does not unload IFP, to unload ifp call unloadIFP function
        IFPElement.unloadFile();
    }

    g_IFPs.push_back(IFPElement);

    printf("Exiting LoadIFPFile function\n");
    ofs << "Exiting LoadIFPFile function" << std::endl;
}



void ReadIFPVersion2(IFP * IFPElement, bool anp3)
{
    IFPElement->readBuffer < IFPHeaderV2 > ( &IFPElement->HeaderV2 );

    IFPElement->AnimationHierarchies.resize(IFPElement->HeaderV2.TotalAnimations);


    for (size_t i = 0; i < IFPElement->AnimationHierarchies.size(); i++)
    {
        CAnimBlendHierarchy * pAnimHierarchy = &IFPElement->AnimationHierarchies[i];

        CAnimBlendHierarchy_Constructor(pAnimHierarchy);

        Animation AnimationNode;

        IFPElement->readCString((char *)&AnimationNode.Name, sizeof(Animation::Name));
        IFPElement->readBuffer < int32_t >(&AnimationNode.TotalObjects);

        ofs << "Animation Name: " << AnimationNode.Name << "  |  Index: " << i << std::endl;

        printf("Animation Name: %s    |  Index: %d \n", AnimationNode.Name, i);

        unsigned char * pKeyFrames = nullptr;
        if (anp3)
        {
            IFPElement->readBuffer < int32_t >(&AnimationNode.FrameSize);
            IFPElement->readBuffer < int32_t >(&AnimationNode.isCompressed);

            pAnimHierarchy->m_bRunningCompressed = AnimationNode.isCompressed & 1;

            pKeyFrames = (unsigned char*)OLD_CMemoryMgr_Malloc(AnimationNode.FrameSize);
        }

        OLD_CAnimBlendHierarchy_SetName(pAnimHierarchy, (char*)&AnimationNode.Name);

        pAnimHierarchy->m_nSeqCount = AnimationNode.TotalObjects;
        pAnimHierarchy->m_nAnimBlockId = 0;
        pAnimHierarchy->field_B = 0;

        char * pNewSequencesMemory = (char *)operator new(12 * IFP_TOTAL_SEQUENCES + 4);//  Allocate memory for sequences ( 12 * seq_count + 4 )

        *(DWORD *)pNewSequencesMemory = IFP_TOTAL_SEQUENCES;

        pAnimHierarchy->m_pSequences = (CAnimBlendSequence *)(pNewSequencesMemory + 4);
        
        std::map < std::string, CAnimBlendSequence > MapOfSequences;

        bool bFirstSeq = true;
        for (size_t SequenceIndex = 0; SequenceIndex  < pAnimHierarchy->m_nSeqCount; SequenceIndex++)
        {
            Object ObjectNode;

            IFPElement->readBuffer < Object >(&ObjectNode);
            
            std::string BoneName (ObjectNode.Name);
            std::transform(BoneName.begin(), BoneName.end(), BoneName.begin(), ::tolower); // convert the bone name to lowercase

            BoneName.erase(std::remove(BoneName.begin(), BoneName.end(), ' '), BoneName.end()); // remove white spaces

            std::string CorrectBoneName;
            
            bool bSkipSequence = false;
            if (ObjectNode.BoneID == -1)
            {
                ObjectNode.BoneID = getBoneIDFromName(BoneName);
                if (ObjectNode.BoneID == -1)
                {
                    bSkipSequence = true;
                }

                CorrectBoneName   = getCorrectBoneNameFromName(BoneName);
            }
            else
            {
                CorrectBoneName = getCorrectBoneNameFromID(ObjectNode.BoneID);
            }

            memset (ObjectNode.Name, 0, sizeof(Object::Name));
            strncpy (ObjectNode.Name, CorrectBoneName.c_str(), CorrectBoneName.size() +1);

            CAnimBlendSequence Sequence;
            if (!bSkipSequence)
            { 
                CAnimBlendSequence_Constructor(&Sequence);

                OLD_CAnimBlendSequence_SetName(&Sequence, ObjectNode.Name);

                OLD_CAnimBlendSequence_SetBoneTag(&Sequence, ObjectNode.BoneID);
            }

            if (bFirstSeq)
            {
                bFirstSeq = false;
                pAnimHierarchy->m_bRunningCompressed = ObjectNode.FrameType == 3 || ObjectNode.FrameType == 4;
            }

            size_t data_size = 0;
            bool bIsRoot;
            bool bIsCompressed;
            bool bInvalidType = false;
            switch (ObjectNode.FrameType)
            {
            case 1:
                data_size = 20 * ObjectNode.TotalFrames; //sizeof(SChildKeyFrame) * seq.frames_count;
                bIsRoot = false;
                bIsCompressed = false;
                break;
            case 2:
                data_size = 32 * ObjectNode.TotalFrames; //sizeof(SRootKeyFrame) * seq.frames_count;
                bIsRoot = true;
                bIsCompressed = false;
                break;
            case 3:
                data_size = 10 * ObjectNode.TotalFrames; //sizeof(SCompressedChildKeyFrame) * seq.frames_count;
                bIsRoot = false;
                bIsCompressed = true;
                break;
            case 4:
                data_size = 16 * ObjectNode.TotalFrames; //sizeof(SCompressedRootKeyFrame) * seq.frames_count;
                bIsRoot = true;
                bIsCompressed = true;
                break;
            default:
                bInvalidType = true;
                break;
            }
            if (!bInvalidType)
            {
                if (bSkipSequence )
                {
                  IFPElement->skipBytes (data_size);
                }
                else
                { 
                    OLD_CAnimBlendSequence_SetNumFrames(&Sequence, ObjectNode.TotalFrames, bIsRoot, bIsCompressed, pKeyFrames);

                    IFPElement->readBytes(Sequence.m_pFrames, data_size);

                    if (anp3)
                    {
                        pKeyFrames += data_size;
                        Sequence.m_nFlags |= 8u;
                    }

                    MapOfSequences [ CorrectBoneName ] = Sequence;
                }
            }
        }
        
        std::map < std::string, CAnimBlendSequence >::iterator it;
        for (size_t SequenceIndex = 0; SequenceIndex < IFP_TOTAL_SEQUENCES; SequenceIndex++)
        {
            std::string BoneName = BoneNames[SequenceIndex];
            DWORD        BoneID = BoneIds[SequenceIndex];
            
            it = MapOfSequences.find(BoneName);
            if (it != MapOfSequences.end())
            {
              
                CAnimBlendSequence * pSequence = (CAnimBlendSequence*)((BYTE*)pAnimHierarchy->m_pSequences + (sizeof(CAnimBlendSequence) * SequenceIndex));

                memcpy (pSequence, &it->second, sizeof(CAnimBlendSequence));
            }
            else
            {
                insertAnimDummySequence(anp3, pAnimHierarchy, SequenceIndex);
            }
        }

        // This order is very important. As we need support for all 32 bones, we must change the total sequences count
        pAnimHierarchy->m_nSeqCount = IFP_TOTAL_SEQUENCES;

        ofs << std::endl << std::endl;

        if (!pAnimHierarchy->m_bRunningCompressed)
        {
            Call_CAnimBlendHierarchy_RemoveQuaternionFlips(pAnimHierarchy);

            Call_CAnimBlendHierarchy_CalcTotalTime(pAnimHierarchy);
        }
    }
}

void ReadIFPVersion1 ( IFP * IFPElement )
{

}

void insertAnimDummySequence ( bool anp3, CAnimBlendHierarchy * pAnimHierarchy, size_t SequenceIndex)
{
    const char * BoneName = BoneNames [SequenceIndex ];
    DWORD        BoneID   = BoneIds   [SequenceIndex];

    ofs << "(Dummy)SequenceIndex: " << SequenceIndex << "   |   Name:  " << BoneName << "   |   BoneID:  " << BoneID << std::endl;

    CAnimBlendSequence * pSequence = (CAnimBlendSequence*)((BYTE*)pAnimHierarchy->m_pSequences + (0xC * SequenceIndex));
    
    CAnimBlendSequence_Constructor(pSequence);

    OLD_CAnimBlendSequence_SetName(pSequence, BoneName);

    OLD_CAnimBlendSequence_SetBoneTag(pSequence, BoneID);

    bool bIsCompressed = true;
    bool bIsRoot = false;

    const size_t TotalFrames = 1;
    size_t FrameSize = 10; // 10 for child, 16 for root

    if (BoneID == 0) // Normal Bone
    {
        // setting this to true means that it will have translation values
        // Also idle_stance contains rootframe for root bone
        bIsRoot   = true;
        FrameSize = 16;
    }
    const size_t FramesDataSizeInBytes = FrameSize * TotalFrames;
    unsigned char* pKeyFrames = nullptr; 

    

    pKeyFrames = (unsigned char*)OLD_CMemoryMgr_Malloc(FramesDataSizeInBytes);

    OLD_CAnimBlendSequence_SetNumFrames(pSequence, TotalFrames, bIsRoot, bIsCompressed, pKeyFrames);

    switch (BoneID)
    {
        case BoneType::NORMAL: // Normal or Root, both are same
        {     
            // This is a root frame. It contains translation as well, but it's compressed just like quaternion                                
            BYTE FrameData[16] = { 0x1F, 0x00, 0x00, 0x00, 0x53, 0x0B, 0x4D, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::PELVIS: 
        {
            BYTE FrameData[10] = { 0xB0, 0xF7, 0xB0, 0xF7, 0x55, 0xF8, 0xAB, 0x07, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::SPINE: 
        {
            BYTE FrameData[10] = { 0x0E, 0x00, 0x15, 0x00, 0xBE, 0xFF, 0xFF, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::SPINE1: 
        {
            BYTE FrameData[10] = { 0x29, 0x00, 0xD9, 0x00, 0xB5, 0x00, 0xF5, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::NECK: 
        {
            BYTE FrameData[10] = { 0x86, 0xFF, 0xB6, 0xFF, 0x12, 0x02, 0xDA, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::HEAD:
        {
            BYTE FrameData[10] = { 0xFA, 0x00, 0x0C, 0x01, 0x96, 0xFE, 0xDF, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::JAW:
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x2B, 0x0D, 0x16, 0x09, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_BROW:
        {
            BYTE FrameData[10] = { 0xC4, 0x00, 0xFF, 0xFE, 0x47, 0x09, 0xF8, 0x0C, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_BROW: 
        {
            BYTE FrameData[10] = { 0xA2, 0xFF, 0xF8, 0x00, 0x4F, 0x09, 0xF8, 0x0C, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));
    
            break;
        }
        case BoneType::L_CLAVICLE: // Bip01 L Clavicle  
        {
            BYTE FrameData[10] = { 0x7E, 0xF5, 0x82, 0x02, 0x37, 0xF4, 0x6A, 0xFF, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_UPPER_ARM: 
        {
            BYTE FrameData[10] = { 0xA8, 0x02, 0x6D, 0x09, 0x1F, 0xFD, 0x51, 0x0C, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_FORE_ARM: 
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x3A, 0xFE, 0xE6, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_HAND: 
        {
            BYTE FrameData[10] = { 0x9D, 0xF5, 0xBA, 0xFF, 0xEA, 0xFF, 0x29, 0x0C, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_FINGER: 
        {
            BYTE FrameData[10] = { 0xFF, 0xFF, 0x00, 0x00, 0xD8, 0x04, 0x3F, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_FINGER_01: 
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x54, 0x06, 0xB1, 0x0E, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_CLAVICLE: // Bip01 R Clavicle 
        {
            BYTE FrameData[10] = { 0x0B, 0x0A, 0x3D, 0xFE, 0xBB, 0xF3, 0xCF, 0xFE, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_UPPER_ARM: 
        {
            BYTE FrameData[10] = { 0xF7, 0xFD, 0xED, 0xF6, 0xEB, 0xFD, 0xD9, 0x0C, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_FORE_ARM: 
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x42, 0xFF, 0xFB, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_HAND:
        {
            BYTE FrameData[10] = { 0xE9, 0x0B, 0x94, 0x00, 0x25, 0x02, 0x72, 0x0A, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_FINGER:
        {
            BYTE FrameData[10] = { 0x37, 0x00, 0xCB, 0xFF, 0x10, 0x09, 0x2E, 0x0D, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_FINGER_01:
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x60, 0x06, 0xAC, 0x0E, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_BREAST: 
        {
            BYTE FrameData[10] = { 0xC5, 0x01, 0x2B, 0x01, 0x53, 0x0A, 0x09, 0x0C, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_BREAST:
        {
            BYTE FrameData[10] = { 0x2F, 0xFF, 0xA5, 0xFF, 0xBA, 0x0A, 0xD6, 0x0B, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));
    
            break;
        }
        case BoneType::BELLY:
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x20, 0x0B, 0x7F, 0x0B, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));
            
            break;
        }
        case BoneType::L_THIGH: 
        {
            BYTE FrameData[10] = { 0x23, 0xFE, 0x44, 0xF0, 0x19, 0xFE, 0x25, 0x01, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_CALF: 
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x1E, 0xFC, 0x85, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_FOOT:
        {
            BYTE FrameData[10] = { 0xBB, 0xFE, 0x3E, 0xFF, 0xD2, 0x01, 0xD3, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::L_TOE_0: 
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x01, 0x00, 0x8D, 0x0B, 0x12, 0x0B, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_THIGH: 
        {
            BYTE FrameData[10] = { 0x0F, 0xFF, 0x19, 0xF0, 0x44, 0x01, 0xBB, 0x00, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_CALF:
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x64, 0xFD, 0xC9, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_FOOT:
        {
            BYTE FrameData[10] = { 0x11, 0x01, 0x9F, 0xFF, 0x9E, 0x01, 0xE0, 0x0F, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        case BoneType::R_TOE_0:
        {
            BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x50, 0x0B, 0x4F, 0x0B, 0x00, 0x00 };
            memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

            break;
        }
        default:
        {
            ofs << " ERROR: BoneID is not being handled!" << std::endl;
        }
    }

    if (anp3)
    {
        pSequence->m_nFlags |= 8u; //EXTERNAL_KEYFRAMES_MEM;
    }
}


int32_t getBoneIDFromName (std::string const& BoneName)
{

    if (BoneName == "root") return BoneType::NORMAL;
    if (BoneName == "normal") return BoneType::NORMAL;

   
    if (BoneName == "pelvis") return BoneType::PELVIS;
    if (BoneName == "spine") return BoneType::SPINE;
    if (BoneName == "spine1") return BoneType::SPINE1;
    if (BoneName == "neck") return BoneType::NECK; 
    if (BoneName == "head") return BoneType::HEAD;
    if (BoneName == "jaw") return BoneType::JAW; 
    if (BoneName == "lbrow") return BoneType::L_BROW;
    if (BoneName == "rbrow") return BoneType::R_BROW;
    if (BoneName == "bip01lclavicle") return BoneType::L_CLAVICLE;
    if (BoneName == "lupperarm") return BoneType::L_UPPER_ARM;
    if (BoneName == "lforearm") return BoneType::L_FORE_ARM;
    if (BoneName == "lhand") return BoneType::L_HAND;

    if (BoneName == "lfingers") return BoneType::L_FINGER;
    if (BoneName == "lfinger") return BoneType::L_FINGER;

    if (BoneName == "lfinger01") return BoneType::L_FINGER_01;
    if (BoneName == "bip01rclavicle") return BoneType::R_CLAVICLE;
    if (BoneName == "rupperarm") return BoneType::R_UPPER_ARM;
    if (BoneName == "rforearm") return BoneType::R_FORE_ARM;
    if (BoneName == "rhand") return BoneType::R_HAND;

    if (BoneName == "rfingers") return BoneType::R_FINGER;
    if (BoneName == "rfinger") return BoneType::R_FINGER;

    if (BoneName == "rfinger01") return BoneType::R_FINGER_01;
    if (BoneName == "lbreast") return BoneType::L_BREAST;
    if (BoneName == "rbreast") return BoneType::R_BREAST;
    if (BoneName == "belly") return BoneType::BELLY;
    if (BoneName == "lthigh") return BoneType::L_THIGH;
    if (BoneName == "lcalf") return BoneType::L_CALF;
    if (BoneName == "lfoot") return BoneType::L_FOOT;
    if (BoneName == "ltoe0") return BoneType::L_TOE_0;
    if (BoneName == "rthigh") return BoneType::R_THIGH;
    if (BoneName == "rcalf") return BoneType::R_CALF;
    if (BoneName == "rfoot") return BoneType::R_FOOT;
    if (BoneName == "rtoe0") return BoneType::R_TOE_0;

    ofs << "ERROR: getCorrectBoneNameFromName: correct bone ID could not be found for (BoneName): " << BoneName << std::endl;

    return -1;
}


std::string getCorrectBoneNameFromID ( int32_t & BoneID )
{
    if (BoneID == BoneType::NORMAL) return "Normal";
 
    if (BoneID == BoneType::PELVIS) return "Pelvis";
    if (BoneID == BoneType::SPINE) return "Spine";
    if (BoneID == BoneType::SPINE1) return "Spine1";
    if (BoneID == BoneType::NECK) return "Neck";
    if (BoneID == BoneType::HEAD) return "Head";
    if (BoneID == BoneType::JAW) return "Jaw";
    if (BoneID == BoneType::L_BROW) return "L Brow";
    if (BoneID == BoneType::R_BROW) return "R Brow";
    if (BoneID == BoneType::L_CLAVICLE) return "Bip01 L Clavicle";
    if (BoneID == BoneType::L_UPPER_ARM) return "L UpperArm";
    if (BoneID == BoneType::L_FORE_ARM) return "L ForeArm";
    if (BoneID == BoneType::L_HAND) return "L Hand";

    if (BoneID == BoneType::L_FINGER) return "L Finger";

    if (BoneID == BoneType::L_FINGER_01) return "L Finger01";
    if (BoneID == BoneType::R_CLAVICLE) return "Bip01 R Clavicle";
    if (BoneID == BoneType::R_UPPER_ARM) return "R UpperArm";
    if (BoneID == BoneType::R_FORE_ARM) return "R ForeArm";
    if (BoneID == BoneType::R_HAND) return "R Hand";

    if (BoneID == BoneType::R_FINGER) return "R Finger";

    if (BoneID == BoneType::R_FINGER_01) return "R Finger01";
    if (BoneID == BoneType::L_BREAST) return "L breast";
    if (BoneID == BoneType::R_BREAST) return "R breast";
    if (BoneID == BoneType::BELLY) return "Belly";
    if (BoneID == BoneType::L_THIGH) return "L Thigh";
    if (BoneID == BoneType::L_CALF) return "L Calf";
    if (BoneID == BoneType::L_FOOT) return "L Foot";
    if (BoneID == BoneType::L_TOE_0) return "L Toe0";
    if (BoneID == BoneType::R_THIGH) return "R Thigh";
    if (BoneID == BoneType::R_CALF) return "R Calf";
    if (BoneID == BoneType::R_FOOT) return "R Foot";
    if (BoneID == BoneType::R_TOE_0) return "R Toe0";

    ofs << "ERROR: getCorrectBoneNameFromID: correct bone name could not be found for (BoneID):" << BoneID << std::endl;
    
    return "Unknown";
}

size_t getCorrectBoneIndexFromID(int32_t & BoneID)
{
    if (BoneID == BoneType::NORMAL) return 0;

    if (BoneID == BoneType::PELVIS) return 1;
    if (BoneID == BoneType::SPINE) return 2;
    if (BoneID == BoneType::SPINE1) return 3;
    if (BoneID == BoneType::NECK) return 4;
    if (BoneID == BoneType::HEAD) return 5;
    if (BoneID == BoneType::JAW) return 6;
    if (BoneID == BoneType::L_BROW) return 7;
    if (BoneID == BoneType::R_BROW) return 8;
    if (BoneID == BoneType::L_CLAVICLE) return 9;
    if (BoneID == BoneType::L_UPPER_ARM) return 10;
    if (BoneID == BoneType::L_FORE_ARM) return 11;
    if (BoneID == BoneType::L_HAND) return 12;

    if (BoneID == BoneType::L_FINGER) return 13;

    if (BoneID == BoneType::L_FINGER_01) return 14;
    if (BoneID == BoneType::R_CLAVICLE) return 15;
    if (BoneID == BoneType::R_UPPER_ARM) return 16;
    if (BoneID == BoneType::R_FORE_ARM) return 17;
    if (BoneID == BoneType::R_HAND) return 18;

    if (BoneID == BoneType::R_FINGER) return 19;

    if (BoneID == BoneType::R_FINGER_01) return 20;
    if (BoneID == BoneType::L_BREAST) return 21;
    if (BoneID == BoneType::R_BREAST) return 22;
    if (BoneID == BoneType::BELLY) return 23;
    if (BoneID == BoneType::L_THIGH) return 24;
    if (BoneID == BoneType::L_CALF) return 25;
    if (BoneID == BoneType::L_FOOT) return 26;
    if (BoneID == BoneType::L_TOE_0) return 27;
    if (BoneID == BoneType::R_THIGH) return 28;
    if (BoneID == BoneType::R_CALF) return 29;
    if (BoneID == BoneType::R_FOOT) return 30;
    if (BoneID == BoneType::R_TOE_0) return 31;

    ofs << "ERROR: getCorrectBoneIndexFromID: correct bone index could not be found for (BoneID):" << BoneID << std::endl;

    return -1;
}

std::string getCorrectBoneNameFromName(std::string const& BoneName)
{

    if (BoneName == "root") return "Normal";
    if (BoneName == "normal") return "Normal";


    if (BoneName == "pelvis") return "Pelvis";
    if (BoneName == "spine") return "Spine";
    if (BoneName == "spine1") return "Spine1";
    if (BoneName == "neck") return "Neck";
    if (BoneName == "head") return "Head";
    if (BoneName == "jaw") return "Jaw";
    if (BoneName == "lbrow") return "L Brow";
    if (BoneName == "rbrow") return "R Brow";
    if (BoneName == "bip01lclavicle") return "Bip01 L Clavicle";
    if (BoneName == "lupperarm") return "L UpperArm";
    if (BoneName == "lforearm") return "L ForeArm";
    if (BoneName == "lhand") return "L Hand";

    if (BoneName == "lfingers") return "L Finger";
    if (BoneName == "lfinger") return "L Finger";

    if (BoneName == "lfinger01") return "L Finger01";
    if (BoneName == "bip01rclavicle") return "Bip01 R Clavicle";
    if (BoneName == "rupperarm") return "R UpperArm";
    if (BoneName == "rforearm") return "R ForeArm";
    if (BoneName == "rhand") return "R Hand";

    if (BoneName == "rfingers") return "R Finger";
    if (BoneName == "rfinger") return "R Finger";

    if (BoneName == "rfinger01") return "R Finger01";
    if (BoneName == "lbreast") return "L Breast";
    if (BoneName == "rbreast") return "R Breast";
    if (BoneName == "belly") return "Belly";
    if (BoneName == "lthigh") return "L Thigh";
    if (BoneName == "lcalf") return "L Calf";
    if (BoneName == "lfoot") return "L Foot";
    if (BoneName == "ltoe0") return "L Toe0";
    if (BoneName == "rthigh") return "R Thigh";
    if (BoneName == "rcalf") return "R Calf";
    if (BoneName == "rfoot") return "R Foot";
    if (BoneName == "rtoe0") return "R Toe0";
      
    ofs <<"ERROR: getCorrectBoneNameFromName: correct bone name could not be found for (BoneName):" << BoneName << std::endl;

    return BoneName;
}

// ----------------------------------------------------------------------------------------------------------
// --------------------------------------  For Hierarchy ----------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
void Call_CAnimBlendHierarchy_RemoveQuaternionFlips(CAnimBlendHierarchy * pAnimHierarchy)
{
    __asm
    {
        push ecx;
        mov ecx, pAnimHierarchy;
    };

    OLD_CAnimBlendHierarchy_RemoveQuaternionFlips();

    __asm pop ecx;
}

void Call_CAnimBlendHierarchy_CalcTotalTime(CAnimBlendHierarchy * pAnimHierarchy)
{

    __asm
    {
        push ecx;
        mov ecx, pAnimHierarchy;
    };

    OLD_CAnimBlendHierarchy_CalcTotalTime();

    __asm pop ecx;
}
