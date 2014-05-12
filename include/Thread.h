#ifndef THREAD_H_
#define THREAD_H_
#include <iostream>
#include <pthread.h>
class Dictionary ;
class Thread{
	private:
		pthread_t _pth ;
		pthread_attr_t _attr ;
	public:
		Thread() ;
		virtual ~Thread() ;
		void start() ;
		static void *thread_handle(void *arg) ;
		virtual void run() = 0 ;
		pthread_t get_tid() const ;
		Dictionary *p_dictionary ; //获取独一的一份字典
};
#endif
