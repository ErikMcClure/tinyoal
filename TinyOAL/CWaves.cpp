/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "openAL\CWaves.h"
#include <ks.h>
#include <ksmedia.h>

#pragma pack(push, 4)

typedef struct
{
	char			szRIFF[4];
	unsigned long	ulRIFFSize;
	char			szWAVE[4];
} WAVEFILEHEADER;

typedef struct
{
	char			szChunkName[4];
	unsigned long	ulChunkSize;
} RIFFCHUNK;

typedef struct
{
	unsigned short	usFormatTag;
	unsigned short	usChannels;
	unsigned long	ulSamplesPerSec;
	unsigned long	ulAvgBytesPerSec;
	unsigned short	usBlockAlign;
	unsigned short	usBitsPerSample;
	unsigned short	usSize;
	unsigned short  usReserved;
	unsigned long	ulChannelMask;
    GUID            guidSubFormat;
} WAVEFMT;

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaves::CWaves()
{
	memset(&m_WaveIDs, 0, sizeof(m_WaveIDs));
}

CWaves::~CWaves()
{
	long lLoop;

	for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++)
	{
		if (m_WaveIDs[lLoop])
		{
			if (m_WaveIDs[lLoop]->pData)
				delete m_WaveIDs[lLoop]->pData;

			if (m_WaveIDs[lLoop]->pFile)
				m_WaveIDs[lLoop]->callbacks.close_func(m_WaveIDs[lLoop]->pFile);

			delete m_WaveIDs[lLoop];
			m_WaveIDs[lLoop] = 0;
		}
	}
}


WAVERESULT CWaves::LoadWaveFile(void *source, WAVEID *pWaveID, wav_callbacks& callbacks)
{
	WAVERESULT wr = WR_OUTOFMEMORY;
	LPWAVEFILEINFO pWaveInfo;

	pWaveInfo = new WAVEFILEINFO;
	if (pWaveInfo)
	{
		if (SUCCEEDED(wr = ParseFile(source, pWaveInfo, callbacks)))
		{
			// Allocate memory for sample data
			pWaveInfo->pData = new char[pWaveInfo->ulDataSize];
			if (pWaveInfo->pData)
			{
				// Seek to start of audio data
				pWaveInfo->callbacks.seek_func(pWaveInfo->pFile, pWaveInfo->ulDataOffset, SEEK_SET);

				// Read Sample Data
				if (pWaveInfo->callbacks.read_func(pWaveInfo->pData, 1, pWaveInfo->ulDataSize, pWaveInfo->pFile) == pWaveInfo->ulDataSize)
				{
					long lLoop = 0;
					for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++)
					{
						if (!m_WaveIDs[lLoop])
						{
							m_WaveIDs[lLoop] = pWaveInfo;
							*pWaveID = lLoop;
							wr = WR_OK;
							break;
						}
					}

					if (lLoop == MAX_NUM_WAVEID)
					{
						delete pWaveInfo->pData;
						wr = WR_OUTOFMEMORY;
					}
				}
				else
				{
					delete pWaveInfo->pData;
					wr = WR_BADWAVEFILE;
				}
			}
			else
			{
				wr = WR_OUTOFMEMORY;
			}

			pWaveInfo->callbacks.close_func(pWaveInfo->pFile);
			pWaveInfo->pFile = 0;
		}

		if (wr != WR_OK)
			delete pWaveInfo;
	}

	return wr;
}


WAVERESULT CWaves::OpenWaveFile(void *source, WAVEID *pWaveID, wav_callbacks& callbacks)
{
	WAVERESULT wr = WR_OUTOFMEMORY;
	LPWAVEFILEINFO pWaveInfo;

	pWaveInfo = new WAVEFILEINFO;
	if (pWaveInfo)
	{
		if (SUCCEEDED(wr = ParseFile(source, pWaveInfo, callbacks)))
		{
			long lLoop = 0;
			for (lLoop = 0; lLoop < MAX_NUM_WAVEID; lLoop++)
			{
				if (!m_WaveIDs[lLoop])
				{
					m_WaveIDs[lLoop] = pWaveInfo;
					*pWaveID = lLoop;
					wr = WR_OK;
					break;
				}
			}

			if (lLoop == MAX_NUM_WAVEID)
				wr = WR_OUTOFMEMORY;
		}

		if (wr != WR_OK)
			delete pWaveInfo;
	}

	return wr;
}

