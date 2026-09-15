# Licensing and distribution review

September 14, 2026. **Initial inventory complete; distribution clearance remains pending.** This records evidence and concrete release work, not a legal opinion or a change to anyone's license. The [App Store feature review](APP_STORE_SCOPE.md) is a separate decision: Apple's acceptance of the product would not settle third-party copyright obligations.

The initial inventory covers the inspected Mac app, seven Wine filesystems, both then-offered Java packages, and the original 36-entry catalog. Dedicated Java support and its two catalog entries have since been removed; the active catalog has 34 entries. Exact hashes, evidence paths, qualifications, and unresolved source mappings are in [Licensing/inventory.json](Licensing/inventory.json). The [demo permissions ledger](Licensing/demo-rights.csv) tracks payload and artwork rights separately. It contains no recipes, icons, or usable catalog fallback.

## What needs resolution first

| Item | Evidence | Required outcome |
| --- | --- | --- |
| App Store terms and GPL/LGPL | Boxedwine headers permit GPL version 2 or later; Wine and Java retain their own licenses | Review the actual developer agreement, customer terms, intended EULA, signing/replacement restrictions, and source delivery with an open-source licensing specialist. Record the applicable license version and whether any additional permission is necessary. |
| Authority to grant additional permissions | Maintainer confirmed five Git aliases; other contributors' code remains | Establish rights for affected contributions before proposing an exception or alternative license. Existing GPL distribution permission is distinct from authority to relicense another contributor's work. |
| psVoodoo SDK headers | Published Wine 11 package includes replacements expressly distributed under the separate 3dfx Glide license, with matching source and notices | Review the combined distribution terms. The source/binary release is verified and pinned; see the follow-up below. |
| Complete corresponding source | Nine exact downloaded packages identified; no complete matching source-delivery set established | Retain source, local changes, required build/install scripts, license texts, and immutable source-to-binary records for every applicable component. |
| Demo payloads and icons | 36 catalog entries; no permission records collected in this audit | Review each exact payload's terms and each icon's origin. Include only entries with an established distribution basis in a Store catalog. |

## Inspected release and limits

The inspected artifact is `project/mac-xcode/bin/Boxedwine.app`, version **26.1.0**, build **1**, from the September 13 Release archive. It contains the UI, embedded engine, SDL, hidapi and MoltenVK: five Mach-O images, with hashes recorded in the inventory. It does **not** include Wine, consistent with the Jenkins build. The proposed Store payload is the separately inspected Wine 11 ZIP.

The local Mac dependency archive marker is **3**, while `fetchDepends.sh` currently requests **4**. Therefore this inventory does not establish the identities of everything a fresh Jenkins build will obtain. Repeat the inventory against the final Store archive and preserve that dependency archive's hash and provenance. No dependency version was changed during this review.

Current arm64 compiler dependency files and the runtime's 381 linked object entries identify compiled vendor code. They are evidence for source inclusion, not a complete transitive license scan of every source file. Guest ZIPs were inspected without executing their contents; all nine files matched the app's recorded byte counts and SHA-256 values. Conventional notice filenames, version metadata, font names, and selected build/source records were inspected. A filename scan can miss notices elsewhere and cannot establish compliance by itself.

## Boxedwine ownership and the Store terms

The maintainer confirmed that **James Bryant, James, james, danoon2 and boxedwine** are all their Git author identities. Reachable history also contains kevodwyer, kevin odwyer, Ribbon, Michael R. Crusoe and rwv. This is an attribution inventory, not a finding that every historical author has relevant surviving copyright. Conversely, `git blame` attributes **214 current lines in `source/emulation/cpu/normal/normalCPU.cpp` to kevodwyer**, so contributed code is not merely a historical possibility. Author names alone do not establish assignments, employer rights, or legal identity.

No assignment, CLA or Store-specific permission was found in the bounded repository document search. The maintainer's answer about any external agreements is still pending. Determine which affected code remains, who owns it, and whether an agreement actually permits the proposed additional grant. Do not assume the maintainer can change all code to a different license.

