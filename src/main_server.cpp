#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include "Server.h"
#include "ThreadPool.h"
#include "Conf.h"
//守护进程
void Daemon()
{
	int i=3;
	if(fork()!= 0)
	{
		exit(0);
	}
	setsid();
	chdir("/");
	umask(0);
	for(; i< MAXFD; ++i)
	{
		 close(i);
	}
}
int main(int argc , char **argv)
{
	/****************守护进程begin***************/
	Daemon();
	/****************守护进程end***************/
/*	std::ifstream fin(argv[1]);  //read configure file，这个文件的目录必须是绝对路径
    std::string ip, port ;
    fin>>ip>>port;*/

    Conf conf("server");
	std::string ip, port;
	ip = conf.get_value("ServerIp");
	port=conf.get_value("ServerPort");
	Server server(ip , port) ;  //连接服务器，并开启服务器

	//监听设置
	fd_set fd_rd, fd_rd_back;
	struct timeval tm;
	int socket_fd = server.get_socket_fd();
	FD_SET(socket_fd, &fd_rd);
	tm.tv_sec = 1;
	tm.tv_usec = 0 ;

	ThreadPool p(10) ;    //创建线程池
	p.start_thread_pool() ;   //启动线程


	std::string recv_buf;
	while(true)
	{
		std::cout<<"enter while"<<std::endl;
		fd_rd_back = fd_rd;
		select(1024, &fd_rd_back, NULL, NULL, &tm);
		//监听到客户端发送过来的请求
		if(FD_ISSET(socket_fd, &fd_rd_back))
		{
			Task task ;
			server.recv_message((void *)recv_buf.c_str() , 1024);   //接收客户端发送过来的任务消息
			task.m_clinet_addr=server.get_client_addr();   //获取客户端的ip地址和port号
			task.req_buf = recv_buf ;   //客户端发送过来的命令请求
			p.add_task_queue(task) ;   //线程池将任务添加到线程池中，有工作线程来执行任务
			sleep(1);
		}
	}
	return 0 ;
}
