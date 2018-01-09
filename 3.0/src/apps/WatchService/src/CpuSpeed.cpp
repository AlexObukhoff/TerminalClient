/* @file Реализация грубого подсчета скорости процессора. */

#include <windows.h>

#ifndef _MSC_VER
#error "CPUSpeed() not implemented for non MSVS compilers."
#else

//----------------------------------------------------------------------------
__inline __int64 rdtsc()
{
	__asm rdtsc
}

//----------------------------------------------------------------------------
unsigned CPUSpeed()
{
	static unsigned speed = 0;

	if (speed == 0)
	{
		__int64 frequency;
		__int64 now, end;
		__int64 counter;

		QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
		QueryPerformanceCounter((PLARGE_INTEGER)&end);
		end += frequency;
		counter = rdtsc();
		do
		{
			QueryPerformanceCounter((PLARGE_INTEGER)&now);
		} while (now < end);

		counter = rdtsc() - counter;
		speed = unsigned(counter / 1000000);
	}

	return speed;
}
#endif

//----------------------------------------------------------------------------
