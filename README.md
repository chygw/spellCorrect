spellCorrect
============
# 文本纠错项目总结

标签（空格分隔）： 项目描述

---
#1.项目要求

> 该项目是一个文本纠错的程序，采用UTP协议的网络通信方式，客户端向服务器发送请求，服务器分析请求，以返回给客户正确的请求，纠正发送方所发送的不准确请求。

#2.项目知识点

> UTP协议；socket网络通信；守护进程；互斥锁；条件变量；多线程编程；字符串处理；最短编辑距离算法；vector，pair，map，set，hash_map等STL容器类的使用；文件操作；GBK与UTF8编码；Makefile的编写；shell脚本语言；日志文件，配置文件的使用。

#3.项目详细设计
##3.1 项目概述

 - 采用客户端服务器C/S的方式，客户端负责向服务器发送请求，并接收返回结果，服务器负责处理客户端请求，并将处理结果返回。
 - 服务器端的任务执行主要是使用线程池，让线程来处理任务，减少了服务器的负担。项目中的分词算法是采用开源代码jieba直接在该开源代码中完成分词。项目采用的策略是将自己分词所得到的字典读入到内存中，与客户的搜索词计算最短编辑距离，返回的是与搜索词具有最短的编辑距离的词。在考虑提高项目的时间与空间的效率问题中，采用cache缓存和索引的策略。使用的cache缓存来记录客户的查询记录，从而直接返回结果；使用的索引，减少了搜索词与词典词的计算量和搜索范围，提高了速度。

##3.2代码介绍

> src/Server.cpp

 - 服务器类，服务器采用UDP的网络协议，将socket网络通信中的过程封装成类，简化程序设计的复杂化。

> src/Conf.cpp

 - 配置类，项目中涉及的读写文件路径通过conf类来执行读写文件中的路径，实现项目的软编码。
src/MutexLock.cpp
 - 将互斥锁进行封装成为类，主要成员函数是初始化锁，加锁，解锁，销毁锁。在该类中加入了智能指针的使用，用于对加锁，解锁进行再一次的封装，从而在使用锁的地方直接可以使用该智能指针，防止在使用加锁的地方由于程序出现问题而不能释放锁的问题。

> src/Condition.cpp

 - 将对条件变量操作的组成为一个类，包括对条件变量的初始化，销毁，等待，通知等操作，完成线程之间的同步操作。

> src/Thread.cpp

 - 该类为线程的抽象类，执行线程的创建。线程创建的第三个参数是一个函数指针，是线程需要完成的任务。因为线程所要完成的任务不同，因此将其的任务执行方法定义为虚函数，通过继承来实现这个run方法的重写。

> src/WorkThread.cpp

 - 工作线程类，继承Thread类，根据线程需要完车任务的需要，重写了类中的run方法，实现了多态。

> src/StringUtil.cpp

 - 该类主要是封装了对字符串相关处理的函数，主要是字符串的大小写转换，去除标点符号，以及两个字符串之间的最短编辑距离的计算。在计算这个编辑距离的问题上，需要考虑中文在内存中存储的问题，因为中文占两个字节，而ASCII码只在内存中占一个字节，要使用一个通用的计算算法，就需要将汉字和ASCII统一为同一种类型。
 - 本项目是使用uint16_t的数据类型，内存在识别字符串的过程是一个字符一个字符读取的，因此需要判断第一个字符的最高位是0还是1，采用与0x80进行&操作，若是大于128，则表示该字节是汉字的第一个字节，需要往后在读一个字节，然后将两个字节拼接存储在一起，否则，则是一个ASCII码字符，直接存储该当前字符即可。

> src/EncodingConverter.cpp

 - 编码转换类，实现UTF8与GBK之间编码的转换，这里主要是直接使用库函数提供的iconv函数，实现两种编码之间的转换。

> src/Dictionary.cpp

 - 字典类，为服务器提供唯一字典，以与搜索词进行匹配。因此将该类定义为单例模式，防止重复初始化。Dictionary类作为Thread类中的一个成员变量，在Thread开启的时候就执行该类中的create_dictionary方法。类中维持了一个存储词和词频的pair对的数组，将词典中的词读入到内存中，同时还有一个map成员变量，为每一词建立索引。
 - 在建索引的过程中主要是要考虑我的数据存储形式，如何更好的建索引及更快的访问索引所对应的内容。数据存储形式：

```
vector<pair<string, size_t> >word_vec;	//用于存储词和词频，通过下标可以直接访问相对应的词与词频
map<string, set<size_t> >m_word ;//为词典中的每一个词建立索引，只要包含该词，就将该词在vector中的下标值存储进来，用set防止了重复下标的存储，使用这个下标就直接访问到相应的词。
```

> src/CacheQuery.cpp

 - cache缓存类，用来将服务器曾经处理过的结果存储在hash_map中，这样的查找速度是非常的快。Task类中执行的查询匹配操作，可以先查询cache缓存，若不存在再去访问内存中的索引表。这个CacheQuery类维持一个cache文件，存储在磁盘中，通过读cache文件将缓存中的内容读入到cache内存中的hash_map中，从而，可以直接将搜索词与匹配的结果值查询出来。

> src/CacheThread.cpp

 - cache线程类，继承类Thread类，这个cache线程类只作一件事，就是定时的更新磁盘中的cache文件。

> src/Task.cpp

 - 任务类，由该类来访问词典，通过为搜索词的每一个字符建立索引，然后将所获得的所有词的索引合并到一起，在这个合并的索引中计算最短编辑距离，在类中的查询方法中提供了一种策略，就是先访问cache缓存中的内存，若存在搜索词，则直接将纠错结果返回，否则，在索引表中查询并计算，同时将这个新的搜索词添加到cache缓存中。

> src/ThreadPool.cpp

 - 线程池类，该类的成员变量包括，WorkThread数组，Task任务队列，CacheThread类对象，锁，条件变量，线程词是否开启的判断变量。
 - 服务器接收客户端发送的任务请求，线程池将该任务放入到任务队列中，每个WorkThread从任务队列中取出任务进行处理，WorkThread处理得到的结果直接返回给客户端，而无须在经过服务器返回。
 - 每个WorkThread类都有一份自己的cache缓存，但CacheThread需要定时的更新cache磁盘文件，因此，需要将WorkThread的CacheQuery类作为指针注册到CacheThread线程类中，从而将每个内存cache缓存写入到cache磁盘中。