WAVERESULT CWaves::ReadWaveData(WAVEID WaveID, void *pData, unsigned long ulDataSize, unsigned long *pulBytesWritten)
{
	LPWAVEFILEINFO pWaveInfo;
	WAVERESULT wr = WR_BADWAVEFILE;

	if (!pData || !pulBytesWritten || ulDataSize == 0)
		return WR_INVALIDPARAM;

	if (IsWaveID(WaveID))
	{
		pWaveInfo = m_WaveIDs[WaveID];
		if (pWaveInfo->pFile)
		{
      unsigned long ulOffset = pWaveInfo->callbacks.tell_func(pWaveInfo->pFile);

			if ((ulOffset - pWaveInfo->ulDataOffset + ulDataSize) > pWaveInfo->ulDataSize)
				ulDataSize = pWaveInfo->ulDataSize - (ulOffset - pWaveInfo->ulDataOffset);

			*pulBytesWritten = (unsigned long)pWaveInfo->callbacks.read_func(pData, 1, ulDataSize, pWaveInfo->pFile);

			wr = WR_OK;
		}
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

LPWAVEFILEINFO* CWaves::GetWaveInfo(WAVEID WaveID)
{
  if (IsWaveID(WaveID))
		return &m_WaveIDs[WaveID];
  return 0;
}

WAVERESULT CWaves::SetWaveDataOffset(WAVEID WaveID, unsigned long ulOffset)
{
	LPWAVEFILEINFO pWaveInfo;
	WAVERESULT wr = WR_INVALIDPARAM;

	if (IsWaveID(WaveID))
	{
		pWaveInfo = m_WaveIDs[WaveID];
		if (pWaveInfo->pFile)
		{
			// Seek into audio data
			pWaveInfo->callbacks.seek_func(pWaveInfo->pFile, pWaveInfo->ulDataOffset + ulOffset, SEEK_SET);
			wr = WR_OK;
		}
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

WAVERESULT CWaves::GetWaveDataOffset(WAVEID WaveID, unsigned long *pulOffset)
{
	LPWAVEFILEINFO pWaveInfo;
	WAVERESULT wr = WR_INVALIDPARAM;

	if (IsWaveID(WaveID))
	{
		pWaveInfo = m_WaveIDs[WaveID];
		if ((pWaveInfo->pFile) && (pulOffset))
		{
			// Get current position
			*pulOffset = pWaveInfo->callbacks.tell_func(pWaveInfo->pFile);
			*pulOffset -= pWaveInfo->ulDataOffset;
			wr = WR_OK;
		}
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}

WAVERESULT CWaves::ParseFile(void *source, LPWAVEFILEINFO pWaveInfo, wav_callbacks& callbacks)
{
	WAVEFILEHEADER	waveFileHeader;
	RIFFCHUNK		riffChunk;
	WAVEFMT			waveFmt;
	WAVERESULT		wr = WR_BADWAVEFILE;

	if(!source || !pWaveInfo)
		return WR_INVALIDPARAM;

	memset(pWaveInfo, 0, sizeof(WAVEFILEINFO));

  pWaveInfo->callbacks = callbacks;
  pWaveInfo->pFile = source;

	// Open the wave file for reading
	//fopen_s(&pWaveInfo->pFile, szFilename, "rb");
	//if (pWaveInfo->pFile)
	//{
		// Read Wave file header
  pWaveInfo->callbacks.read_func(&waveFileHeader, 1, sizeof(WAVEFILEHEADER), pWaveInfo->pFile);
		if (!_strnicmp(waveFileHeader.szRIFF, "RIFF", 4) && !_strnicmp(waveFileHeader.szWAVE, "WAVE", 4))
		{
			while (pWaveInfo->callbacks.read_func(&riffChunk, 1, sizeof(RIFFCHUNK), pWaveInfo->pFile) == sizeof(RIFFCHUNK))
			{
				if (!_strnicmp(riffChunk.szChunkName, "fmt ", 4))
				{
					if (riffChunk.ulChunkSize <= sizeof(WAVEFMT))
					{
						pWaveInfo->callbacks.read_func(&waveFmt, 1, riffChunk.ulChunkSize, pWaveInfo->pFile);
					
						// Determine if this is a WAVEFORMATEX or WAVEFORMATEXTENSIBLE wave file
						if (waveFmt.usFormatTag == WAVE_FORMAT_PCM)
						{
							pWaveInfo->wfType = WF_EX;
							memcpy(&pWaveInfo->wfEXT.Format, &waveFmt, sizeof(PCMWAVEFORMAT));
						}
						else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE)
						{
							pWaveInfo->wfType = WF_EXT;
							memcpy(&pWaveInfo->wfEXT, &waveFmt, sizeof(WAVEFORMATEXTENSIBLE));
						}
					}
					else
					{
						pWaveInfo->callbacks.seek_func(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
					}
				}
				else if (!_strnicmp(riffChunk.szChunkName, "data", 4))
				{
					pWaveInfo->ulDataSize = riffChunk.ulChunkSize;
					pWaveInfo->ulDataOffset = pWaveInfo->callbacks.tell_func(pWaveInfo->pFile);
					pWaveInfo->callbacks.seek_func(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
				}
				else
				{
					pWaveInfo->callbacks.seek_func(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
				}

				// Ensure that we are correctly aligned for next chunk
				if (riffChunk.ulChunkSize & 1)
					pWaveInfo->callbacks.seek_func(pWaveInfo->pFile, 1, SEEK_CUR);
			}

			if (pWaveInfo->ulDataSize && pWaveInfo->ulDataOffset && ((pWaveInfo->wfType == WF_EX) || (pWaveInfo->wfType == WF_EXT)))
				wr = WR_OK;
			else
				pWaveInfo->callbacks.close_func(pWaveInfo->pFile);
    }
	//}
	//else
	//{
	//	wr = WR_INVALIDFILENAME;
	//}

	return wr;
}


WAVERESULT CWaves::DeleteWaveFile(WAVEID WaveID)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (m_WaveIDs[WaveID]->pData)
			delete m_WaveIDs[WaveID]->pData;

		if (m_WaveIDs[WaveID]->pFile)
      m_WaveIDs[WaveID]->callbacks.close_func(m_WaveIDs[WaveID]->pFile);

		delete m_WaveIDs[WaveID];
		m_WaveIDs[WaveID] = 0;
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}


WAVERESULT CWaves::GetWaveType(WAVEID WaveID, WAVEFILETYPE *wfType)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (!wfType)
		return WR_INVALIDPARAM;

	*wfType = m_WaveIDs[WaveID]->wfType;

	return WR_OK;
}


WAVERESULT CWaves::GetWaveFormatExHeader(WAVEID WaveID, WAVEFORMATEX *wfex)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (!wfex)
		return WR_INVALIDPARAM;

	memcpy(wfex, &(m_WaveIDs[WaveID]->wfEXT.Format), sizeof(WAVEFORMATEX));

	return WR_OK;
}


WAVERESULT CWaves::GetWaveFormatExtensibleHeader(WAVEID WaveID, WAVEFORMATEXTENSIBLE *wfext)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (m_WaveIDs[WaveID]->wfType != WF_EXT)
		return WR_NOTWAVEFORMATEXTENSIBLEFORMAT;

	if (!wfext)
		return WR_INVALIDPARAM;

	memcpy(wfext, &(m_WaveIDs[WaveID]->wfEXT), sizeof(WAVEFORMATEXTENSIBLE));

	return WR_OK;
}

WAVERESULT CWaves::GetWaveData(WAVEID WaveID, void **lplpAudioData)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (!lplpAudioData)
		return WR_INVALIDPARAM;

	*lplpAudioData = m_WaveIDs[WaveID]->pData;

	return WR_OK;
}

WAVERESULT CWaves::GetWaveSize(WAVEID WaveID, unsigned long *size)
{
	if (!IsWaveID(WaveID))
		return WR_INVALIDWAVEID;

	if (!size)
		return WR_INVALIDPARAM;

	*size = m_WaveIDs[WaveID]->ulDataSize;

	return WR_OK;
}


WAVERESULT CWaves::GetWaveFrequency(WAVEID WaveID, unsigned long *pulFrequency)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (pulFrequency)
			*pulFrequency = m_WaveIDs[WaveID]->wfEXT.Format.nSamplesPerSec;
		else
			wr = WR_INVALIDPARAM;
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}
WAVERESULT CWaves::GetWaveBitsPerSample(WAVEID WaveID, unsigned short *pusBitsPerSample)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (pusBitsPerSample)
      *pusBitsPerSample = m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample;
		else
			wr = WR_INVALIDPARAM;
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}


