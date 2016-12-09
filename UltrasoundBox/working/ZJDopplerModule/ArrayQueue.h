#pragma once
// ------------------------- [6/17/2015 AlexHuang]
// �����ͻ��ζ���
// ʵ���������ݵĶ���ȡ

//#define T unsigned short
template <class T>
class CArrayQueue
{
private:
	T *m_pData;
	unsigned char m_defVal;
	int m_sz;
	int m_delta;
	int m_WPos;
	long long m_Sum;
	long long m_RPos;
	CCriticalSection m_CS;
public:
	int GetCurWritePos()
	{
		return m_WPos%m_sz;
	}
	int GetLen()
	{
		if (m_Sum < m_sz)
		{
			return m_Sum;
		}
		else
			return m_sz;
	}
	//���Buffer
	void Clear()
	{
		m_CS.Lock();
		if (m_pData)
		{
			delete[] m_pData;
			m_pData = 0;
		}
		m_sz = 0;
		m_delta = 0;
		m_WPos = 0;
		m_RPos = 0;
		m_Sum = 0;
		m_CS.Unlock();
	}
	void Reset()
	{
		m_CS.Lock();
		memset(m_pData, m_defVal, (m_sz + m_delta)*sizeof(T));
		m_WPos = 0;
		m_RPos = 0;
		m_Sum = 0;
		m_CS.Unlock();
	}
	CArrayQueue()
	{
		m_pData = 0;
		m_sz = 0;
		m_delta = 0;
		m_WPos = 0;
		m_RPos = 0;
		m_Sum = 0;
	}
	~CArrayQueue()
	{
		Clear();
	}
	//����Buffer������
	//szΪBuffer��С��deltaΪ���Ķ����ݵĴ�С��defvalΪ��ʼ��Ĭ��ֵ
	//����һ��sz+delta��С��buffer���û�ʵ�ʿ�����ֻ��sz��С
	//delta�Ĵ���ֻΪ�˵���ָ����sz����ʱ�������������ڴ�����ݣ�����ƴ�ڴ�Ĳ���
	int Create(int sz, int delta, unsigned char defval)
	{
		Clear();
		m_CS.Lock();
		m_pData = new T[sz + delta];
		m_delta = delta;
		m_defVal = defval;
		m_sz = sz;
		m_CS.Unlock();
		Reset();
		return 0;
	}
	//�������
	//pData����buffer��lenΪ���ݳ���
	//�ѿ�ͷdelta��С�����ݺ�sz����delta��С���������ó�һ����
	int _Add(T* pData, int len)
	{
		m_CS.Lock();
		if (!pData || len <= 0)
		{
			m_CS.Unlock();
			return -1;
		}
		if (m_WPos + len < m_sz)
		{
			memcpy(m_pData + m_WPos, pData, len*sizeof(T));
			m_WPos += len;
			m_Sum += len;
		}
		else
		{
			if (m_WPos + len < m_sz + m_delta)
			{
				memcpy(m_pData + m_WPos, pData, len*sizeof(T));
				if (m_WPos < m_sz)
				{
					int start = m_sz - m_WPos;
					memcpy(m_pData, pData + start, (len - start)*sizeof(T));
				}
				else
				{
					int start = m_WPos - m_sz;
					memcpy(m_pData + start, pData, len*sizeof(T));
				}
				m_WPos += len;
				m_Sum += len;
			}
			else
			{
				int endlen = m_sz + m_delta - m_WPos;
				memcpy(m_pData + m_WPos, pData, endlen*sizeof(T));
				if (m_WPos < m_sz)
				{
					int start = m_sz - m_WPos;
					memcpy(m_pData, pData + start, (len - start)*sizeof(T));
				}
				else
				{
					int start = m_WPos - m_sz;
					memcpy(m_pData + start, pData, len*sizeof(T));
				}
				m_WPos += len;
				m_Sum += len;
				m_WPos %= m_sz;
			}
		}
		m_CS.Unlock();
		return 0;
	}

	int Add(T* pData, int len)
	{
		if (!pData || len <= 0)
		{
			return -1;
		}
		int ret = 0;
		if (len>m_delta)
		{
			int sum = (len / m_delta);
			for (int i = 0; i < sum; i++)
			{
				ret = _Add(pData + i*m_delta, m_delta);
			}
			ret = _Add(pData + sum*m_delta, len-sum*m_delta);
		}
		else
		{
			ret = _Add(pData, len);
		}
		return ret;
	}
	//��ȡ������delta��С��len����,���ϴζ���λ�ÿ�ʼ
	//lenΪ��ȡ���ݳ��ȣ�len<=delta������ֻ����delta��������
	//����ֵΪ���ݵ�ָ��

	T* Get(int &len)
	{
		m_CS.Lock();
		if (m_sz <= 0)
		{
			len = 0;
			m_CS.Unlock();
			return 0;
		}
		if (m_RPos + len > m_Sum)
		{
			len = m_Sum - m_RPos;
			if (len <= 255)
			{
				len = 0;
				m_CS.Unlock();
				return 0;
			}
			T* pCur = m_pData + (m_RPos%m_sz);
			m_RPos += len;
			m_CS.Unlock();
			return pCur;
		}
		T* pCur = 0;
		if (len <= m_delta)
		{
			pCur = m_pData + (m_RPos%m_sz);
			m_RPos += len;
		}
		else
		{
			len = m_delta;
			pCur = m_pData + (m_RPos%m_sz);
			m_RPos += len;
		}
		m_CS.Unlock();
		return pCur;
	}
	//��ȡ������delta��С��len����,�ӵ�ǰд��λ����ǰ��len��ʼ
	//lenΪ��ȡ���ݳ��ȣ�len<=delta������ֻ����delta��������
	//����ֵΪ���ݵ�ָ��
	T* GetLastest(int &len)
	{
		T* pCur = 0;
		m_CS.Lock();
		if (m_Sum == 0 || m_sz <= 0)
		{
			len = 0;
			m_CS.Unlock();
			return 0;
		}
		if (len > m_delta)
		{
			len = m_delta;
		}
		if (m_Sum - len < 0)
		{
			len = m_Sum;
			pCur = m_pData;
			m_CS.Unlock();
			return pCur;
		}
		else
		{
			int startpos = (m_Sum - len) % m_sz;
			pCur = m_pData + startpos;
			m_CS.Unlock();
			return pCur;
		}

	}
	T* GetFromPos(int pos, int &len)
	{
		T* pCur = 0;
		m_CS.Lock();
		if (m_Sum == 0 || m_sz <= 0)
		{
			len = 0;
			m_CS.Unlock();
			return 0;
		}
		if (pos<0 || pos >= m_sz)
		{
			len = 0;
			m_CS.Unlock();
			return 0;
		}
		if (pos + len>m_Sum)
		{
			len = m_Sum - pos;
		}
		if (pos + len > m_sz + m_delta)
		{
			len = m_sz + m_delta - pos;
		}
		pCur = m_pData + pos;
		m_CS.Unlock();
		return pCur;
	}



};