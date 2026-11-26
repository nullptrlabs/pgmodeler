v2.0.1
------
*Release date: November 27, 2026*
*Changes since:* ***v2.0.0***

This patch release for pgModeler 2.0.x brings the following improvements and fixes:

* [New] Added initial support for PostgreSQL 19, including the new `oid8` and `regdatabase` types.
* [New] Added the PDF format to the graphics file export, producing a vector file of the whole model or one page per paper sheet.
* [New] Added the `--export-to-pdf` option to `pgmodeler-cli`.
* [New] Added the selection of a range of pages to the model printing.
* [New] Added a progress window to the model printing, which can be cancelled.
* [Change] PostgreSQL 19 is now the default version.
* [Change] Updated the translations of all languages with the new texts of this release.
* [Fix] Fixed pages being skipped when printing or exporting some models, e.g. with objects at negative coordinates or with scaled pages.
* [Fix] Fixed a false warning about changed paper or margin settings when printing.
* [Fix] Fixed a bug that was removing the points of all relationships when the points of several selected relationships were removed.
* [Fix] Fixed the undo of the segments of orthogonal relationship lines moved by the user.
