#include <Windows.h>
#include <fstream>
#include "wrapper.h"
#include "IFPLoader.h"

extern std::ofstream ofs;
extern std::wofstream ofsW;

extern hLoadAnimFile_stream OLD_LoadAnimFile_stream;
extern hGetAnimationBlock   OLD_GetAnimationBlock;

extern hCAnimBlendHierarchy_SetName OLD_CAnimBlendHierarchy_SetName;

extern hCMemoryMgr_Malloc		         OLD_CMemoryMgr_Malloc;
extern hCAnimBlendSequence_SetName      OLD_CAnimBlendSequence_SetName;
extern hCAnimBlendSequence_SetBoneTag   OLD_CAnimBlendSequence_SetBoneTag;
extern hCAnimBlendSequence_SetNumFrames OLD_CAnimBlendSequence_SetNumFrames;

extern hCAnimBlendHierarchy_RemoveQuaternionFlips OLD_CAnimBlendHierarchy_RemoveQuaternionFlips;
extern hCAnimBlendHierarchy_CalcTotalTime         OLD_CAnimBlendHierarchy_CalcTotalTime;


hCreateAnimAssocGroups OLD_CreateAnimAssocGroups = (hCreateAnimAssocGroups)0x004D3CC0;
hRwStreamOpen          OLD_RwStreamOpen          = (hRwStreamOpen)0x007ECEF0;
hRwStreamClose         OLD_RwStreamClose         = (hRwStreamClose)0x007ECE20;
hGetFirstAssocGroup    OLD_GetFirstAssocGroup    = (hGetFirstAssocGroup)0x004D39B0;

size_t * g_numAnimAssocDefinitions = (size_t *)0x00B4EA28;
size_t * g_AnimAssocGroups = (size_t *)0x00B4EA34;

int * g_numAnimBlocks = (int *)VAR_CAnimManager_NumAnimBlocks;
int * g_numAnimations = (int *)VAR_CAnimManager_NumAnimations;

void __cdecl CAnimBlendAssocGroup_Constructor(char * pThis);
void CAnimBlendSequence_Constructor(CAnimBlendSequence * pSequence);
void LoadPedAnimFile(const char * FilePath);
void ReadPedIfpAnp23(IFP * IFPElement, bool anp3);

int NEW_LoadPedAnimIFPFile (void)
{
	void *pAnimAssocGroups; 
	//int * RwIFPStream;
	size_t NumNumAnimAssocGroups; 
	char *NewMemory; 
	char PedIfpPath [] = "C:\\Users\\danish\\Desktop\\gtaTxdDff\\ped.ifp";

	pAnimAssocGroups = 0;

	/*
	RwIFPStream = OLD_RwStreamOpen(2, 1, (DWORD*)&PedIfpPath);
	OLD_LoadAnimFile_stream((int)RwIFPStream, 1, 0);
	OLD_RwStreamClose((DWORD*)RwIFPStream, 0);
	*/

	LoadPedAnimFile ( (char*)&PedIfpPath);

	NumNumAnimAssocGroups = *g_numAnimAssocDefinitions;
	NewMemory = (char *)operator new(20 * NumNumAnimAssocGroups + 4);// Size of each AssocGroup is 20 bytes, first 4 bytes contain Total AnimAssocGroups
																		 // 


	if (NewMemory)
	{
		pAnimAssocGroups = NewMemory + 4;           // Skip first four bytes as it should contain Total AnimAssocGroups
		*(DWORD *)NewMemory = NumNumAnimAssocGroups;// As you can see that the first 4 bytes consists of a DWORD holding Total AnimAssocGroups

		for (size_t i = 0; i < NumNumAnimAssocGroups; i++)
		{
			char * pThis = (char*)((BYTE*)pAnimAssocGroups + (0x14 * i));

			CAnimBlendAssocGroup_Constructor ( pThis );
		}
	}

	*g_AnimAssocGroups = (size_t)pAnimAssocGroups;

	return OLD_CreateAnimAssocGroups();
}

