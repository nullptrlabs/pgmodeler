<img width="1254" height="663" alt="pgModeler 2.0 Community Edition" src="https://pgmodeler.io/img/pgmodeler_2_gh_ss.png" />

## :rocket: Why pgModeler?

**pgModeler** is a PostgreSQL-first, multiplatform database modeling platform designed specifically for the **PostgreSQL** ecosystem. Built with a strong focus on correctness, productivity, and PostgreSQL feature parity, pgModeler aims to be the definitive open-source reference for visual database design and schema engineering.

Unlike generic modeling tools, pgModeler fully embraces PostgreSQL’s advanced capabilities, ensuring that your visual models translate accurately into production-ready database schemas. Whether you are designing a new system from scratch or maintaining a large and complex infrastructure, pgModeler provides the flexibility and precision required by modern PostgreSQL workflows.

The platform is distributed in two editions:

### pgModeler Community (Open Source)

The Community Edition is free and open source, focused on visual modeling, PostgreSQL schema design, SQL generation, and educational or development workflows.

#### Key capabilities:
* **Visual modeling:** Create and edit complex relational schemas through an intuitive drag-and-drop interface. Manage tables, views, relationships, functions, operators, and many other PostgreSQL-specific objects.
* **Schema generation:** Generate high-quality SQL scripts directly from your visual models while preserving PostgreSQL compatibility and advanced object definitions.
* **Validation & consistency tools:** Detect modeling inconsistencies, broken references, and dependency issues before deployment.
* **Cross-platform native experience:** Built with **C++ and Qt**, delivering native performance on Linux, Windows, and macOS.

The Community Edition remains the foundation of the pgModeler platform and continues receiving active maintenance, PostgreSQL compatibility updates, performance improvements, bug fixes, and usability enhancements.

### pgModeler Plus (Commercial Edition)

**pgModeler Plus** extends the Community Edition with advanced database lifecycle management capabilities designed for professional and enterprise environments.

#### Exclusive Plus features:
* **Reverse engineering:** Connect to existing PostgreSQL instances and generate complete visual representations of live databases.
* **Database Diff & Synchronization:** Compare models against databases or other model files and generate precise synchronization scripts automatically.
* **Integrated database management:** Execute SQL commands, browse data, and manage database objects through a streamlined administration interface.
* **Professional workflows:** Advanced productivity resources designed for teams, consultants, DBAs, and large-scale PostgreSQL deployments.

Revenue generated through pgModeler Plus directly funds the continuous development and long-term sustainability of the project.

For detailed feature comparisons, screenshots, and tutorials, visit [pgmodeler.io](https://pgmodeler.io).

---

## :hammer_and_wrench: Build & installation

As a multiplatform project, pgModeler supports Linux, Windows, and macOS. The build process is optimized for each environment to ensure native performance and consistent behavior across platforms.

You can find detailed, step-by-step guides for compiling from source in our [installation section](https://www.pgmodeler.io/support/installation).

Want to add custom functionality? Explore the [official plugin repository](https://github.com/pgmodeler/plugins) to learn how plugins can extend the platform capabilities.

---

## :heart: Support & professional services

For nearly two decades, pgModeler has evolved through continuous community feedback and independent development. Today, the project is officially backed by **Nullptr Labs**, focused on ensuring long-term sustainability, professional reliability, and continuous innovation around the PostgreSQL ecosystem.

### Ways to support the project:
* **Use pgModeler Plus:** By purchasing a **Plus** license on our [official website](https://www.pgmodeler.io/purchase), you gain access to advanced lifecycle management features while directly funding the development of the platform.
* **Official pre-compiled packages:** Save time and ensure a stable environment with professionally packaged binaries for Linux, Windows, and macOS.
* **Contribute code:** We are always looking for talented **C++ and Qt framework** developers. If you want to help improve performance, fix bugs, or implement new features, your contributions are highly appreciated.
* **Spread the word:** Share pgModeler with your colleagues, teams, and PostgreSQL communities to help strengthen the ecosystem around the project.

---

## :bomb: Technical notes (Known issues)

* **Large-scale models:** Due to the Qt raster rendering engine, very large models containing hundreds of tables may experience rendering slowdowns depending on hardware and relationship complexity.
* **Quoted identifiers:** Special characters, uppercase names, or reserved keywords automatically trigger quoted identifier generation to preserve SQL standard compatibility.
* **Windows compilation:** The source code currently relies on GCC/Clang-specific extensions and is not compatible with Microsoft Visual Studio (MSVC) compilers.
* **Thread-intensive operations:** Extremely large exports, validations, or model processing operations may still expose race conditions in specific scenarios. Improvements in thread safety are continuously being implemented.

---

### :bookmark_tabs: Licensing

The pgModeler Community Edition is free software distributed under the terms of the **GNU General Public License v3 (GPLv3)**.

See the [LICENSE](https://github.com/nullptrlabs/pgmodeler/blob/main/LICENSE) file for complete details.

The names **pgModeler**, **Nullptr Labs**, and their associated logos and visual identities are registered trademarks and intellectual property of Nullptr Labs. Forks and redistributed versions derived from the open-source codebase must adopt distinct branding and may not present themselves as official pgModeler distributions without prior authorization.
