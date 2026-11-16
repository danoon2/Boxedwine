# Third-party patches

Local patches and supporting build instructions for external projects used by
Boxedwine. Each project directory records its source revision, patch purpose,
and application instructions.

- [cnc-ddraw](cnc-ddraw/README.md): OpenGL function loading fix and DLL builder.
- [psVoodoo](psVoodoo/README.md): saturated W-depth fix and Glide regressions.

The [Wine filesystem profile](../buildWine/filesystem_wine11.json) selects the
source revisions and patches used by the filesystem builder.