WAVERESULT CWaves::GetWaveALBufferFormat(WAVEID WaveID, PFNALGETENUMVALUE pfnGetEnumValue, unsigned long *pulFormat)
{
	WAVERESULT wr = WR_OK;

	if (IsWaveID(WaveID))
	{
		if (pfnGetEnumValue && pulFormat)
		{
			*pulFormat = 0;

			if (m_WaveIDs[WaveID]->wfType == WF_EX)
			{
				if (m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 1)
					*pulFormat = m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16 ? pfnGetEnumValue("AL_FORMAT_MONO16") : pfnGetEnumValue("AL_FORMAT_MONO8");
				else if (m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 2)
					*pulFormat = m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16 ? pfnGetEnumValue("AL_FORMAT_STEREO16") : pfnGetEnumValue("AL_FORMAT_STEREO8");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 4) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_QUAD16");
			}
			else if (m_WaveIDs[WaveID]->wfType == WF_EXT)
			{
				if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 1) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == SPEAKER_FRONT_CENTER))
					*pulFormat = m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16 ? pfnGetEnumValue("AL_FORMAT_MONO16") : pfnGetEnumValue("AL_FORMAT_MONO8");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 2) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)))
					*pulFormat = m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16 ? pfnGetEnumValue("AL_FORMAT_STEREO16") : pfnGetEnumValue("AL_FORMAT_STEREO8");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 2) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					*pulFormat =  pfnGetEnumValue("AL_FORMAT_REAR16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 4) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_QUAD16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 6) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_51CHN16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 7) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_BACK_CENTER)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_61CHN16");
				else if ((m_WaveIDs[WaveID]->wfEXT.Format.nChannels == 8) && (m_WaveIDs[WaveID]->wfEXT.Format.wBitsPerSample == 16) && (m_WaveIDs[WaveID]->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT)))
					*pulFormat = pfnGetEnumValue("AL_FORMAT_71CHN16");
			}

			if (*pulFormat == 0)
				wr = WR_INVALIDWAVEFILETYPE;
		}
		else
		{
			wr = WR_INVALIDPARAM;
		}
	}
	else
	{
		wr = WR_INVALIDWAVEID;
	}

	return wr;
}


