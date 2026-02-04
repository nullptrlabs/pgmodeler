v1.2.3
------
<em>Release date: February 6, 2026</em><br/>
<em>Changes since: <strong>v1.2.2</strong></em><br/>

This patch release for pgModeler 1.2.x brings the following improvements and fixes:

* [New] Added the method TaskProgressWidget::setNoProgressState.
* [Change] Refactored MainWindow initialization by moving startup timers to a new startOtherTimers method.
* [Change] Improved MainWindow initialization control by overriding the showEvent method.
* [Change] Updated AboutWidget and WelcomeWidget UI layouts.
* [Change] Updated GlobalAttributes with new site URL.
* [Fix] Fixed a bug in DatabaseModel::loadObjectsMetadata that was causing crashes in some circumstances.
* [Fix] Fixed a bug in NumberedTextEditor that was copying all text when hitting Ctrl+C when there was only a portion of the text selected.
* [Fix] Fixed a bug in ModelValidationHelper that was not properly swapping ids between table children objects and their dependencies (e.g., trigger -> function).
* [Fix] Fixed a bug in the Connection class that was creating an invalid connection string when other params were manually specified.
