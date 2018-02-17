#include "IFPLoader.h"
#include <algorithm>

extern std::ofstream ofs;
extern std::wofstream ofsW;

std::vector <IFP> g_IFPs;

extern hCAnimBlendHierarchy_SetName OLD_CAnimBlendHierarchy_SetName;

hCMemoryMgr_Malloc		         OLD_CMemoryMgr_Malloc = (hCMemoryMgr_Malloc)0x0072F420;
hCAnimBlendSequence_SetName      OLD_CAnimBlendSequence_SetName = (hCAnimBlendSequence_SetName)0x4D0C50;
hCAnimBlendSequence_SetBoneTag   OLD_CAnimBlendSequence_SetBoneTag = (hCAnimBlendSequence_SetBoneTag)0x4D0C70;
hCAnimBlendSequence_SetNumFrames OLD_CAnimBlendSequence_SetNumFrames = (hCAnimBlendSequence_SetNumFrames)0x4D0CD0;

hCAnimBlendHierarchy_RemoveQuaternionFlips OLD_CAnimBlendHierarchy_RemoveQuaternionFlips = (hCAnimBlendHierarchy_RemoveQuaternionFlips)0x4CF4E0;
hCAnimBlendHierarchy_CalcTotalTime         OLD_CAnimBlendHierarchy_CalcTotalTime = (hCAnimBlendHierarchy_CalcTotalTime)0x4CF2F0;

void ReadAnimSequencesWithDummies(IFP * IFPElement, bool anp3, CAnimBlendHierarchy * pAnimHierarchy, unsigned char *st);
void insertAnimDummySequence(bool anp3, CAnimBlendHierarchy * pAnimHierarchy, size_t SequenceIndex);

DWORD BoneIds[] =
{
	0,
	1,
	2,
	3,
	4,
	5,
	8,
	6,
	7,
	31,
	32,
	33,
	34,
	35,
	36,
	21,
	22,
	23,
	24,
	25,
	26,
	302,
	301,
	201,
	41,
	42,
	43,
	44,
	51,
	52,
	53,
	54
};

char BoneNames[][24] = {
	"Normal",
	"Pelvis",
	"Spine",
	"Spine1",
	"Neck",
	"Head",
	"Jaw",
	"L Brow",
	"R Brow ",
	"Bip01 L Clavicle",
	"L UpperArm",
	"L ForeArm",
	"L Hand",
	"L Finger",
	"L Finger01",
	"Bip01 R Clavicle",
	"R UpperArm",
	"R ForeArm",
	"R Hand",
	"R Finger",
	"R Finger01",
	"L breast",
	"R breast",
	"Belly",
	"L Thigh",
	"L Calf",
	"L Foot",
	"L Toe0",
	"R Thigh",
	"R Calf",
	"R Foot",
	"R Toe0"
};

void LoadIFPFile(const char * FilePath)
{
	static size_t IfpIndex = 0;

	g_IFPs.emplace_back ();

	IFP & IFPElement = g_IFPs [ IfpIndex ];

	IfpIndex++;

	IFPElement.createLoader(FilePath);

	if (IFPElement.loadFile())
	{
		printf("IfpLoader: File loaded. Parsing it now.\n");

		IFPHeader * Header = &IFPElement.Header;

		IFPElement.readBuffer < IFPHeader >(Header);

		ofs << "Total Animations: " << Header->TotalAnimations << std::endl;

		if (strncmp(Header->Version, "ANP2", sizeof(Header->Version)) == 0 || strncmp(Header->Version, "ANP3", sizeof(Header->Version)) == 0)
		{
			bool anp3 = strncmp(Header->Version, "ANP3", sizeof(Header->Version)) == 0;

			ReadIfpAnp23(&IFPElement, anp3);
		}
		else
		{
			//ReadANP1(stream, loadCompressed, uncompressedAnims);
		}

		// We are unloading the data because we don't need to read it anymore. 
		// This function does not unload IFP, to unload ifp call unloadIFP function
		IFPElement.unloadFile();
	}

	//g_IFPs.push_back(IFPElement);

	printf("Exiting LoadIFPFile function\n");
	ofs << "Exiting LoadIFPFile function" << std::endl;
}



