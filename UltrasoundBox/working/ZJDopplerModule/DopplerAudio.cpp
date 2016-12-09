#include "stdafx.h"
#include "DopplerAudio.h"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#define MAXIMUM_VOLUME 0xFFFFFFFF


//----------------------------�趨����---------------------------------------

bool GetVolumeControl(HMIXER hmixer, long componentType, long ctrlType, MIXERCONTROL* mxc)
{
	MIXERLINECONTROLS mxlc;
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(mxl);
	mxl.dwComponentType = componentType;
	if (!mixerGetLineInfo((HMIXEROBJ)hmixer, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE))
	{
		mxlc.cbStruct = sizeof(mxlc);
		mxlc.dwLineID = mxl.dwLineID;
		mxlc.dwControlType = ctrlType;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(MIXERCONTROL);
		mxlc.pamxctrl = mxc;
		if (mixerGetLineControls((HMIXEROBJ)hmixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE))
			return 0;
		else
			return 1;
	}
	return 0;
}
//---------------------------------------------------------------------------
long GetMuteValue(HMIXER hmixer, MIXERCONTROL *mxc)
{
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_BOOLEAN mxcdMute;
	mxcd.hwndOwner = 0;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = mxc->dwControlID;
	mxcd.cbDetails = sizeof(mxcdMute);
	mxcd.paDetails = &mxcdMute;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	if (mixerGetControlDetails((HMIXEROBJ)hmixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE))
		return -1;
	return mxcdMute.fValue;
}

//---------------------------------------------------------------------------
unsigned GetVolumeValue(HMIXER hmixer, MIXERCONTROL *mxc)
{
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED vol; vol.dwValue = 0;
	mxcd.hwndOwner = 0;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = mxc->dwControlID;
	mxcd.cbDetails = sizeof(vol);
	mxcd.paDetails = &vol;
	mxcd.cChannels = 1;
	if (mixerGetControlDetails((HMIXEROBJ)hmixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE))
		return -1;
	return vol.dwValue;
}

//---------------------------------------------------------------------------
bool SetMuteValue(HMIXER hmixer, MIXERCONTROL *mxc, bool mute)
{
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_BOOLEAN mxcdMute; mxcdMute.fValue = mute;
	mxcd.hwndOwner = 0;
	mxcd.dwControlID = mxc->dwControlID;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.cbDetails = sizeof(mxcdMute);
	mxcd.paDetails = &mxcdMute;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	if (mixerSetControlDetails((HMIXEROBJ)hmixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE))
		return 0;
	return 1;
}

//---------------------------------------------------------------------------

bool SetVolumeValue(HMIXER hmixer, MIXERCONTROL *mxc, long volume)
{
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED vol; vol.dwValue = volume;
	mxcd.hwndOwner = 0;
	mxcd.dwControlID = mxc->dwControlID;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.cbDetails = sizeof(vol);
	mxcd.paDetails = &vol;
	mxcd.cChannels = 1;
	if (mixerSetControlDetails((HMIXEROBJ)hmixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE))
		return 0;
	return 1;
}


//---------------------------------------------------------------------------
unsigned WINAPI GetVolume(int dev)//�õ��豸������dev=0��������1WAVE ,2MIDI ,3 LINE IN
{

	long device; unsigned rt = 0;
	MIXERCONTROL volCtrl;
	HMIXER hmixer;
	switch (dev)
	{
	case 1:
		device = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT; break;
	case 2:
		device = MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER; break;
	case 3:
		//  device=MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC; break;  // cd ����
		//  device=MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE; break;   //��˷�����
		//  device=MIXERLINE_COMPONENTTYPE_SRC_LINE; break;         //PC ����������
		device = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC; break;

	default:
		device = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	}

	if (mixerOpen(&hmixer, 0, 0, 0, 0)) return 0;
	if (!GetVolumeControl(hmixer, device, MIXERCONTROL_CONTROLTYPE_VOLUME, &volCtrl))
		return 0;
	rt = GetVolumeValue(hmixer, &volCtrl) * 100 / volCtrl.Bounds.lMaximum;
	mixerClose(hmixer);
	return rt;

}

//---------------------------------------------------------------------------

