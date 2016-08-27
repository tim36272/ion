#ifndef ION_THREAD_H_
#define ION_THREAD_H_
namespace ion
{
	typedef void(*thread_ptr)(void*);
	void StartThread(const char* name, thread_ptr);
}
#endif //ION_THREAD_H_