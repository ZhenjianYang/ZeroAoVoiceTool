ZeroAoVoiceTool
===============

A tool which can add scene voice for PC games *The Legend of Heroes:
Zero no Kiseki* & *The Legend of Heroes: Ao no Kiseki*

**NOTE:**This projcet is licensed under the GPLv3. You MUST copy,
distribute and/or modify any code or binaries from this projcet under
this license. See
[LICENSE](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/LICENSE)
for details

**NOTE:**Do NOT upload any offical resources (The entire game or game
resources like voice files, Chinese or Japesene texts, scena files,
etc.) or anything modified from them.

By now, this tool can only work for:

-   *The Legend of Heroes: Zero no Kiseki* , Chinese Simplified,
    1.0, 1.1 & JOYO Platform Edition

-   *The Legend of Heroes: Ao no Kiseki*, Chinese Simplified,
    1.0 & JOYO Platform Edition

To let it work, you need these things done before:

-   Extact the voice files(\*.at9) from the PS Vita games
    *Zero no Kiseki Evolution* & *Ao no Kiseki Evolution* (no
    mater they are card or downloaded games) and convert them to
    supported files(\*.wav or \*.ogg).

-   Get legal copies of the games (both PC & PS Vita games).

See
[doc/使用说明.md](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/doc/%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E.md)
for details about the usage.

External libraries used in this project
---------------------------------------

-   [Audiere](http://audiere.sourceforge.net/), licensed under the
    [LGPL](http://opensource.org/licenses/lgpl-license.html).
    
-   [Windows Template Library (WTL)](http://wtl.sourceforge.net/), licensed under the
    [Microsoft Public License (MS-PL)](https://opensource.org/licenses/MS-PL).
	
-   [ini-parser](https://github.com/Poordeveloper/ini-parser), licensed under the
    [MIT](https://tldrlegal.com/license/mit-license) license.

------------------------------------------------------------------------

------------------------------------------------------------------------

ZeroAoVoiceTool
===============

这是个可以为PC游戏《英雄传说：零之轨迹》与《英雄传说：碧之轨迹》添加剧情语音的外挂程序

**注意：**本项目基于GPLv3开源协议，对本项目的任何代码或二进制文件的复制、修改、分发需遵循此协议。
具体细节请参见[LICENSE](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/LICENSE)文件

**注意：**请勿在此项目中上传任何官方或基于官方修改的资源（游戏本体，或其资源文件如语音文件、中日文本、脚本文件等）

目前，本工具仅适用于：

-   《英雄传说：零之轨迹》，简体中文版，1.0、1.1及JOYO平台版

-   《英雄传说：碧之轨迹》，简体中文版，1.0及JOYO平台版

为了能让其正常工作，您需要事先：

1.  导出PS Vita游戏《零の軌跡 Evolution》及《碧の軌跡 Evolution》（卡带或下载版游戏均可）
    中的语音文件(\*.at9），并将其转换为本工具支持的文件格式(\*.wav或\*.ogg)。

2.  通过合法的途径获取游戏(PC及PS Vita版)。

关于工具的详细使用说明请参考
[doc/使用说明.md](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/doc/%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E.md)

本项目使用的外部库
------------------

-   [Audiere](http://audiere.sourceforge.net/),
    基于[LGPL协议](http://opensource.org/licenses/lgpl-license.html).
    
-   [Windows Template Library (WTL)](http://wtl.sourceforge.net/), 基于
    [Microsoft Public License (MS-PL)](https://opensource.org/licenses/MS-PL).
    
-   [ini-parser](https://github.com/Poordeveloper/ini-parser), 基于
    [MIT](https://tldrlegal.com/license/mit-license) 协议.

各目录说明
----------

-   **doc** 这里放的是项目相关文档，如工具的使用说明、原理说明等

-   **example** 这里放置配置文件及语音表的样例

-   **extlibs** 外部库目录（源码或二进制文件）

-   **src** 放置源代码的目录

-   **solution** 用VS2015建立的工程，各项目分述如下

-   -   **ZaBase** 静态库，实现语音工具的基础逻辑部分

-   -   **ZeroAoVoiceTool** 窗口程序，实现语音工具的界面部分

-   -   **ZaMakeVoiceTableFound** 测试程序，用于寻找scena文件文本的特征。已无用。
        
-   -   **ZaMakeVoiceTablePrepare** 导出scena文件的文本，用于建立语音表

-   -   **ZaMakeVoiceTable** 建立语音表的工具，需要上一工具先导出文本

TODO
----

-   [doc/解决原理说明](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/doc/%E5%8E%9F%E7%90%86%26%E9%97%AE%E9%A2%98%26%E8%A7%A3%E5%86%B3.md)中还有尚未解决的问题


