![pgmodeler_mainwindow](https://github.com/user-attachments/assets/0022f022-8412-491c-9155-8b84abf837ef)

[![Linux build](https://github.com/pgmodeler/pgmodeler/workflows/Linux%20build/badge.svg)](https://github.com/pgmodeler/pgmodeler/actions?query=workflow%3A%22Linux+build%22)
[![Windows build](https://github.com/pgmodeler/pgmodeler/workflows/Windows%20build/badge.svg)](https://github.com/pgmodeler/pgmodeler/actions?query=workflow%3A%22Windows+build%22)
[![macOs build](https://github.com/pgmodeler/pgmodeler/workflows/macOs%20build/badge.svg)](https://github.com/pgmodeler/pgmodeler/actions?query=workflow%3A%22macOs+build%22)

:rocket: What's pgModeler?
------------

An **open-source, multiplatform database modeler for PostgreSQL**. This project aims to be a reference database design tool in the PostgreSQL ecosystem when it comes to FOSS. Its feature-rich interface allows for quick data modeling and fast code deployment on a server. It also supports reverse engineering by creating a visual representation of existing databases. Moreover, pgModeler can generate SQL scripts to sync a model and a database through a process called diff. 

This tool is not only about modeling; it also includes a minimalist yet functional database server administration module that allows the execution of SQL commands and provides database browsing and data handling through a simple and intuitive UI.

For more details about additional features, screenshots, and other useful information, please visit the [project's official website](https://pgmodeler.io). For feedback about the software, visit the [Issues](https://github.com/pgmodeler/pgmodeler/issues) page. Additionally, follow pgModeler on [X](https://x.com/pgmodeler), [Bluesky](https://bsky.app/profile/pgmodeler.io), or [Mastodon](https://mastodon.social/@pgmodeler) to stay up-to-date with new features, fixes, and releases.

:hammer_and_wrench: Building & Installing
----------------------

Being multiplatform software, the build process differs slightly across supported OSes. Therefore, the installation procedures are fully described in the [Installation](https://www.pgmodeler.io/support/installation) section of the project's website.

You may also want to check the [official plugin repository](https://github.com/pgmodeler/plugins) for information on extending pgModeler's features.

:spiral_notepad: Licensing
---------

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License, version 3, as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See [LICENSE](https://github.com/pgmodeler/pgmodeler/blob/main/LICENSE) for details.

:heart: pgModeler needs your support!
-------------------

* :moneybag: **Financial support:** a lot of knowledge and effort has been put into this project over the past **19 years** to deliver a reliable and constantly improved product to the community. pgModeler is an independent project with no sponsorship, relying exclusively on [donations](https://pgmodeler.io/#donationForm) and [pre-compiled package purchases](https://www.pgmodeler.io/purchase). If you like pgModeler and believe it deserves financial support, please consider contributing!

* :man_technologist: **Developers:** pgModeler has reached a point where its solo developer can no longer handle all modifications and new feature requests in a reasonable time. If you know how to code in C++ using the Qt framework and want to contribute, let me know! Any help with project maintenance will be greatly appreciated!

* :speaking_head: **Other contributors:** whether you're a developer or not, you can always help this project by spreading the word. Share this repository on your social networks, workplaces, family & friends. The more people know about pgModeler, the stronger the support for the project, creating a virtuous cycle.

:bookmark_tabs: Changelog
----------

The detailed changelog can be found in the [CHANGELOG.md](https://github.com/pgmodeler/pgmodeler/blob/main/CHANGELOG.md) file.

:card_file_box: Older Releases & Code
-------------------

Older releases of pgModeler can be found on [SourceForge.net](http://sourceforge.net/projects/pgmodeler).

:bomb: Known Issues
-----------

* Due to the usage of Qt's raster engine to draw objects, the process of handling objects on the canvas tends to get slower as lots of objects are created, causing constant CPU usage. Heavy performance degradation can be noticed when dealing with models with ~500+ tables and/or relationships. There are plans to improve the speed of drawing operations for large models in future releases. Changing the relationship connection mode and rendering smoothness options may help in the performance when handling big database models.
 
* The diff process still presents false-positive results due to its limitations. Sometimes, there is the need to run the process twice and/or tweaking the options to get the full changes.

* pgModeler does not fully support the [quoted identifier notation](http://www.postgresql.org/docs/current/static/sql-syntax-lexical.html#SQL-SYNTAX-IDENTIFIERS). The non-printable characters, some special UTF8 characters, some special characters, and upper case letters in the objects' names will automatically trigger the usage of quoted identifier notation. The following characters are considered special in pgModeler's implementation: '_-.@ $:()/<>+*\=~!#%^&|?{}[]`;

* pgModeler can't be compiled with Microsoft Visual Studio due to the use of some GCC and clang extensions.

* Compiling the source using '-Wconversion' (disabled by Qt in its defaults) generates a lot of warnings. These warnings are 99% harmless, and there are no plans to fix them in the near future.

* pgModeler can sometimes crash during the export, import, validation, or diff processes due to some threaded operations that, depending on the size and the arrangement of the model, cause race conditions between threads. 

* Due to the particularities of executing bundles on macOS, the file association does not work correctly on this system. So it's not possible to open a model file by clicking it in Finder.
