/**
 * @file TimeTick.h
 * @brief calc the pc ticks
 * @author liangsijian
 * @version 
 * @date 2015-04-25
 */
#include <windows.h>

class CTimeTick
{
	CTimeTick(const CTimeTick& d);
	CTimeTick& operator=(const CTimeTick& d);

public:	
	CTimeTick();
	~CTimeTick();

	// Operations
	void Start();
	float Tick();
	bool  isPerformanceCounter() {return m_nPerformanceFrequency!=0;}

	// Implementation
protected:
	static __int64 GetPerformanceFrequency();
	static float GetTimeInMilliSeconds(__int64 nTime);

	// Data
private:
	static __int64 m_nPerformanceFrequency;

	LARGE_INTEGER m_nTimeElapsed;
	LARGE_INTEGER m_nTime;
};
