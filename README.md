varlib
======
#Introduction
varlib是一个可重用的源代码集合库,他是我在学习开源项目阅读源代码过程中,从源码里面抽取出自己感觉比较实用的部分,在平时编程中它们都非常实用而且易于移植,varlib是一个跨平台的源码集合库,里面的代码来自linux kernel，apache server，
freeBSD kernel，coreutils等等开源项目.如果varlib库无意中侵犯了你的权力请及时与我联系,我会在第一时间将对应的代码删除
varlib is a library that contain source code from many open projects.
#How to use
varlib是源代码集合库,在编程中你是否很羡慕C++ boost库，只需头文件包含无需编译就能实用，varlib内部的各个库是互相分离的,你只要将对应的头文件与对应的源文件添加进你的项目中就能使用，无需另外编译或者链接
queue.h ,arg_parser.hpp  , you just need to include "headname" it in your code.It is convinent.

arg_parser.cc,arg_parser.h , you should add the *.cc to your oject,and include the *.h

a set code have a num mask , when you use it ,you can rename it
for example the arg_parser
    [arg_parser1.1.cc arg_parser1.1.h] cpp version
    [arg_parser1.2.hpp] cpp version
    [carg_parser1.3.c carg_parser1.3.h] c version
    [cmain1.c main1.cc] just show how to use it
    [README1] show the author , licenses or other info
If I have violated some open source licenses, please contact me,I will correct it
#How to Contribution
varlib是一个开放的，乐于接受优雅艺术代码的项目，如果你在学习过程中看到你感觉到满意的，令你灵机一动，令你过目不忘的代码的话，你可以给我发email告诉我，我会考虑将它加入进varlib中

