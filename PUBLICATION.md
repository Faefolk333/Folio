# Publication preparation

Version: 0.4.8
Status: prerelease candidate; 0.4.7 START confirmed, HOME Close Software still hangs.
Author: Faefolk333
Suggested repository name: Folio
Public description: A customizable TXT, HTML and EPUB e-reader for New Nintendo 3DS.

## GitHub contents

The source repository includes source/, include/, assets/, tests/, reference/,
sharing/, the Makefile, current guides, changelogs and build notes. Font source
files and their notices remain together. Build outputs, extracted older releases,
ZIP files, Python caches and local publication staging are ignored.

No top-level code license has been selected by the project owner. Do not label
the project as MIT/GPL or otherwise assign a license without that choice.
The icon is owner-supplied; no third-party icon license has been inferred.

## Release assets

Tag: v0.4.8
Title: Folio 0.4.8 — Faefolk333

- folio.3dsx: direct executable for database discovery and manual installation.
- folio.smdh: standalone launcher metadata.
- folio-0.4.8-test.cia: installed test build, revision 8; HOME closing needs console retest.
- Folio-0.4.8-SD.zip: full layout for 3DSX/CIA, progress folder and font notices.
- Folio-0.4.8-source.zip: editable project snapshot.
- Folio-0.4.8-sharing.zip: descriptions, icon and explicitly labelled previews.

sharing/release-notes.md is the GitHub release body. Publication is authorized
by the project owner. Public repository: https://github.com/Faefolk333/Folio
Release: https://github.com/Faefolk333/Folio/releases/tag/v0.4.8
Universal-DB submission and acceptance are separate from publishing this release.

## Universal-DB considerations

The request page generates metadata; actual submission is a GitHub issue or PR:
https://db.universal-team.net/app-request

Reviewed rules:
https://github.com/Universal-Team/db/blob/master/CONTRIBUTING.md

The project uses substantial LLM assistance and must be declared accordingly,
not as "minor". The database's rules prohibit AI-written code/assets for the
database itself and AI-written issue/PR descriptions. The human submitter must
write their own brief request and complete/review the database metadata.
No database JSON, issue text or PR text has been generated for submission.

The direct .3dsx alone does not create /Folio/progress/. Installation through
Universal-Updater must also arrange that directory and retain font notices.
The full SD ZIP includes the required layout. Database maintainers will need
to account for its archive contents instead of treating it as a bare binary.

The existing images are host-rendered previews, not hardware screenshots.
Their caption/provenance must be preserved. No complete commercial books are
included. Authentic console captures can replace these previews later.
