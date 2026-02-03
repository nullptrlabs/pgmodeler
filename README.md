![pgmodeler_mainwindow](https://github.com/user-attachments/assets/0022f022-8412-491c-9155-8b84abf837ef)

[![Linux build](https://github.com/pgmodeler/pgmodeler/workflows/Linux%20build/badge.svg)](https://github.com/pgmodeler/pgmodeler/actions?query=workflow%3A%22Linux+build%22)
[![Windows build](https://github.com/pgmodeler/pgmodeler/workflows/Windows%20build/badge.svg)](https://github.com/pgmodeler/pgmodeler/actions?query=workflow%3A%22Windows+build%22)
[![macOs build](https://github.com/pgmodeler/pgmodeler/workflows/macOs%20build/badge.svg)](https://github.com/pgmodeler/pgmodeler/actions?query=workflow%3A%22macOs+build%22)

---

## :rocket: Why pgModeler?

**pgModeler** is an open-source, multiplatform database modeling tool built specifically for the **PostgreSQL** ecosystem. This project aims to be the definitive reference for database design in the FOSS world, offering a feature-rich interface that allows for rapid data modeling and seamless code deployment. 

Unlike generic tools, pgModeler is tailored to PostgreSQL's specific strengths, ensuring that your visual designs translate perfectly into high-performance database schemas. Whether you are starting a new project from scratch or maintaining a massive legacy system, pgModeler provides the agility and precision required by modern development workflows.

### Key capabilities:
* **Visual modeling:** Create and edit complex relational schemas through an intuitive drag-and-drop interface. Manage tables, views, functions, operators, and more with specialized tools for every PostgreSQL object.
* **Robust reverse engineering:** Connect to existing database instances and generate comprehensive visual representations in seconds. This allows you to document, audit, or modify live databases without writing a single line of DDL manually.
* **Smart synchronization (diff):** Compare your visual model against a live database or another model file. pgModeler generates the precise SQL scripts needed to synchronize both, handling dependencies and structural changes automatically.
* **Integrated management module:** Beyond design, the tool includes a functional database administration module. Execute custom SQL commands, browse table data, and manage server objects through a clean, minimalist UI.

For deep dives into features, screenshots, and tutorials, visit [pgmodeler.io](https://pgmodeler.io).

---

## :hammer_and_wrench: Build & installation

As a multiplatform project, pgModeler supports Linux, Windows, and macOS. The build process is optimized for each environment to ensure native performance. You can find detailed, step-by-step guides for compiling from source in our [installation section](https://www.pgmodeler.io/support/installation).

Want to add custom functionality? Explore the [official plugin repository](https://github.com/pgmodeler/plugins) to see how you can extend the core features of pgModeler.

---

## :heart: Support & professional services

For 19 years, pgModeler has thrived as an independent project driven by community feedback. To ensure long-term sustainability and faster development cycles, the project is now officially backed by **Nullptr Labs**, a startup founded to bridge the gap between open-source passion and professional reliability.

### How to power your workflow:
* **pgModeler Plus:** Get access to exclusive, advanced features designed for power users and enterprise environments. By purchasing a **Plus** subscription on our [official website](https://www.pgmodeler.io/purchase), you unlock professional capabilities while directly funding the core development of the software.
* **Pre-compiled packages:** While the source code is open, you can save significant time and ensure a stable environment by purchasing our official binaries. These packages come with dedicated professional support and easy installers for all major platforms.
* **Contribute code:** We are always looking for talented **C++ and Qt framework** developers. If you want to help us fix bugs, optimize performance, or build the next big feature, your contributions are highly appreciated.
* **Spread the word:** As a growing startup, our strongest asset is our community. Share pgModeler with your colleagues, your team, and on social media to help us build a stronger support ecosystem.

---

## :bomb: Technical notes (Known issues)

* **Large-scale models:** Due to the Qt raster engine used for rendering, performance may degrade in models with 500+ tables. Changing relationship connection modes or rendering smoothness in the options can help mitigate this.
* **Diff precision:** In certain edge cases, the diff process may return false-positive results. We recommend reviewing the generated scripts or running the process a second time to ensure absolute accuracy.
* **Quoted identifiers:** Special characters, uppercase letters, or non-printable characters in object names will automatically trigger the usage of quoted identifier notation to remain compliant with SQL standards.
* **Windows compilation:** The source code currently utilizes specific GCC/Clang extensions, making it incompatible with Microsoft Visual Studio (MSVC) compilers.
* **Thread stability:** In extremely complex or large models, heavy threaded operations like exports or validations may occasionally trigger race conditions. We are actively working on improving thread safety in these modules.

---

### :bookmark_tabs: Licensing
This program is free software: you can redistribute it and/or modify it under the terms of the **GNU General Public License, version 3**. See the [LICENSE](https://github.com/pgmodeler/pgmodeler/blob/main/LICENSE) file for complete details.

---

### :office: About Nullptr Labs
**Nullptr Labs Software e Tecnologia LTDA** was founded in early 2026 to provide a stable corporate structure for the pgModeler project. Our goal is to continue the legacy of open-source innovation while delivering the support and professional tools that modern businesses require.