Charging for a GPL-covered distribution is allowed by the GPL; the unresolved question is the conditions attached to that distribution, not its price. A free GitHub build is useful for compatibility testing but does not cure conflicting conditions on the Store copy. See the repository's [GPL text](../../../../license.txt), particularly sections 3 and 6.

Apple's current [standard EULA](https://www.apple.com/legal/internet-services/itunes/dev/stdeula/) contains transfer/redistribution limits and an open-source qualification within its copying/modification clause. That qualification should not be treated as a complete answer for every restriction. Apple's [Media Services terms](https://www.apple.com/legal/internet-services/itunes/) apply additional usage rules. Apple supports [custom EULAs](https://developer.apple.com/help/app-store-connect/manage-app-information/provide-a-custom-license-agreement), but publishing one is not evidence that all other applicable conditions disappear.

Give the licensing reviewer this inventory, the intended commercial terms, and the actual agreement accepted by the developer account. Ask for a component-specific assessment of GPL version selection, additional restrictions, corresponding source, and LGPL modification/replacement rights. In particular, examine Apache-2.0 MoltenVK together with the GPL-2.0-or-later engine; do not assume an unqualified GPLv2-only combination. The copied LLVM helpers have a separate LLVM exception that also needs to be retained and assessed. This review has not selected a different GPL version or drafted an exception.

Keep the separate helper process for its technical benefits. Moving Wine into or out of that process does not, by itself, remove distribution obligations for the files we supply. Likewise, making Wine a download instead of a bundle resource is not a substitute for complying with its licenses.

## Native components and notices

| Component | Identified license/version evidence | Remaining work |
| --- | --- | --- |
| Boxedwine engine and UI | GPL-2.0-or-later headers; root GPLv2 text bundled | Exact source release, notices, contributor and Store-term review |
| SDL | 2.0.14 from framework metadata; zlib license bundled | Match the prebuilt framework to source, build flags, modifications, and bundled dependencies |
| hidapi | Bundled BSD-style, original permissive, and GPLv3 alternatives | Identify exact source revision; record an applicable permissive choice, not a mandatory GPLv3 classification |
| MoltenVK | Upstream Apache-2.0; binary contains `1.2.11` | Confirm version/revision and archive origin; supply its full license and applicable third-party notices, including compiled dependencies |
| AsmJit | 1.21.0, zlib | Preserve vendored source and changes |
| GLEW | 2.1.0; BSD-style and MIT-style notices | Include source-header credits as well as standalone license; they differ in dates/authors |
| pugixml | 1.10, MIT | Include notice |
| SoftFloat | Release 3e, BSD-3-Clause | Include binary-distribution notice |
| zlib and MiniZip | 1.2.11 and 1.1, zlib | Preserve contributor/modification notices and source |
| stb_image | 2.25, MIT or public-domain alternative | Preserve an applicable license and attribution |
| Copied LLVM helpers | Apache-2.0 with LLVM exception in `llvm_helper.cpp` | Record upstream origin and include full license/exception |
| Notepad artwork | Wine 11, LGPL-2.1-or-later | Original ICO, PNG, provenance and license already included; retain them in release materials |

The bundle's readily identifiable license documents cover Boxedwine, SDL, hidapi and the Notepad icon. That is not a complete notices set for the components above. [MoltenVK's upstream repository](https://github.com/KhronosGroup/MoltenVK) identifies its Apache license, but an upstream page is not proof of the exact bundled binary's complete inputs.

Removing software Mesa and its host LLVM libraries reduced the binary inventory. It did not remove the copied LLVM helpers, GLEW's Mesa-derived notices, guest GLU, or all other guest graphics dependencies. Repository-only libraries were not automatically classified as shipping simply because their directories exist.

## Wine filesystem and custom graphics libraries

The initially inspected Wine 11 filesystem was **167,016,911 bytes**, SHA-256 **`a3367c4e977dbbe0295ce3b4b102bf7b0baa1278ea121cefe6065d561b960452`**, from `https://boxedwine.org/v2/11/TinyCore15Wine11.0.zip`. The current published package is **167,173,559 bytes**, SHA-256 **`08cf3f51db0b0d00d9895763cd51a13ee6eaddd0e0a7bfb1f93cdbf2463cb079`**; the follow-up below records its changes and verification. The six alternate Wine packages also matched the app's pinned identities; each needs its own source/notice mapping while offered for download.

The ZIP identifies TinyCore 15 and Wine 11.0. Its `build.txt` names `wine-11.0` and two patches already present under `tools/buildWine/patches`: `fixSetupApiFromCrashingDuringDllDetach.patch` and `FAudio_opentdd_mac_crash.patch`. This is useful provenance, but not a complete record of the compiler, environment, base image, or subsequent graphics replacements. The newer [Wine builder](../../../../tools/buildWine/README.md) provides a better mechanism; its current inputs must not be represented as proof that it produced the already inspected ZIP.

`installed.txt` names **60 unique TinyCore extensions**, including compression, crypto, font, media and utility packages. This is installation history, not a reliable final package inventory: the recreation instructions delete some files. Base components such as BusyBox, glibc and filesystem utilities add further obligations. `packages.txt` is a repository availability list; `dlls.txt` and `fonts.txt` list Winetricks options. None establishes that every listed item ships.

The seven Wine ZIPs each have 14 conventional license-path matches, including Liberation, libjpeg-turbo, libpng, mpg123, p7zip, Ogg/Theora/Vorbis and generic license texts. This is substantially less than a per-component notice/source manifest. In particular, **`usr/local/lib/p7zip/Codecs/Rar.so` is present**, and its accompanying license includes an unRAR restriction. Do not describe every guest component as unrestricted LGPL merely because Wine uses LGPL. Inventory its exact source and additional terms or remove it only after checking functional needs.

Graphics additions require separate records:

- **psVoodoo:** the initially inspected `glide2x.dll` hash was `55aee80ac08478f8d965cede9e07b011fa046ec02f84ebc18e78b1abaf44a939`, matching the documented build at fork commit `e83947c2f4d800831db0643ad589fba1c98b2457`. Implementation files mention LGPL without consistently stating a version. The current package includes the header replacement below: DLL SHA-256 `6c4453a3316a60d409290ea41f53c98fabd51ef655401e4d196e8a07e10c3a14`, built from `67fcb0a0eb1be9c5f77d04250ad715b2f481754b`, with matching source and license materials. Preserve the implementation's applicable license plus compiler-runtime notices/source obligations; the fork is not cleared simply by being public.
- **CNC DDraw:** `C:/ddraw/ddraw.dll` is `318406915f4d11b08321949026626253554d7ae780d9aa3541cbfe5dcedbddf2`. The source is revision `a902db06e9830a9feafda69da05c766a81722b9b` plus the tracked loader patch. [Build instructions](../../../../tools/cnc-ddraw/README.md) and source identify MIT licensing. Preserve its full notices. The separate `C:/webgl/ddraw.dll` has a different hash and still needs its own identification.
- **DXVK:** guest `version.txt` reports 2.5.2. Match the actual DLLs to the exact release/build and retain notices; this review has not established the complete mapping.
- **Boxedwine GL/EGL/GLES/Vulkan and X11 shims:** associate each binary with its exact source revision and build recipe. Current repository sources alone do not identify every overwritten filesystem version.

For fonts, the inspected Wine 11 ZIP has six Wine replacement fonts, twelve Liberation fonts, and Font Awesome 4.6.3. Embedded name tables identify Wine authors and LGPL notices; Tahoma includes Bitstream Vera origin information. These are not evidence of copied Microsoft core fonts. Liberation has an OFL license file. [Font Awesome v4's font license](https://fontawesome.com/v4/license/) is SIL OFL 1.1. Preserve the exact font notices and source requirements rather than applying a single license to every font.

## Java (removed from the native release)

September 14: dedicated Java support, JRE package downloads, Java metadata and the two Java demo recipes have been removed. The records below preserve the initial inspection; Java source collection is no longer a prerequisite for this native release. They remain relevant to older packages still hosted or distributed separately.

The Java 8 ZIP contains **OpenJDK 1.8.0_492-492-b09**, a GPLv2 license with the applicable Classpath exception, `ASSEMBLY_EXCEPTION`, and `THIRD_PARTY_README`. Its `release` records source identifier `24fbffc3f77f`; it does not identify the vendor. **Original distribution URL/vendor, matching source and local modifications remain unknown.** Do not infer Oracle's proprietary JRE terms merely from the Java 8 name, or infer a vendor from one embedded string.

Java 17 identifies **Azul Zulu17.66+19-CA / 17.0.19+10-LTS** and source identifier `fde93e399a55+`. It includes module-specific `legal/` files, GPLv2, Classpath clarifications and the assembly exception. Some files refer to another module's license: preserve the directory structure and referenced documents. [Azul's matching release licensing page](https://docs.azul.com/core/tpls/april-2026/zulu17_jdk_tpl.html) confirms GPLv2 with applicable Classpath provisions and describes requesting corresponding source. We have not requested or obtained that complete matching source set.

Classpath provisions concern covered combinations with other code; they do not erase obligations for redistributing the Java runtime itself. Do not rely on passing along someone else's source offer without checking that the chosen license compliance route permits it for our distribution. Retain the original upstream archive identity, any repackaging steps, all applicable notices, and a complete source-delivery plan for both Java versions.

## Demos and artwork

The initially inspected catalog was **26R2-catalog-1**, SHA-256 `700724af3d60ef15cc594566324c66a0d2daa083a1475f0586c20477d9bf5cce`, with 36 payload entries and 36 icons. The 34 remaining entries are **pending**, meaning this audit has not collected their permission evidence; that is not a finding that permission does not exist. The two Java entries are marked excluded. The publication record for the new catalog is [Licensing/java-removal.json](Licensing/java-removal.json).

For each row, retain the exact installer/archive terms and evidence supporting mirroring or distribution through a monetized launcher. Check original-package requirements, bundled redistributables, and whether demo distribution and advertising/artwork use are treated separately. An extracted EXE icon, including Cities in Motion's, is not automatically free to redistribute. Open-source game code can accompany separately licensed commercial game data; review them separately, including the vkQuake entry.

User-imported games are a different activity from supplying our own catalog, but packaging/hosting rights still apply to everything we distribute. Do not contact publishers or alter the current catalog merely because a row is pending. A later Store-specific catalog can contain the entries with established rights while other distribution questions are resolved.

## Completion criteria and next work

1. Complete the combined-license review for the published psVoodoo replacement. Matching source and license materials are now included and the archive is verified and pinned. Java package provenance is deferred because dedicated Java support and its downloads have been removed from the native release.
2. Establish an immutable binary-to-source record for the Mac dependency archive and Wine 11 base image. Include retained TinyCore components, graphics DLLs/shims, fonts, patches, build/install scripts, and applicable compiler runtimes. Extend it to each alternate downloadable package.
3. Assemble actual third-party notices and corresponding-source materials. Make licenses and source information accessible from the app's About area and support site. A link to a moving upstream branch is not an adequate record of our modifications or build inputs; verify the chosen license-specific delivery mechanism, retention period and availability.
4. Complete the contributor authority check and obtain review of the actual Apple agreements and proposed distribution terms. Additional permission, if needed, must be scoped to rights its grantors actually hold. Do not assume a Boxedwine exception can cover Wine, Java or other third-party code.
5. Fill the demo/artwork ledger with evidence, then choose the release catalog. Repeat the inventory on the exact signed Store archive and retain its source and license materials with that release.

No app behavior, package pins, catalog entries or license grants changed during the initial audit. No source archive was published, no publisher/contributor was contacted, and no Store submission was made. Working evidence is under ignored `tmp/licensing-audit-20260914`; the tracked inventory contains hashes and repository evidence so it remains reviewable without those local files.

## September 14 follow-up: psVoodoo headers

The local `psVoodoo` fork now replaces all five proprietary-worded headers with files from the [open-source Glide tree](https://github.com/sezero/glide/tree/2f226f0f9225ce8ee83e6a4a7042981e719d19ee), pinned to `2f226f0f9225ce8ee83e6a4a7042981e719d19ee`. Each original file expressly names the **3DFX GLIDE Source Code General Public License**, identified by [SPDX as `Glide`](https://spdx.org/licenses/Glide.html). This is a distinct license with source, notice and trademark conditions, not MIT/BSD or the GNU GPL. The full upstream license, file hashes, paths and local changes are preserved in the fork's `licenses/Glide.txt` and `docs/glide-headers.{md,json}`. Three headers are byte-identical to the selected upstream files; two have dated, narrowly scoped compiler/include/internal-format adaptations.

The source change includes compile-time Win32 ABI checks and makes the build place its license and provenance documents beside the DLL. Validation found identical executable code, data, resources and all 369 export records compared with the preceding source built using the same toolchain; only build-identification metadata differs. The regression results and limitations are recorded in [Licensing/psvoodoo-header-validation.json](Licensing/psvoodoo-header-validation.json).

This addresses the replacement headers' provenance, not historical distribution or overall Store clearance. The user committed the changes as `67fcb0a0eb1be9c5f77d04250ad715b2f481754b`; the source build pin now names that revision. A rebuilt DLL and matching source/license materials were assembled into a Wine 11 filesystem using a fresh download as the base, preserving six WebGL DLL updates that were already online. After the user's upload, a fresh download matched the tested archive byte for byte and passed native package validation. The app's download size and SHA-256 now pin this published archive. See [the filesystem update record](Licensing/psvoodoo-filesystem-update.json). Do not use the earlier private validation snapshot's commit as the production source pin.


## September 14 follow-up: native notices and source records

About Boxedwine and the Help menu now open a native, offline Third-Party Licenses
window. The bundle includes selectable notices and source links for 18 components,
plus `Licenses/Third-Party-Licenses.txt` for redistribution. The build reads the
reviewed text from `Licensing/native-notices.json`; changed/missing notice text
or mismatched original libraries fail preparation. The bundle audit verifies
the generated resources, included source records and re-signed library identities.
Full external license texts and their origins are retained under `Licensing/Notices`.

The earlier native-component table describes the initial audit. Its SDL/hidapi
binary-origin question is now resolved: both original framework binaries match
the official SDL 2.0.14 DMG byte for byte. Their source is the exact official SDL
2.0.14 source archive, including its nested hidapi source and Xcode build project.
The archive served from the version-4 Mac dependencies URL contains a version-3
marker; its actual binaries match the inspected local inputs. The fresh archive
hash is recorded rather than treating this marker as a different build.

With the maintainer's approval, the untraceable MoltenVK 1.2.11 input is replaced
by the official **1.4.2** public-API macOS artifact. Native Xcode/Jenkins builds
prepare the separately pinned archive before linking and embedding. Source is
`db66022459ffb663aa2b50f6b018bc2e124f5edf`; the source tree pins SPIRV-Cross,
SPIRV-Tools, SPIRV-Headers, Vulkan-Headers and cereal. Their notices are included.
The public artifact supports arm64/x86_64 and macOS 12+, within this app's macOS 15
floor. Wine's default OpenGL renderer is unchanged.

[`Licensing/native-dependencies.json`](Licensing/native-dependencies.json) records
10 source/binary archives, their hashes, the exact original framework hashes,
build-recipe locations and limitations. Build-producer flags are not independently
reproduced. The LLVM extraction's original revision remains unknown; the adapted
code and the full LLVM exception are retained. Local source archives are under
`tmp/native-licenses/downloads`; these records do not publish a corresponding-source
release or establish Store eligibility. Wine/TinyCore and alternate filesystem
source delivery, final Boxedwine source release, demo rights, contributor authority
and the combined-license/Apple-terms review remain open.
