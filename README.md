ZeroAoVoiceTool
===============

A tool which can add scene voice for the PC games *The Legend of Heroes:
Zero no Kiseki* & *The Legend of Heroes: Ao no Kiseki*

**NOTE:**This projcet is licensed under the GPLv3. You MUST copy,
distribute and/or modify any code or binaries from this projcet under
this license. See [LICENSE](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/LICENSE) for details

**NOTE:**Do NOT upload any offical resources (The entire game or game
resources like voice files, Chinese or Japesene texts, scena files,
etc.) or anything modified from them.

By now, this tool can only work for:

-   *The Legend of Heroes: Zero no Kiseki* , Chinese Simplified version,
    version 1.1 (version 1.1 has 1000 save slots but version 1.0 has
    only 100)

-   *The Legend of Heroes: Ao no Kiseki*, Chinese Simplified version,
    version 1.0

To let it work, you need three things done before:

-   first, you need to extact the voice files(\*.at9) from the PS Vita
    games *Zero no Kiseki Evolution* & *Ao no Kiseki Evolution* (no
    mater they are card or downloaded games) and convert them to
    wave files(\*.wav).

-   Second, you need to make voice table files for each scena file(a
    scena file means a file under the folder "scena").This project also
    provide tools to make voice table files -- if you can get the
    decrypted scena files of the PC version games and the original scena
    files of the PS Vita version games -- and two sets of voice table
    files, one for *Zero no Kiseki* and the other for *Ao no kiseki*.

-   Third, get legal copies of the games (both PC & PS Vita version).

See
[doc/使用说明.md](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/doc/%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E.md)
for details about the usage.

External libraries used in this project
---------------------------------------

-   [SFML](http://www.sfml-dev.org/), licensed under the terms and
    conditions of the zlib/png license. External libraries used by SFML
    (and indirectly used by this project) and their licenses can be
    found in
    [/extlibs/SFML/license.txt](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/extlibs/SFML/license.txt).



--------

--------


ZeroAoVoiceTool
===============

这是个可以为PC游戏《英雄传说：零之轨迹》与《英雄传说：碧之轨迹》添加剧情语音的外挂程序

**注意：**本项目基于GPLv3开源协议，对本项目的任何代码或二进制文件的复制、修改、分发需遵循此协议。
具体细节请参见[LICENSE](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/LICENSE)文件

**注意：**请勿在此项目中上传任何官方或基于官方修改的资源（游戏本体，或其资源文件如语音文件、中日文本、脚本文件等）

目前，本工具仅适用于：

-   《英雄传说：零之轨迹》，简体中文版，版本1.1（1.1版有1000个存档位，而1.0版仅有100个）

-   《英雄传说：碧之轨迹》，简体中文版，版本1.0

为了能让其正常工作，您需要事先：

1.  导出PS Vita游戏《零の軌跡 Evolution》及《碧の軌跡
    Evolution》（卡带或下载版游戏均可）中的语音文件(\*.at9），并将其转换为wav文件。

2.  为每个scena文件（即在scena文件夹下的文件）制作一个语音表。本项目提供了制作语音表的工具
    （需要解密之后的PC版的scena文件和原始的PS
    Vita版的scena文件），以及两套（零、碧各一套）完整的语音表文件。

3.  通过合法的途径获取游戏(PC及PS Vita版)。

关于工具的详细使用说明请参考
[doc/使用说明.md](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/doc/%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E.md)

本项目使用的外部库
------------------

-   [SFML](http://www.sfml-dev.org/),
    基于zlib/png协议。SFML使用（本项目间接使用）的外部库及它们的使用协议请参考
    [/extlibs/SFML/license.txt](https://github.com/ZhenjianYang/ZeroAoVoiceTool/blob/master/extlibs/SFML/license.txt).

各目录说明
---------

-   **doc** 这里放的是项目相关文档，如工具的使用说明、原理说明等

-   **example**
    这里放置了一个配置文件样例，以及零、碧各一套的语音表文件样例

-   **extlibs** 外部库目录（源码或二进制文件）

-   **src** 放置源代码的目录

-   **solution** 用VS2015建立的工程，各项目分述如下

-   -   **ZeroAoVoiceTool** 即为前文提及的外挂语音程序

-   -   **ZaMakeVoiceTableFound**
        测试程序，用于寻找scena文件文本的特征。已无用。
        
-   -   **ZaMakeVoiceTablePrepare** 导出scena文件的文本，用于建立语音表

-   -   **ZaMakeVoiceTable** 建立语音表的工具，需要上一工具先导出文本

TODO
----

-   解决原理说明中尚未解决的问题，已解决的问题是否有更好的方案？

-   简陋的界面...

-   简陋的音频播放系统——现在，使用SFML作为音频播放系统，至少已经支持了ogg音频的播放

-   ...