bool CWaves::IsWaveID(WAVEID WaveID)
{
	bool bReturn = false;

	if ((WaveID >= 0) && (WaveID < MAX_NUM_WAVEID))
	{
		if (m_WaveIDs[WaveID])
			bReturn = true;
	}

	return bReturn;
}


char *CWaves::GetErrorString(WAVERESULT wr, char *szErrorString, unsigned long nSizeOfErrorString)
{	
	switch (wr)
	{
		case WR_OK:
			strncpy_s(szErrorString, strlen(szErrorString), "Success\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDFILENAME:
			strncpy_s(szErrorString, strlen(szErrorString), "Invalid file name or file does not exist\n", nSizeOfErrorString-1);
			break;

		case WR_BADWAVEFILE:
			strncpy_s(szErrorString, strlen(szErrorString), "Invalid Wave file\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDPARAM:
			strncpy_s(szErrorString, strlen(szErrorString), "Invalid parameter passed to function\n", nSizeOfErrorString-1);
			break;

		case WR_FILEERROR:
			strncpy_s(szErrorString, strlen(szErrorString), "File I/O error\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDWAVEID:
			strncpy_s(szErrorString, strlen(szErrorString), "Invalid WAVEID\n", nSizeOfErrorString-1);
			break;

		case WR_NOTSUPPORTEDYET:
			strncpy_s(szErrorString, strlen(szErrorString), "Function not supported yet\n", nSizeOfErrorString-1);
			break;

		case WR_WAVEMUSTBEMONO:
			strncpy_s(szErrorString, strlen(szErrorString), "Input wave files must be mono\n", nSizeOfErrorString-1);
			break;

		case WR_WAVEMUSTBEWAVEFORMATPCM:
			strncpy_s(szErrorString, strlen(szErrorString), "Input wave files must be in Wave Format PCM\n", nSizeOfErrorString-1);
			break;
		
		case WR_WAVESMUSTHAVESAMEBITRESOLUTION:
			strncpy_s(szErrorString, strlen(szErrorString), "Input wave files must have the same Bit Resolution\n", nSizeOfErrorString-1);
			break;

		case WR_WAVESMUSTHAVESAMEFREQUENCY:
			strncpy_s(szErrorString, strlen(szErrorString), "Input wave files must have the same Frequency\n", nSizeOfErrorString-1);
			break;

		case WR_WAVESMUSTHAVESAMEBITRATE:
			strncpy_s(szErrorString, strlen(szErrorString), "Input wave files must have the same Bit Rate\n", nSizeOfErrorString-1);
			break;

		case WR_WAVESMUSTHAVESAMEBLOCKALIGNMENT:
			strncpy_s(szErrorString, strlen(szErrorString), "Input wave files must have the same Block Alignment\n", nSizeOfErrorString-1);
			break;

		case WR_OFFSETOUTOFDATARANGE:
			strncpy_s(szErrorString, strlen(szErrorString), "Wave files Offset is not within audio data\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDSPEAKERPOS:
			strncpy_s(szErrorString, strlen(szErrorString), "Invalid Speaker Destinations\n", nSizeOfErrorString-1);
			break;

		case WR_OUTOFMEMORY:
			strncpy_s(szErrorString, strlen(szErrorString), "Out of memory\n", nSizeOfErrorString-1);
			break;

		case WR_INVALIDWAVEFILETYPE:
			strncpy_s(szErrorString, strlen(szErrorString), "Invalid Wave File Type\n", nSizeOfErrorString-1);
			break;

		case WR_NOTWAVEFORMATEXTENSIBLEFORMAT:
			strncpy_s(szErrorString, strlen(szErrorString), "Wave file is not in WAVEFORMATEXTENSIBLE format\n", nSizeOfErrorString-1);
			break;

		default:
			strncpy_s(szErrorString, strlen(szErrorString), "Undefined error\n", nSizeOfErrorString-1);
	}
	szErrorString[nSizeOfErrorString-1] = '\0';
	return szErrorString;
}