void ReadIfpAnp23(IFP * IFPElement, bool anp3)
{
	IFPElement->AnimationHierarchies.resize(IFPElement->Header.TotalAnimations);


	for (size_t i = 0; i < IFPElement->AnimationHierarchies.size(); i++)
	{
		CAnimBlendHierarchy * pAnimHierarchy = &IFPElement->AnimationHierarchies[i];


		CAnimBlendHierarchy_Constructor(pAnimHierarchy);

		Animation AnimationNode;

		IFPElement->readCString((char *)&AnimationNode.Name, sizeof(Animation::Name));
		IFPElement->readBuffer < int32_t >(&AnimationNode.TotalObjects);

		ofs << "Animation Name: " << AnimationNode.Name << std::endl;

		printf("Animation Name: %s    |  Index: %d \n", AnimationNode.Name, i);

		unsigned char *st = nullptr;
		if (anp3)
		{
			IFPElement->readBuffer < int32_t >(&AnimationNode.FrameSize);
			IFPElement->readBuffer < int32_t >(&AnimationNode.isCompressed);

			pAnimHierarchy->m_bRunningCompressed = AnimationNode.isCompressed & 1;

			st = (unsigned char*)OLD_CMemoryMgr_Malloc(AnimationNode.FrameSize);
		}

		OLD_CAnimBlendHierarchy_SetName(pAnimHierarchy, (char*)&AnimationNode.Name);

		pAnimHierarchy->m_nSeqCount = AnimationNode.TotalObjects;
		pAnimHierarchy->m_nAnimBlockId = 0;
		pAnimHierarchy->field_B = 0;

		unsigned short TotalMaxSequences = pAnimHierarchy->m_nSeqCount;

		if (TotalMaxSequences < IFP_TOTAL_SEQUENCES)
		{
			TotalMaxSequences = IFP_TOTAL_SEQUENCES;
		}

		ofs << "TotalMaxSequences: " << TotalMaxSequences << std::endl;

		char * pNewSequencesMemory = (char *)operator new(12 * TotalMaxSequences + 4);//  Allocate memory for sequences ( 12 * seq_count + 4 )

		pAnimHierarchy->m_pSequences = (CAnimBlendSequence *)(pNewSequencesMemory + 4);

		// 242 = SEAT_down animation, 127 = facanger
		if ( i == 218)
		{
			if (pNewSequencesMemory)
			{
				*(DWORD *)pNewSequencesMemory = TotalMaxSequences;
			}

			ReadAnimSequencesWithDummies ( IFPElement,  anp3, pAnimHierarchy, st );
		}
		else
		{
			if (pNewSequencesMemory)
			{
				*(DWORD *)pNewSequencesMemory = pAnimHierarchy->m_nSeqCount;
			}

			bool bFirstSeq = true;
			for (size_t j = 0; j < pAnimHierarchy->m_nSeqCount; j++)
			{
				CAnimBlendSequence *pSequence = (CAnimBlendSequence*)((BYTE*)pAnimHierarchy->m_pSequences + (0xC * j)); // &IFPElement->AnimationSequences[j];

				CAnimBlendSequence_Constructor(pSequence);

				Object ObjectNode;

				IFPElement->readBuffer < Object >(&ObjectNode);

				ofs << "SequenceIndex: " << j;

				if (ObjectNode.FrameType == 4)
				{
					ofs << "   |   Type: Root";
				}
				else
				{
					ofs << "   |   Type: Child";
				}
				ofs << "   |   Name:  " << ObjectNode.Name << "   |   BoneID:  " << ObjectNode.BoneID;
				ofs << "   |   TotalFrames: " << ObjectNode.TotalFrames;


				OLD_CAnimBlendSequence_SetName(pSequence, ObjectNode.Name);

				OLD_CAnimBlendSequence_SetBoneTag(pSequence, ObjectNode.BoneID);

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
					OLD_CAnimBlendSequence_SetNumFrames(pSequence, ObjectNode.TotalFrames, bIsRoot, bIsCompressed, st);

					IFPElement->readBytes(pSequence->m_pFrames, data_size);

					ofs << "   | pSequence->m_pFrames Address: " << std::hex << pSequence->m_pFrames << std::dec << std::endl;

					if (anp3)
					{
						st += data_size;
						pSequence->m_nFlags |= 8u; //EXTERNAL_KEYFRAMES_MEM;
					}
				}
			}
		}
		

		ofs << std::endl << std::endl;

		if (!pAnimHierarchy->m_bRunningCompressed)
		{
			Call_CAnimBlendHierarchy_RemoveQuaternionFlips(pAnimHierarchy);

			Call_CAnimBlendHierarchy_CalcTotalTime(pAnimHierarchy);
		}
	}
}

