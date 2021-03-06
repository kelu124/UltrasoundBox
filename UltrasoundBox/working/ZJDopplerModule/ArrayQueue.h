#pragma once
// ------------------------- [6/17/2015 AlexHuang]
// 数组型环形队列
// 实现连续数据的读和取

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
	//清除Buffer
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
	//创建Buffer环队列
	//sz为Buffer大小，delta为最大的读数据的大小，defval为初始化默认值
	//创建一个sz+delta大小的buffer，用户实际看到的只有sz大小
	//delta的存在只为了当读指针在sz附近时，能连续读到内存的数据，减少拼内存的步骤
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
	//添加数据
	//pData数据buffer，len为数据长度
	//把开头delta大小的数据和sz后面delta大小的数据设置成一样的
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
	//读取不大于delta大小的len数据,从上次读的位置开始
	//len为读取数据长度，len<=delta，否则只返回delta长度数据
	//返回值为数据的指针

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
	//读取不大于delta大小的len数据,从当前写的位置往前推len开始
	//len为读取数据长度，len<=delta，否则只返回delta长度数据
	//返回值为数据的指针
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