# Automatic Java setup

Add App accepts a self-contained JAR through **Java App…**. **App Folder…** copies an app with its companion JARs and data; the program chooser includes JARs whose main manifest section declares `Main-Class`. Selecting or saving a JAR starts Java preparation. Opening it retries deferred setup. Java remains hidden for ordinary Windows executables.

The default **Java: Automatic** choice selects the lowest packaged runtime that meets the inspected bytecode requirement: Java 8 for requirements up to 8, Java 17 for 9–17. Requirements above 17, preview bytecode, malformed classes, missing dependencies and unsupported dependency locations get a useful error. Explicit Java 8/17 choices live under Advanced and cannot bypass an incompatible bytecode requirement. The selected exact package is retained for subsequent launches; missing or changed prepared packages are reported rather than silently substituted.

## Inspection

`JavaJar` reads the ZIP and class headers without executing the app. It parses only the main manifest section, including folded values. A `Created-By` or `Build-Jdk` value is not treated as a runtime requirement. Top-level `module-info.class` is ignored for this class-path launch. Multi-release overlays are considered only for the selected runtime; optional newer variants do not force a compatible Java 8 app to upgrade. Manifest `Class-Path` JARs must resolve inside the copied app root. Missing companion files direct the user to App Folder. Nested JARs are inspected conservatively, with limits on recursion, entry counts, data retained in memory, and total expanded bytes. The scanner checks CRCs on inspected entries, honors cancellation and rejects source changes.

JAR entries are data, never extracted as host links or executable files by the inspector. Their ZIP UNIX-mode bits are not used: the real FreeCol Maven dependencies include non-POSIX bits on directory entries. Java component ZIP extraction still applies the existing strict path/link/type/CRC checks.

This determines a conservative bytecode requirement, not complete API compatibility. Custom class loaders, optional libraries, platform APIs, dynamically obtained code and native DLLs can affect runtime behavior. Framework-specific launchers and Java preview releases are not automatically configured. Inspection does not download dependencies from manifest URLs.

## Packages and storage

The existing, bundled `filesV2.xml` supplies the Java 8/17 components. `WindowsSupport/java-packages.json` pins their HTTPS URLs to complete ZIP hashes and exact byte counts for this release. Java 8 is 57,937,992 bytes; Java 17 is 39,289,773 bytes. Known demo recipes retain Java 8 for Java Solitaire and Java 17 with `-Xmx768M` for FreeCol. Both recipes include the visible window-presentation workaround described below.

Before an uncached Java download, the native sheet names the chosen version and exact formatted download size, with **Not Now** and **Download and Prepare/Open**. Demo cards also disclose the possible additional Java download before their own download starts. Cached packages are verified again and reused; losing an available package cannot trigger an undisclosed download.

Verified component ZIPs are cached once under `JavaDownloads/<sha256>.zip`. Each app has a prepared runtime under `Applications/<id>/Java/<sha256>/runtime`, plus a receipt of its files. The runtime is mounted at `/mnt/boxedwine-java` for that launch; host Java and system-wide guest PATH settings are not modified. The prepared runtime is part of that app's storage estimate and self-contained backup. The compressed download cache is separate and retained for reuse; it is not currently part of automatic Wine package cleanup.

Preparation extracts into a new reserved staging directory, verifies all files, and publishes the complete directory without replacing an existing package. Only then does the caller save the app's package reference. Cancellation removes the current stage; an interrupted process's reserved stages are cleaned on retry. Existing app files, previous complete Java packages and metadata remain available if preparation or the metadata write fails. Completed packages are verified before reuse. Removing an app keeps its runtime in Removed Apps; permanent deletion follows the normal app-storage lifecycle.

Library format 8, recovery format 5 and backup format 4 prevent older launchers from misunderstanding Java entries. Existing non-Java entries retain their previous formats until a Java entry requires the upgrade. Backups contain the prepared runtime and receipt, so restoration can launch without downloading Java again. Apps backed up before Java was prepared still need that setup after restoration.

## Argument placement

**App arguments** are passed after the executable, or after the JAR path for Java apps. They were already app arguments in the previous UI; the label and explanation are now explicit. Each line is one argument. Spaces are retained, without a shell or surrounding quotes. A separate [Boxedwine arguments](BOXEDWINE_ARGUMENTS.md) field supplies validated runtime overrides before `/bin/wine`.

**Java arguments** are VM options before `-jar`, such as `-Xmx768M` or `-Dname=value`. The launcher supplies the selected Java executable, `-jar`, and JAR path. Launch-target/class-path switches are not accepted in this field. New Java entries start with `-Dsun.java2d.d3d.onscreen=false` visible in this field. This keeps accelerated offscreen drawing available while using ordinary window presentation.