void ReadAnimSequencesWithDummies (IFP * IFPElement, bool anp3, CAnimBlendHierarchy * pAnimHierarchy, unsigned char *st)
{
	size_t SequenceIndex = 0;
	size_t TotalSequencesRead = 0;

	Object ObjectNode;

	bool bFirstSeq = true;
	while (SequenceIndex < IFP_TOTAL_SEQUENCES )
	{
		ofs << "ReadAnimSequencesWithDummies: SequenceIndex: " << SequenceIndex << std::endl;

		if (TotalSequencesRead < pAnimHierarchy->m_nSeqCount)
		{
			IFPElement->readBuffer < Object >(&ObjectNode);
			TotalSequencesRead++;
		}
		
		if (SequenceIndex == 0)
		{
			strncpy(ObjectNode.Name, "Normal", sizeof("Normal"));
		}
		

		while (BoneIds[SequenceIndex] != ObjectNode.BoneID)
		{
			insertAnimDummySequence(anp3, pAnimHierarchy, SequenceIndex);
			SequenceIndex++;

			if (SequenceIndex >=  IFP_TOTAL_SEQUENCES)
			{
				break;
			}
		}

		if (SequenceIndex >= IFP_TOTAL_SEQUENCES )
		{
			break;
		}

		ofs << "SequenceIndex: " << SequenceIndex;

		if (ObjectNode.FrameType == 4)
		{
			ofs << "   |   Type: Root";
		}
		else
		{
			ofs << "   |   Type: Child";
		}
		ofs << "   |   Name:  " << ObjectNode.Name << "   |   BoneID:  " << ObjectNode.BoneID << std::endl;

		CAnimBlendSequence *pSequence = (CAnimBlendSequence*)((BYTE*)pAnimHierarchy->m_pSequences + (0xC * SequenceIndex)); // &IFPElement->AnimationSequences[j];

		CAnimBlendSequence_Constructor(pSequence);

		OLD_CAnimBlendSequence_SetName(pSequence, ObjectNode.Name);

		OLD_CAnimBlendSequence_SetBoneTag(pSequence, ObjectNode.BoneID);

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
			OLD_CAnimBlendSequence_SetNumFrames(pSequence, ObjectNode.TotalFrames, bIsRoot, bIsCompressed, st);

			IFPElement->readBytes(pSequence->m_pFrames, data_size);

			if (anp3)
			{
				st += data_size;
				pSequence->m_nFlags |= 8u; //EXTERNAL_KEYFRAMES_MEM;
			}
		}

		SequenceIndex++;
	}

	// This order is very important. As we need support for all 32 bones, we must change the total sequences count
	pAnimHierarchy->m_nSeqCount = IFP_TOTAL_SEQUENCES;
}