IFP g_PedIFPElement;

void LoadPedAnimFile ( const char * FilePath )
{
	IFP & IFPElement = g_PedIFPElement;

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

			ReadPedIfpAnp23(&IFPElement, anp3);
		}
		else
		{
			//ReadANP1(stream, loadCompressed, uncompressedAnims);
		}

		// We are unloading the data because we don't need to read it anymore. 
		// This function does not unload IFP, to unload ifp call unloadIFP function
		IFPElement.unloadFile();
	}

}


void ReadPedIfpAnp23(IFP * IFPElement, bool anp3)
{
	const char * BlockName = IFPElement->Header.InternalFileName;

	CAnimBlock* animBlock = OLD_GetAnimationBlock ( BlockName );
	if (animBlock)
	{
		if (animBlock->m_numAnims  == 0)
		{
			animBlock->m_numAnims = IFPElement->Header.TotalAnimations;
			animBlock->m_idOffset = *g_numAnimations;
		}
	}
	else
	{
		animBlock = (CAnimBlock*)((BYTE*)ARRAY_CAnimManager_AnimBlocks + sizeof(CAnimBlock) * (*g_numAnimBlocks));

		*g_numAnimBlocks = *g_numAnimBlocks + 1;

		strncpy(animBlock->m_name, BlockName, 0x10); // 0x10 = 16 in decimal

		animBlock->m_numAnims = IFPElement->Header.TotalAnimations;
		animBlock->m_idOffset = *g_numAnimations;
		animBlock->m_assocGroup = OLD_GetFirstAssocGroup(animBlock->m_name);
	}

	int AnimationIndex = animBlock->m_idOffset;

	animBlock->m_loaded = true;

	for (size_t i = 0; i < animBlock->m_numAnims; i++)
	{
		CAnimBlendHierarchy * pAnimHierarchy = (CAnimBlendHierarchy*)((BYTE*)ARRAY_CAnimManager_Animations + sizeof(CAnimBlendHierarchy) * AnimationIndex);

		AnimationIndex++;

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
		pAnimHierarchy->m_nAnimBlockId = (animBlock - (CAnimBlock*)ARRAY_CAnimManager_AnimBlocks) / sizeof(CAnimBlock);

		ofs << "pAnimHierarchy->m_nAnimBlockId:  " << pAnimHierarchy->m_nAnimBlockId << std::endl;

		pAnimHierarchy->field_B = 0;

		char * pNewSequencesMemory = (char *)operator new(12 * pAnimHierarchy->m_nSeqCount + 4);//  Allocate memory for sequences ( 12 * seq_count + 4 )
	
		int pSequences = NULL;
		if (pNewSequencesMemory)
		{
			pSequences = (int)(pNewSequencesMemory + 4);

			*(DWORD *)pNewSequencesMemory = pAnimHierarchy->m_nSeqCount;
		}
		else
		{
			pSequences = 0;
		}
		
		pAnimHierarchy->m_pSequences = (CAnimBlendSequence *)(pNewSequencesMemory + 4);
	

		bool bFirstSeq = true;

		for (size_t j = 0; j < pAnimHierarchy->m_nSeqCount; j++)
		{
			CAnimBlendSequence *pSequence = (CAnimBlendSequence*)((BYTE*)pAnimHierarchy->m_pSequences + (0xC * j)); // &IFPElement->AnimationSequences[j];

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

		//ofs << std::endl << std::endl;
	}

	if (AnimationIndex > *g_numAnimations)
	{
		*g_numAnimations = AnimationIndex;
	}

	ofs << "*g_numAnimations: " << *g_numAnimations << std::endl;
}

void __cdecl CAnimBlendAssocGroup_Constructor(char * pThis)
{
	*(DWORD *)pThis = 0;
	*((DWORD *)pThis + 1) = 0;
	*((DWORD *)pThis + 2) = 0;
	*((DWORD *)pThis + 3) = 0;
	*((DWORD *)pThis + 4) = -1;
}