# hookQuickBuild
设置打开windows应用快捷键

配置文件config.proc中有详细说明

本项目主要运用<windows.h> ,<winuser.h>等windows库
没有任何外部库需要配置。

直接用 [visual studio 2022] 打开 .sln 文件
ctrl + b 编译即可

最后记得将配置文件 [userdefine.proc] 和 [config.proc] 
复制到 x64 文件夹下的 relase/debug 目录下