void insertAnimDummySequence ( bool anp3, CAnimBlendHierarchy * pAnimHierarchy, size_t SequenceIndex)
{
	const char * BoneName = BoneNames [ SequenceIndex ];
	DWORD        BoneID	  = BoneIds   [ SequenceIndex ];

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
		bIsRoot	  = true;
		FrameSize = 16;
	}
	const size_t FramesDataSizeInBytes = FrameSize * TotalFrames;
	unsigned char* pKeyFrames = nullptr; 

	

	pKeyFrames = (unsigned char*)OLD_CMemoryMgr_Malloc(FramesDataSizeInBytes);

	OLD_CAnimBlendSequence_SetNumFrames(pSequence, TotalFrames, bIsRoot, bIsCompressed, pKeyFrames);
	
	/* 10 Bytes, Child Frames

	Name: Jaw | BoneID: 8
	00 00 00 00 37 0D 04 09 00 00

	Name: L Brow | BoneID: 6
	C4 00 FF FE 47 09 F8 0C 00 00

	Name: R Brow | BoneID: 7
	A2 FF F8 00 4F 09 F8 0C 00 00

	Name: L breast | BoneID: 302
	C5 01 2B 01 53 0A 09 0C 00 00

	Name: R breast | BoneID: 301
	2F FF A5 FF BA 0A D6 0B 00 00

	Name: Belly | BoneID: 201
	00 00 00 00 20 0B 7F 0B 00 00
	*/

	switch (BoneID)
	{
		case 0: // Normal or Root, both are same
		{                                     // Translate x,yz
			//1F 00 00 00 53 0B 4D 0B 00 00     00 00 00 00 18 00
			BYTE FrameData[16] = { 0x1F, 0x00, 0x00, 0x00, 0x53, 0x0B, 0x4D, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 1: // Pelvis
		{
			BYTE FrameData[10] = { 0xB0, 0xF7, 0xB0, 0xF7, 0x55, 0xF8, 0xAB, 0x07, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 2: // Spine
		{
			BYTE FrameData[10] = { 0x0E, 0x00, 0x15, 0x00, 0xBE, 0xFF, 0xFF, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 3: //  Spine1
		{
			BYTE FrameData[10] = { 0x29, 0x00, 0xD9, 0x00, 0xB5, 0x00, 0xF5, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 4: // Neck
		{
			BYTE FrameData[10] = { 0x86, 0xFF, 0xB6, 0xFF, 0x12, 0x02, 0xDA, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 5: // Head
		{
			BYTE FrameData[10] = { 0xFA, 0x00, 0x0C, 0x01, 0x96, 0xFE, 0xDF, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 8:
		{
			BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x2B, 0x0D, 0x16, 0x09, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 6:
		{
			BYTE FrameData[10] = { 0xC4, 0x00, 0xFF, 0xFE, 0x47, 0x09, 0xF8, 0x0C, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 7:
		{
			BYTE FrameData[10] = { 0xA2, 0xFF, 0xF8, 0x00, 0x4F, 0x09, 0xF8, 0x0C, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));
	
			break;
		}
		case 31: // Bip01 L Clavicle  
		{
			BYTE FrameData[10] = { 0x7E, 0xF5, 0x82, 0x02, 0x37, 0xF4, 0x6A, 0xFF, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 32: // L UpperArm
		{
			BYTE FrameData[10] = { 0xA8, 0x02, 0x6D, 0x09, 0x1F, 0xFD, 0x51, 0x0C, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 33: // L ForeArm
		{
			BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x3A, 0xFE, 0xE6, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 34: // L Hand
		{
			BYTE FrameData[10] = { 0x9D, 0xF5, 0xBA, 0xFF, 0xEA, 0xFF, 0x29, 0x0C, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 35: // L Finger
		{
			BYTE FrameData[10] = { 0xFF, 0xFF, 0x00, 0x00, 0xD8, 0x04, 0x3F, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 36: // L Finger01
		{
			BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x54, 0x06, 0xB1, 0x0E, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 21: // Bip01 R Clavicle 
		{
			BYTE FrameData[10] = { 0x0B, 0x0A, 0x3D, 0xFE, 0xBB, 0xF3, 0xCF, 0xFE, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 22: // R UpperArm 
		{
			BYTE FrameData[10] = { 0xF7, 0xFD, 0xED, 0xF6, 0xEB, 0xFD, 0xD9, 0x0C, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 23: // R ForeArm
		{
			BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x42, 0xFF, 0xFB, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 24: // R Hand
		{
			BYTE FrameData[10] = { 0xE9, 0x0B, 0x94, 0x00, 0x25, 0x02, 0x72, 0x0A, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 25: // R Finger
		{
			BYTE FrameData[10] = { 0x37, 0x00, 0xCB, 0xFF, 0x10, 0x09, 0x2E, 0x0D, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 26: // R Finger01
		{
			BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x60, 0x06, 0xAC, 0x0E, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 302:
		{
			BYTE FrameData[10] = { 0xC5, 0x01, 0x2B, 0x01, 0x53, 0x0A, 0x09, 0x0C, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 301:
		{
			BYTE FrameData[10] = { 0x2F, 0xFF, 0xA5, 0xFF, 0xBA, 0x0A, 0xD6, 0x0B, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));
	
			break;
		}
		case 201:
		{
			BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x20, 0x0B, 0x7F, 0x0B, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));
			
			break;
		}
		case 41: // L Thigh
		{
			BYTE FrameData[10] = { 0x23, 0xFE, 0x44, 0xF0, 0x19, 0xFE, 0x25, 0x01, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 42: // L Calf
		{
			BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x1E, 0xFC, 0x85, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 43: // L Foot
		{
			BYTE FrameData[10] = { 0xBB, 0xFE, 0x3E, 0xFF, 0xD2, 0x01, 0xD3, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 44: // L Toe0
		{
			BYTE FrameData[10] = { 0x00, 0x00, 0x01, 0x00, 0x8D, 0x0B, 0x12, 0x0B, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 51: // R Thigh
		{
			BYTE FrameData[10] = { 0x0F, 0xFF, 0x19, 0xF0, 0x44, 0x01, 0xBB, 0x00, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 52: // R Calf
		{
			BYTE FrameData[10] = { 0x00, 0x00, 0x00, 0x00, 0x64, 0xFD, 0xC9, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 53: // R Foot
		{
			BYTE FrameData[10] = { 0x11, 0x01, 0x9F, 0xFF, 0x9E, 0x01, 0xE0, 0x0F, 0x00, 0x00 };
			memcpy(pSequence->m_pFrames, FrameData, sizeof(FrameData));

			break;
		}
		case 54: // R Toe0
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

	//memset(pSequence->m_pFrames, 5, FramesDataSizeInBytes);


	if (anp3)
	{
		pSequence->m_nFlags |= 8u; //EXTERNAL_KEYFRAMES_MEM;
	}
}

/*

void ReadIfpAnp23(IFP * IFPElement, bool anp3)
{

	IFPElement->AnimationHierarchies.resize(IFPElement->Header.TotalAnimations);

	size_t SequencesIndex = 0;

	for (size_t i = 0; i < IFPElement->AnimationHierarchies.size(); i++)
	{
		CAnimBlendHierarchy * pAnimHierarchy = &IFPElement->AnimationHierarchies[i];

		CAnimBlendHierarchy_Constructor(pAnimHierarchy);

		Animation AnimationNode;

		IFPElement->readCString((char *)&AnimationNode.Name, sizeof(Animation::Name));
		IFPElement->readBuffer < int32_t >(&AnimationNode.TotalObjects);

		//ofs << "Animation Name: " << AnimationNode.Name << std::endl;

		printf("Animation Name: %s    |  Index: %d \n", AnimationNode.Name, i);

		unsigned char *st = nullptr;
		if (anp3)
		{
			IFPElement->readBuffer < int32_t >(&AnimationNode.FrameSize);
			IFPElement->readBuffer < int32_t >(&AnimationNode.isCompressed);

			pAnimHierarchy->m_bRunningCompressed = AnimationNode.isCompressed & 1;

			st = (unsigned char*)OLD_CMemoryMgr_Malloc(AnimationNode.FrameSize);
		}

		OLD_CAnimBlendHierarchy_SetName(pAnimHierarchy, (char*)&AnimationNode.Name);

		pAnimHierarchy->m_nSeqCount = AnimationNode.TotalObjects;

		size_t SequencesEndIndex = SequencesIndex + pAnimHierarchy->m_nSeqCount;

		IFPElement->AnimationSequences.resize ( SequencesEndIndex );

		pAnimHierarchy->m_pSequences = &IFPElement->AnimationSequences[SequencesIndex];
		pAnimHierarchy->field_B = 0;

		bool bFirstSeq = true;

		
		for (size_t j = SequencesIndex; j < SequencesEndIndex; j++)
		{
			CAnimBlendSequence *pSequence = &IFPElement->AnimationSequences[j];

			CAnimBlendSequence_Constructor(pSequence);

			Object ObjectNode;

			IFPElement->readBuffer < Object >(&ObjectNode);

			OLD_CAnimBlendSequence_SetName(pSequence, ObjectNode.Name);

			OLD_CAnimBlendSequence_SetBoneTag(pSequence, ObjectNode.BoneID);

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
				OLD_CAnimBlendSequence_SetNumFrames(pSequence, ObjectNode.TotalFrames, bIsRoot, bIsCompressed, st);

				IFPElement->readBytes(pSequence->m_pFrames, data_size);

				if (anp3)
				{
					st += data_size;
					pSequence->m_nFlags |= 8u; //EXTERNAL_KEYFRAMES_MEM;
				}
			}
		}

		if (!pAnimHierarchy->m_bRunningCompressed)
		{
			Call_CAnimBlendHierarchy_RemoveQuaternionFlips(pAnimHierarchy);

			Call_CAnimBlendHierarchy_CalcTotalTime(pAnimHierarchy);
		}

		SequencesIndex += AnimationNode.TotalObjects;
		//ofs << std::endl << std::endl;
	}
}
*/



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