bool WINAPI SetVolume(long dev, long vol)//�����豸������
{
	// dev =0,1,2   �ֱ��ʾ������,����,MIDI ,LINE IN
	// vol=0-100     ��ʾ�����Ĵ�С , �����뷵��������ֵ�õ��ǰٷֱȣ���������0 - 100���������豸�ľ���ֵ
	// retrun false ��ʾ���������Ĵ�С�Ĳ������ɹ�
	// retrun true  ��ʾ���������Ĵ�С�Ĳ����ɹ�

	long device;
	bool rc = false;
	MIXERCONTROL volCtrl;
	HMIXER hmixer;
	switch (dev)
	{
	case 1:
		device = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT; break;
	case 2:
		device = MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER; break;
	case 3:
		device = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC; break;

	default:
		device = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	}

	if (mixerOpen(&hmixer, 0, 0, 0, 0)) return 0;

	if (GetVolumeControl(hmixer, device, MIXERCONTROL_CONTROLTYPE_VOLUME, &volCtrl))
	{
		vol = vol*volCtrl.Bounds.lMaximum / 100;
		if (SetVolumeValue(hmixer, &volCtrl, vol))
			rc = true;
	}
	mixerClose(hmixer);
	return rc;
}

//---------------------------------------------------------------------------

bool WINAPI SetMute(long dev, long vol)//�����豸����
{
	// dev =0,1,2   �ֱ��ʾ������,����,MIDI ,LINE IN
	// vol=0,1      �ֱ��ʾȡ������,���þ���
	// retrun false ��ʾȡ�������þ����������ɹ�
	// retrun true  ��ʾȡ�������þ��������ɹ�

	long device;
	bool rc = false;
	MIXERCONTROL volCtrl;
	HMIXER hmixer;
	switch (dev)
	{
	case 1:
		device = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT; break;
	case 2:
		device = MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER; break;
	case 3:
		device = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC; break;

	default:
		device = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	}

	if (mixerOpen(&hmixer, 0, 0, 0, 0)) return 0;
	if (GetVolumeControl(hmixer, device, MIXERCONTROL_CONTROLTYPE_MUTE, &volCtrl))
		if (SetMuteValue(hmixer, &volCtrl, (bool)vol))
			rc = true;
	mixerClose(hmixer);
	return rc;
}

//---------------------------------------------------------------------------

bool WINAPI GetMute(long dev)//����豸�Ƿ���
{
	//dev =0,1,2  �ֱ��ʾ������������,MIDI ,LINE IN
	// retrun false ��ʾû�о���
	// retrun true  ��ʾ����
	long device;
	bool rc = false;
	MIXERCONTROL volCtrl;
	HMIXER hmixer;
	switch (dev)
	{
	case 1:
		device = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT; break;
	case 2:
		device = MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER; break;
	case 3:
		device = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC; break;
	default:
		device = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	}

	if (mixerOpen(&hmixer, 0, 0, 0, 0)) return 0;

	if (GetVolumeControl(hmixer, device, MIXERCONTROL_CONTROLTYPE_MUTE, &volCtrl))
		rc = GetMuteValue(hmixer, &volCtrl);
	mixerClose(hmixer);
	return rc;

}






CDopplerAudio::CDopplerAudio()
{
	m_MaxValue = 100;
}


CDopplerAudio::~CDopplerAudio()
{
}

void CDopplerAudio::SetMaxValue(int maxvalue /*= 100*/)
{
	if (maxvalue>0)
	{
		m_MaxValue = maxvalue;
	}
	
}

int CDopplerAudio::SetVolume(int vol)
{
	if (vol<0)
	{
		vol = 0;
	}
	else if (vol>m_MaxValue)
	{
		vol = m_MaxValue;
	}
	int ret = waveOutSetVolume(0, 1.0* MAXIMUM_VOLUME / m_MaxValue * vol);
	if ( ret == MMSYSERR_NOERROR)
	{
		return 0;
	}
	else
		return ret;
}

int CDopplerAudio::GetVolume()
{
	unsigned int  vol = 0;
	waveOutGetVolume(0, (LPDWORD)&vol);
	vol = 1.0 * vol * m_MaxValue / MAXIMUM_VOLUME;
	return vol;
}
