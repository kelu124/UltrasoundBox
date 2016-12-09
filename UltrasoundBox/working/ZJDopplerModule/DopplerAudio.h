#pragma once
class CDopplerAudio
{
public:
	CDopplerAudio();
	~CDopplerAudio();
	void SetMaxValue(int maxvalue = 100);
	int SetVolume(int vol);
	int GetVolume();
private:
	int m_MaxValue;
};