All new Java settings receive this default, including demo recipes with their own VM options and apps changed from an EXE to a JAR. Recipes only need to list their app-specific Java arguments. Existing settings without `argumentVersion`, or with version 1, receive the visible argument when read and advance to version 2. An explicit value for `sun.java2d.d3d.onscreen` is preserved without adding a duplicate; other graphics properties no longer skip this default. After migration, a user can still edit or remove it in Advanced settings. This replaces the previous hidden `-Dsun.java2d.d3d=false` launch argument and keeps accelerated offscreen drawing available.

Java 8 and Java 17 use their normal JVM defaults, including JIT compilation. The launcher does not add a JVM execution-mode option. Explicit options in Java arguments, including `-Xint`, `-Xmixed` and `-Xcomp`, are passed through as entered.

```
Boxedwine [runtime options] /bin/wine program.exe [app arguments]
Boxedwine [runtime options] /bin/wine /mnt/boxedwine-java/bin/java.exe \
  [Java arguments] -jar C:/App/program.jar [app arguments]
```

## Verification

The default Swift suite includes detection/selection, manifests, multi-release JARs, dependencies and nested JARs, malformed/preview/linked/cancelled input, catalog fingerprints, independent prepared runtimes, cache reuse without downloads, failed/cancelled setup, interrupted staging cleanup, linked-storage rejection, library/recovery formats, backup/restore, and argument ordering with spaces and option-like app arguments. An opt-in test (`BOXEDWINE_JAVA_REAL_DOWNLOADS`) checks both actual Java ZIPs and actual demo imports without executing them. The real payloads report a bytecode minimum of Java 5 for Java Solitaire and Java 11 for FreeCol; their tested recipe choices remain Java 8 and 17.

In a separately identified, signed and sandboxed Debug app on Apple Silicon/macOS 26.4.1, direct JAR import displayed the Java 8 download size, allowed deferral, and retried preparation from App Settings. A Java 8 Swing fixture rendered and reported `-Dboxedwine.label=vm words` as one VM option and `--name`, `two words`, and `-root` as three app arguments. Java Solitaire downloaded through Demos, reused the Java 8 cache, rendered its card table, and responded to a card draw. Both exited with code 0. Their prepared runtime files still matched their receipts afterward.

FreeCol downloaded and prepared Java 17 through the native flow. App Folder also discovered both runnable fixture JARs; choosing the Java 17 fixture prepared the cached runtime automatically. Changing that selection to the Java 8 JAR automatically prepared Java 8 from cache and retained the complete Java 17 directory.

## Java 17 startup and accelerated drawing

The ARM64 startup hang was traced to a compiled `ConcurrentHashMap.fullAddCount` loop. Boxedwine's inlined locked `CMPXCHG8B` wrote ZF directly but, when no other arithmetic flags were live, left the previous lazy flag type active. Java's conditional branch could therefore retry a successful atomic update indefinitely. The ARM64 JIT now clears that stale lazy state when it writes ZF. Regression cases consume ZF after a preceding lazy comparison, for matching and mismatching values, with locked/unlocked, aligned/unaligned and cross-page accesses. Four locked/aligned cases failed before the fix; the full entry passes afterward.

On Apple Silicon/macOS 26.4.1, Wine 11.0 filesystem 11 and Zulu Java 17.0.19, the installed FreeCol 1.2 now reaches its main menu and a single-player game map with `-Xmx768M`, the visible presentation workaround, and JIT enabled. No automatic JVM execution-mode flag is injected.

The native launcher also no longer injects `-Dsun.java2d.d3d=false`. Graphics properties remain visible/editable in Advanced > Java arguments. Java 8 and Java 17 graphics fixtures selected `D3DGraphicsConfig`, reported accelerated volatile images, and passed drawing/readback checks. With `-Dsun.java2d.d3d.onscreen=false`, both also displayed the expected blue rectangle, green square, and text. Without that property, Java 17's direct onscreen presentation produced a black client area despite successful offscreen readback. That separate presentation bug remains unresolved; the workaround does not claim full Direct3D window support. Extended gameplay and every Java graphics path are not covered by these checks.

FreeCol's local single-player server requires the launcher's `com.apple.security.network.server` entitlement in addition to its existing client entitlement. The runtime inherits both from its parent; the bundle audit checks that they are present. In the regular sandboxed native app, the test confirmed a listener on `127.0.0.1:3541`, an established loopback connection, an accepted single-player login, and a rendered game map.
