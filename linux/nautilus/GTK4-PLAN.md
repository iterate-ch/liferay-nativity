# Plan: Nautilus 43+ (GTK 4, extension API 4.0) support

Status: plan, nothing implemented. Date: 2026-10-14.
Scope: `linux/nautilus` (C++ extension) and the Linux classes in `java/src/.../unix`. Nautilus only (no Nemo; Dolphin lives
in the Mountain Duck repository). Consumer: Mountain Duck `FileIconService` / `ContextMenuService`.

Verification: the extension API statements below were checked against the upstream sources `libnautilus-extension/*.h`
and `src/*.c` of **Nautilus 47.0** and **42.0** (GNOME GitLab tags, downloaded 2026-10-14). Items still marked **[verify]**
need a runtime check on a real session (Step 0). Confirmed facts are in the new section 1a.

## 1. What exists today (read from this checkout)

| Area | Fact | File |
|---|---|---|
| Build | `find_package(GTK2)`, hard-coded `/usr/include/{nautilus,gtk-3.0,...}`, links `nautilus-extension gobject-2.0 glib-2.0 pthread`, Boost (`algorithm`, `lexical_cast`), bundled jsoncpp | `CMakeLists.txt` |
| Entry points | `nautilus_module_{initialize,shutdown,list_types}` plus identical `nemo_module_*` | `plugin.cpp` |
| Type | `NautilusLiferay` registered with `g_type_module_register_type` + `g_type_module_add_interface` for `NAUTILUS_TYPE_MENU_PROVIDER` and `NAUTILUS_TYPE_INFO_PROVIDER` | `handlers.cpp` |
| Menu | `get_file_items(provider, GtkWidget* window, GList* files)` only; no `get_background_items`. Builds `NautilusMenu` submenus from JSON `contextMenuItems`; ignores separators (`_SEPARATOR_`); `activate` → `commandExecuted` sends `contextMenuAction` | `handlers.cpp` |
| Badges | `update_file_info` → `nautilus_file_info_add_emblem(file, getFileIconName(path))`; icon name derived from registered icon *path* (`gtk_icon_theme_get_default()` + `gtk_icon_theme_append_search_path`) | `handlers.cpp`, `content.cpp` |
| Transport | Two loopback TCP servers inside Nautilus: command port **33001** (Java → plugin, replies "1"/index) and callback port **33002** (plugin → Java, 100 ms receive timeout). Newline-framed JSON. One client per port | `requests.cpp`, `socket.cpp` |
| Java | `LinuxNativityControlImpl.loaded()` checks only `/usr/lib{,64}/nautilus/extensions-3.0/libliferaynativity.so`; ports hard-coded in `UnixNativityControlBaseImpl` | `java/src/.../unix/` |

### Defects found that block a working GTK 4 result (independent of GTK 4)

1. **`registerIconWithId` is routed to `execRegisterIconCmd`** (`requests.cpp`, the `else if` chain). The handler
   `execRegisterIconWithIdCmd` is never called, so id → icon mapping never works with Mountain Duck's string ids.
2. **No badge push or pull on Linux.** `LinuxFileIconControlImpl.refreshIcons` and `refreshWindow` are empty, and the
   plugin never asks Java for an icon id (`getFileIconId` exists only for FinderSync). Mountain Duck's
   `FileIconService.getIconForFile` is therefore never called and `setFileIcons` is never sent. Result: no badges.
3. `removeAllFileIcons` iterates the (empty) value instead of calling `removeAllFileIcons()`.
4. `ContentManager`/`SocketServer` state is shared between Nautilus' main thread and socket threads with no locking
   (`sync.h` `Mutex` exists, unused).
5. `SocketServer`: uses fd `0` as "not connected"; first constructor leaves `timeoutSeconds_/Microseconds_`
   uninitialized; `acceptHandler`/`readHandler` have no `return`; no `SO_REUSEADDR`; byte-at-a-time `recv`; a timed-out
   menu query leaves the late reply in the stream so the **next** query reads the previous answer (no request ids).
6. `addMenuItems` recursion passes `depth++` (post-increment, so depth never advances) and identifiers can collide.
7. `catch (boost::bad_lexical_cast)` by value; `-Wall` not enabled.

## 1a. Verified API facts (Nautilus 47.0 vs 42.0 headers)

| Topic | Nautilus 43+ (API 4, checked 47.0) | Nautilus ≤ 42 (API 3.0, checked 42.0) |
|---|---|---|
| Include | `#include <nautilus-extension.h>` only (installed in `.../nautilus/`, sub-headers in `libnautilus-extension/`). It declares `nautilus_module_{initialize,shutdown,list_types}`. No `nautilus-extension-types.h` any more | `nautilus-extension.h` also exists; the sub-header warns "Only <nautilus-extension.h> should be included directly" |
| pkg-config | `libnautilus-extension-4`; requires only `gio-2.0`, `glib-2.0`; `extensiondir=${libdir}/nautilus/extensions-4` | `libnautilus-extension`; requires `gtk+-3.0`; `extensiondir=${libdir}/nautilus/extensions-3.0` |
| Interface struct | `NautilusMenuProviderInterface`, `NautilusInfoProviderInterface` (no `Iface` alias) | Same names **and** `typedef ... NautilusMenuProviderIface` alias, so `...Interface` compiles on both |
| `get_file_items` | `(NautilusMenuProvider*, GList* files)` | `(provider, GtkWidget* window, GList* files)` |
| `get_background_items` | `(provider, NautilusFileInfo* current_folder)` | `(provider, GtkWidget* window, NautilusFileInfo* current_folder)` |
| Info provider | `update_file_info(provider, file, GClosure* update_complete, NautilusOperationHandle** handle)` and `cancel_update(provider, handle)`; `nautilus_info_provider_update_complete_invoke(update_complete, provider, handle, result)`; results `COMPLETE`, `FAILED`, `IN_PROGRESS` | unchanged |
| File info | `nautilus_file_info_{get_uri,get_location,add_emblem,invalidate_extension_info,list_copy,list_free}` present | present |
| Menu | `nautilus_menu_new/append_item`, `nautilus_menu_item_new(name,label,tip,icon)`, `nautilus_menu_item_set_submenu`, `nautilus_menu_item_activate`, `nautilus_menu_item_list_free`; no separator API | same |
| GTK in extension headers | **None** (only glib/gio). Linking GTK from the extension is not needed or declared | headers include `gtk/gtk.h` |
| Provider refresh | `nautilus_menu_provider_emit_items_updated_signal()` available | available |

Emblem rendering in 47.0 (read from `src/nautilus-file.c`, `nautilus-name-cell.c`, `nautilus-grid-cell.c`):

- Both list view (`NautilusNameCell`, `emblems_box`) and grid view (`NautilusGridCell`) render extension emblems. The
  assumption that GTK 4 removed emblems is **wrong for 47**; emblems are shown.
- The emblem name passed to `add_emblem` becomes a themed icon with fallback names `["emblem-" + name, name]`
  (`nautilus_file_get_emblem_icons`), then the cell checks `gtk_icon_theme_has_gicon` on
  `gtk_icon_theme_get_for_display(gdk_display_get_default())` and **skips with a warning** if neither name is in the
  theme ("Failed to add emblem ... not found in the icon theme").
  Consequence: icons must be installed in an icon theme directory (e.g. `hicolor/scalable/emblems/`), not registered by
  file path. The plugin cannot (and need not) call `gtk_icon_theme_append_search_path`.
- `add_emblem` appends to `pending_extension_emblems` while info providers are pending, otherwise directly to
  `extension_emblems`, and at `nautilus_file_info_providers_done` the pending list replaces the current one. Therefore:
  call `add_emblem` only **inside** an update (before `update_complete_invoke`), and refresh a changed badge with
  `nautilus_file_info_invalidate_extension_info` (which re-runs `update_file_info`). Calling `add_emblem` later outside an
  update would append a duplicate emblem each time.

## 2. Target

- Builds for **both** `libnautilus-extension-3.0` (GTK 3, Nautilus ≤ 42) and `libnautilus-extension-4` (Nautilus ≥ 43)
  from one source tree, selected by CMake option `NAUTILUS_API=3|4`.
- Full menu parity with the other platforms: file menu, folder-background menu, submenus, separators, disabled items.
- Badges that update: pull (`getFileIconId`) plus push (`refreshIcons`) end to end.
- Unix-domain-socket-only transport (0700 directory, `SO_PEERCRED` check) with strict timeouts and request ids.
- Java `loaded()` finds the extension in any real extension directory.

Non-goals: Nemo (hooks removed from the API 4 build, left untouched in the API 3 build), Dolphin, Windows/macOS code.

## 3. Steps

Each step lists files, the exact change, and an exit check. Order matters; steps 1–3 are prerequisites for everything.

### Step 0 – Spike: confirm the 4.0 API and set up a build/test environment (½ day)

Environment (macOS dev machine → use a container; CI must also run it):

```
podman run --rm -it -v $PWD:/src fedora:latest bash   # Fedora ships Nautilus 4x
dnf install -y cmake gcc-c++ boost-devel nautilus-devel glib2-devel pkgconf
pkg-config --modversion libnautilus-extension-4
pkg-config --variable=extensiondir libnautilus-extension-4
```

Plus an Ubuntu 22.04 (Nautilus 42, API 3.0) container for the 3.0 build. A real session test needs a Fedora/Ubuntu
GNOME VM (Nautilus cannot be run headless in a meaningful way).

Items 1–5 below are already confirmed from the upstream sources (section 1a); keep them as a build-time checklist. Only
item 6 needs a live session. Original list:

1. Include: `#include <nautilus-extension.h>` (single header) for API 4; per-file `libnautilus-extension/...` for API 3.
2. Interface structs: API 4 `NautilusMenuProviderInterface`, `NautilusInfoProviderInterface`; API 3 `...Iface`.
3. Signatures: API 4 `get_file_items(NautilusMenuProvider*, GList* files)` (no window), `get_background_items(provider,
   NautilusFileInfo* current_folder)`; API 3 `get_file_items(provider, GtkWidget* window, GList* files)`,
   `get_background_items(provider, GtkWidget* window, NautilusFileInfo* current_folder)`.
4. `nautilus_menu_item_new`, `nautilus_menu_item_set_submenu`, `nautilus_menu_new`, `nautilus_menu_append_item`,
   `nautilus_file_info_list_copy/free`, `nautilus_file_info_add_emblem`, `nautilus_file_info_invalidate_extension_info`,
   `nautilus_info_provider_update_complete_invoke` exist in both.
5. Extension GTK dependency: confirmed none for API 4 (section 1a). The plugin does not link GTK 4.
6. Emblem rendering on a live session of the oldest and newest supported releases (e.g. 43/44 and 47+) in grid and list
   view with a trivial extension that emblems every file, using an icon installed under `hicolor`. Source for 47.0 shows
   rendering in both views (section 1a); older 4.x and 48+ unchecked. **Exit gate:** if a release does not render
   emblems, badges are descoped for it and the plan continues with menus only (Step 8).

### Step 1 – Build system (½ day) — `CMakeLists.txt`

Replace the file with pkg-config based discovery; drop GTK2 and the hard-coded include paths.

```cmake
cmake_minimum_required(VERSION 3.16)
project(liferaynativity CXX)
set(CMAKE_CXX_STANDARD 17)
set(NAUTILUS_API "4" CACHE STRING "Nautilus extension API: 3 or 4")
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED)
if(NAUTILUS_API STREQUAL "4")
  pkg_check_modules(NAUTILUS REQUIRED libnautilus-extension-4)   # requires only glib/gio, no GTK
  add_compile_definitions(NATIVITY_NAUTILUS_API=4)
else()
  pkg_check_modules(NAUTILUS REQUIRED libnautilus-extension)   # 3.0 package name (confirmed in 42.0 meson.build)
  pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
  add_compile_definitions(NATIVITY_NAUTILUS_API=3)
endif()
add_compile_definitions(JSON_IS_AMALGAMATION)
add_library(liferaynativity SHARED plugin.cpp handlers.cpp logger.cpp socket.cpp
            requests.cpp content.cpp sync.cpp jsoncpp/jsoncpp.cpp)
target_include_directories(liferaynativity PRIVATE ${NAUTILUS_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
            ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/jsoncpp ${CMAKE_SOURCE_DIR}/jsoncpp/json)
target_link_libraries(liferaynativity ${NAUTILUS_LIBRARIES} pthread)
target_compile_options(liferaynativity PRIVATE -Wall -Wextra -fvisibility=hidden)
# install into the extension directory reported by pkg-config
pkg_get_variable(EXTDIR <libnautilus-extension-4 | libnautilus-extension> extensiondir)
install(TARGETS liferaynativity LIBRARY DESTINATION ${EXTDIR})
```

Keep `-Wl,--no-undefined`. Export only the three `nautilus_module_*` symbols (`__attribute__((visibility("default")))`
via a `NATIVITY_EXPORT` macro in a new `compat.h`). Drop Boost: replace `boost::starts_with`/`lexical_cast` with
`std::string::rfind`/`std::to_string` (removes a build dependency and the by-value `catch`). Exit check: `cmake -DNAUTILUS_API=4
&& make` and `-DNAUTILUS_API=3 && make` succeed in their containers (compiles may fail until Step 2; the build-system
steps are validated by configuration only).

### Step 2 – Compatibility header (½ day) — new `compat.h`

One place for all API differences so `handlers.cpp` has no `#if`s beyond calls into it. Verified simplifications:
`<nautilus-extension.h>` exists and is the correct include on **both** APIs, and `NautilusMenuProviderInterface` /
`NautilusInfoProviderInterface` are the struct names on both (3.0 only adds the deprecated `Iface` alias), so no aliases
are needed.

```cpp
#include <nautilus-extension.h>            // API 3 and API 4
#if NATIVITY_NAUTILUS_API == 4
  #define NATIVITY_MENU_FILE_ITEMS_ARGS(p, files)       p, files
#else
  // API 3.0 passes a GtkWidget* window we ignore
  #define NATIVITY_MENU_FILE_ITEMS_ARGS(p, files)       p, GtkWidget* /*window*/, files
#endif
```

(in practice two small wrapper functions per API calling the shared `buildMenu`, see Step 4). Also declares a `FileList`
helper (`std::vector<std::string>` of local paths from `GList*` of `NautilusFileInfo*`, centralizing the
`get_uri → g_filename_from_uri → free` pattern duplicated 3× in `handlers.cpp`). `libnautilus-extension/...` per-file
includes in `handlers.cpp`, `content.cpp` are replaced by `compat.h`.

### Step 3 – Entry points and type registration (½ day) — `plugin.cpp`, `handlers.cpp`

1. `plugin.cpp`: guard the `nemo_module_*` exports with `#if NATIVITY_NAUTILUS_API == 3` (so API 4 builds export only
   `nautilus_module_*`).
2. `handlers.cpp` `registerHandlers`: keep `g_type_module_register_type` + `g_type_module_add_interface`, using the
   `MenuProviderIface`/`InfoProviderIface` aliases. Name the type `MountainDuckNativity` is **not** done: keep
   `"LiferayNativity"` to avoid ABI churn for existing installs.
3. `nautilus_module_initialize`: call `RequestManager::instance()` (starts servers) only after `registerHandlers`;
   in `nautilus_module_shutdown` stop the servers, close sockets and unlink the socket path (new `RequestManager::stop()`).
4. `nautilus_module_list_types`: unchanged.

Exit check: both builds compile with only menu/info handlers stubbed; `nautilus -q` followed by starting Nautilus loads
the module (log line `nautilus_module_initialize` in `~/.liferay-nativity/liferaynativity.log`).

### Step 4 – Menu provider for API 4 and background menus (1 day) — `handlers.cpp`

1. Split the shared work into `static GList* buildMenu(const std::vector<std::string>& paths, GList* fileInfos)`:
   builds the request, queries Java, returns `NautilusMenuItem` list.
2. Wrappers:
   - API 4: `get_file_items(provider, files)` → `buildMenu`; **new** `get_background_items(provider, current_folder)` →
     `buildMenu({folderPath}, g_list_append(NULL, current_folder))`.
   - API 3: same with the extra `GtkWidget* window` parameter ignored; also add `get_background_items`.
3. Honour `rootFolder_` (mountpoint filter) for each path with a proper prefix check that requires a path-separator
   boundary (`/mnt/md` must not match `/mnt/md2`); return `NULL` if no selected path is under a filter folder.
4. Use `nautilus_file_info_list_copy` per item as today, but copy once per menu and share via `g_object_set_data_full`
   on the **top-level** item only; submenu items look the files up from the parent through a `std::shared_ptr` kept in
   `GObject` data (removes one list copy per leaf).
5. Items: render separators. Confirmed: `nautilus-menu.h` (47.0 and 42.0) has no separator API, so `_SEPARATOR_` entries stay
   skipped (same as today) and separators are unrendered on Nautilus.
6. Fix defects 6 and 7: pass `depth + 1`, make identifiers `"nativity:<depth>:<index>"` with the parent's id as prefix.
7. Menu icon: honour the optional `icon` field of a menu item (`jsonMenuItem["icon"]`), falling back to `"drive-harddisk"`.
   Java `ContextMenuItem` has `iconPath` (serialized as `iconPath`): reduce it to a theme icon name (basename, no
   extension); empty keeps the `drive-harddisk` fallback.
8. `commandExecuted`: unchanged semantics (`contextMenuAction` with title, uuid, files); build the file list from the
   `nativity::files` data. Send through `RequestManager::menuExecuted` (non-blocking write, Step 5).

Exit check: manual: right-click a file, a folder and the folder background inside a filtered folder; submenu appears;
activating an entry reaches `fireContextMenuAction` in a Java test harness (Step 9).

### Step 5 – Transport hardening (1–1.5 days) — `socket.{h,cpp}`, `requests.{h,cpp}`, Java `UnixNativityControlBaseImpl`

Goal: no UI stall, no cross-request replies, local-user-only.

1. **Unix domain sockets only.** Replace `AF_INET` with `AF_UNIX`; remove ports 33001/33002 and `htons`/loopback code.
   Paths: `$XDG_RUNTIME_DIR/liferay-nativity/<app>-command.sock` and `<app>-callback.sock` (directory mode 0700, sockets
   0600; fall back to `/run/user/<uid>` and refuse to start if neither exists, never `/tmp`). `<app>` is the first
   component of a new `NATIVITY_APP` environment variable read by the plugin (default `default`); Java derives the same
   path from the application name it is given (`NativityControl` constructor argument / system property
   `nativity.app`). The plugin verifies on `accept` that the peer uid equals its own via `SO_PEERCRED`. Java uses
   `SocketChannel.open(UnixDomainSocketAddress.of(path))` (no bytecode restriction, decision 2).
2. **Request ids.** Every callback request (`getContextMenuList`, `getFileIconId`) carries `"id": <uint64>`; replies echo it.
   `queryMenuItems`/`queryIconId` loop: read lines until the matching id or the deadline; discard stale ids. Java
   `NativityMessage` gets an optional `id` field copied from request to response (Jackson `@JsonInclude(NON_NULL)`).
3. **Deadlines, not per-recv timeouts.** `readString` uses `poll()` with an absolute deadline (menus 500 ms, icon ids
   100 ms), non-blocking sockets, buffered reads (4 KiB chunk, line splitter). Initialise all members in the constructors.
4. **Fix connection handling:** `int fd = -1` sentinel, `SO_REUSEADDR`/unlink stale socket on start, `SIGPIPE` suppressed
   with `MSG_NOSIGNAL`, accept loop returns properly, the previous client is closed under a mutex, read threads detached
   and joined on shutdown.
5. **Circuit breaker.** After 3 consecutive timeouts the plugin stops querying for 5 s and answers "no menu / no badge"
   immediately, so a hung Mountain Duck cannot repeatedly stall Nautilus.
6. **Locking.** `ContentManager` and `RequestManager` protect all maps with the existing `Mutex` (used via an RAII
   `MutexLock`); Nautilus calls arrive on the main thread, command handlers on the socket thread.
7. Java: `UnixNativityControlBaseImpl.connect()` reads the socket paths from one place; `LinuxNativityControlImpl.loaded()`
   (Step 7) shares the logic.

Exit check: kill/restart Mountain Duck while Nautilus is open: no stall > 500 ms, reconnect works; a deliberately stuck Java
callback does not freeze Nautilus for more than one deadline per 5 s window.

### Step 6 – Badges end to end (1.5 days) — `content.*`, `handlers.cpp`, `requests.cpp`, Java Linux file-icon classes

This closes defect 2 and is the largest functional change.

**Protocol additions (all backwards compatible):**

| Message | Direction | Value | Reply |
|---|---|---|---|
| `registerIconWithId` | Java → plugin (command) | `{path, iconId}` (fix routing, defect 1) | `1` |
| `getFileIconId` | plugin → Java (callback) | `{"id":n,"command":"getFileIconId","value":["/path"]}` | `{"id":n,"command":"setFileIcons","value":{"/path": iconId}}` (same shape as `FSFileIconControlImpl`) |
| `refreshIcons` | Java → plugin (command) | `["/path", ...]` | `1`; plugin drops cached icon ids for those paths and calls `nautilus_file_info_invalidate_extension_info` on the matching live `NautilusFileInfo` objects |

**Plugin:**

1. Keep a `std::unordered_map<std::string, int> iconIds_` cache plus an LRU cap (default 10 000) in `ContentManager`.
2. `update_file_info`:
   1. if the path is not under a filter folder → `NAUTILUS_OPERATION_COMPLETE`;
   2. cache hit → add the emblem synchronously;
   3. miss → **asynchronous completion**: return `NAUTILUS_OPERATION_IN_PROGRESS`, enqueue `{path, update_complete closure,
      handle, NautilusFileInfo ref}` to a worker thread that performs `getFileIconId` (100 ms deadline), then on the main
      loop (`g_idle_add`) adds the emblem and calls `nautilus_info_provider_update_complete_invoke(update_complete,
      provider, handle, NAUTILUS_OPERATION_COMPLETE)`. Implement `cancel_update` (API requires it when returning
      IN_PROGRESS) by flagging the handle cancelled. This moves all socket I/O off Nautilus' main thread.
   4. Batch: the worker coalesces requests that arrive within 20 ms into one `getFileIconId` with several paths.
3. `refreshIcons` handler: for each path remove the cache entry; for live file infos (a `std::unordered_map<path,
   GWeakRef/NautilusFileInfo*>` filled in `update_file_info` and cleaned on destroy via `g_object_weak_ref`) call
   `nautilus_file_info_invalidate_extension_info` **on the main loop** (`g_idle_add`), never from the socket thread.
4. Icon resolution (`getFileIconName`), based on the verified emblem lookup (section 1a): pass an icon **name** to
   `nautilus_file_info_add_emblem`; Nautilus resolves `emblem-<name>` then `<name>` in the display's icon theme and drops
   the emblem if neither exists. Mountain Duck registers `iconId → name` (`mountainduck-badge-<state>`) and installs the
   SVGs into `hicolor/scalable/emblems/`. Paths passed to `registerIconWithId` are reduced to the basename without
   extension (as `getFileIconName` does today) and the directory part is ignored; **no GTK icon-theme calls remain in the
   plugin** (removes `gtk_icon_theme_get_default`, the only GTK use, which does not exist in GTK 4 builds anyway).
   Log once per name if `GtkIconTheme` lookups fail is not possible from the extension; the Nautilus warning
   "Failed to add emblem" is the diagnostic, documented in the README.
   `add_emblem` is called only inside the update (before `update_complete_invoke`) to avoid duplicates (section 1a).
5. Java: implement in `LinuxFileIconControlImpl`:
   - `refreshIcons(String[] paths)` → send `REFRESH_ICONS` (copy of `FSFileIconControlImpl`);
   - register a `MessageListener(GET_FILE_ICON_ID)` returning the `SET_FILE_ICONS` map (copy of the FinderSync code);
   - `registerIconWithId` also puts `Constants.LABEL` for symmetry.
   `UnixNativityControlBaseImpl.fireMessage` already dispatches callback messages to registered listeners.

Exit check: set a badge via Mountain Duck, create a file: emblem appears; change state: `refreshIcons` updates it within
one coalescing window with Nautilus idle; no main-thread socket I/O (verify with `strace -f -p $(pidof nautilus) -e
trace=connect,sendto,recvfrom,poll` while scrolling a 5 000 entry folder).

### Step 7 – Java loader and packaging detection (½ day) — `LinuxNativityControlImpl`, `OSDetector`

1. `loaded()`: return true when `libliferaynativity.so` exists in any of
   `/usr/lib/nautilus/extensions-{3.0,4}/`, `/usr/lib64/nautilus/extensions-{3.0,4}/`,
   `/usr/lib/<multiarch>/nautilus/extensions-{3.0,4}/`, `/usr/local/lib/...`, `$NAUTILUS_EXTENSION_PATH` (colon list).
   Multiarch triplet from `Files.list` of `/usr/lib/*-linux-gnu`.
2. Add `String nautilusApi()` returning `"3"`, `"4"` or `null` for logging.
3. Unit tests (Step 9) using a temp-directory injected through the env var.

### Step 8 – Compatibility and degrade behaviour (½ day)

1. Compile-time: API 3 and API 4 builds. Run-time: a Nautilus ≥ 43 with only the API 3 `.so` simply does not load it
   (Nautilus scans `extensions-4`); document that both packages exist.
2. If emblem rendering is not available on a release (Step 0 gate), `update_file_info` is still implemented but returns
   immediately and the plugin reports `"emblems":false` in the `enableFileIcons` reply so Java can log it; menus unaffected.
3. Version-gate quirks in `compat.h` with `NAUTILUS_MAJOR_VERSION` if Step 0 finds behavioural differences between 43–47.

### Step 9 – Tests (1–1.5 days)

Plugin (C++, built only with `-DNATIVITY_BUILD_TESTS=ON`, no Nautilus needed):

1. Factor transport + protocol + `ContentManager` into a static library `nativity_core` that does **not** include Nautilus
   headers. `liferaynativity.so` links it. Unit tests (Catch2 or GoogleTest via `FetchContent`) for:
   JSON command dispatch (including `registerIconWithId` routing, defect 1), request-id matching and stale-reply discard,
   deadline expiry, circuit breaker, filter-folder prefix boundary, LRU cap, concurrent access under TSan.
2. Fake peer test: a thread acting as the Java side over a socketpair/Unix socket.

Java (`java/test`): extend with a fake plugin server (Unix socket): connect, `registerIconWithId`, `getFileIconId`
round trip, `refreshIcons` message, `contextMenuAction`, reconnect after server close, `loaded()` search paths.
`mvn -q test`.

Manual matrix (record results in this file): Ubuntu 22.04 (Nautilus 42, API 3), Ubuntu 24.04 (46, API 4), Fedora current
(API 4), GNOME 47+/48: file menu, folder background, multi-selection, submenu depth 2, disabled item, badge create/update/
clear, Mountain Duck restart, Nautilus restart (`nautilus -q`), 10 000-file folder scroll smoothness.

### Step 10 – CI and packaging (1 day)

1. GitHub Actions (or the existing CI) jobs: `cmake -DNAUTILUS_API=3` on `ubuntu:22.04`, `-DNAUTILUS_API=4` on `fedora:latest`
   and `ubuntu:24.04`; run `nativity_core` tests; upload `libliferaynativity-nautilus3.so` and `-nautilus4.so` artifacts.
2. `install` target honours `DESTDIR`; `cpack` DEB/RPM subpackages `nativity-nautilus3`, `nativity-nautilus4` depending on
   `libnautilus-extension1a`/`libnautilus-extension4`.
3. README.md "Linux" section rewritten: supported Nautilus versions, build commands per API, extension directories,
   `nautilus -q` instead of `killall -9 nautilus`, log path `~/.liferay-nativity/liferaynativity.log`, Nemo no longer
   supported in the API 4 build.
4. Release: bump to `2.2.0`; the Java jar and the `.so` must be released together (protocol changes in Steps 5–6).

## 4. Sequencing and effort

| Step | Days | Depends on |
|---|---|---|
| 0 Spike | 0.5 | – |
| 1 Build system | 0.5 | 0 |
| 2 compat.h | 0.5 | 0 |
| 3 Entry/types | 0.5 | 1, 2 |
| 4 Menus | 1 | 3 |
| 5 Transport | 1–1.5 | 3 (can run in parallel with 4) |
| 6 Badges | 1.5 | 4, 5 |
| 7 Java loader | 0.5 | – (parallel) |
| 8 Degrade | 0.5 | 6 |
| 9 Tests | 1–1.5 | 5, 6 (core tests start with 5) |
| 10 CI/packaging | 1 | 1 |
| **Total** | **~9–10** | |

Critical path: 0 → 1/2 → 3 → 5 → 6 → 9. A **minimum viable GTK 4** (menus only, no badges, transport hardening deferred) is steps 0–4 and
7, about 3.5 days, and can ship first.

## 5. Risks

| Risk | Mitigation |
|---|---|
| API differences | Verified against 42.0/47.0 (section 1a); isolated in `compat.h`; CI builds both APIs |
| Emblems not rendered on some 4.x releases (47.0 renders in list and grid) | Step 0.6 live check on oldest/newest supported release; menus-only degrade (Step 8); document supported releases |
| Async `update_file_info` bugs (leaks, completion after file info destroyed) | Hold a `g_object_ref` on the `NautilusFileInfo`, `cancel_update` flag, TSan + ASan runs in CI, soak test scrolling 10 000 files |
| Linux jar needs a newer bytecode level than macOS/Windows consumers | Decision 2: classifier or enforcer exemption |
| Transport change breaks old Mountain Duck/new plugin or vice versa | Unix sockets are a hard break: plugin and Java jar are released and installed together (same version check in step 10) |
| Emblem icon missing from theme → silently skipped by Nautilus | Install SVGs in `hicolor`; packaging test runs `gtk4-icon-browser`/`gtk-query-settings` equivalent check; README documents the warning |
| Flatpak/Snap Nautilus cannot load the extension | Out of scope, documented |

## 6. Decisions (resolved)

1. **Transport: Unix domain sockets only.** TCP ports 33001/33002 are removed from the plugin and the Java client; no
   fallback.
2. **Java baseline: no bytecode 1.8 restriction.** The Linux transport uses `java.net.UnixDomainSocketAddress` /
   `SocketChannel` directly. Raise `maven.compile.source/target` in `pom.xml` to the lowest LTS that provides it (17;
   Mountain Duck's Linux module runs 25). Caveat to resolve in Mountain Duck: its parent POM enforcer fails the build on
   non-test dependencies with bytecode above 1.8 and macOS/Windows consume the same `ch.iterate.liferay:nativity`
   artifact. Either publish the Linux-capable build with a classifier (`linux`) or exempt this artifact in the enforcer
   rule for the `linux` profile; macOS/Windows keep the 1.8 artifact.
3. **Extension/type name stays `LiferayNativity`** (library `libliferaynativity.so`).
4. **Badges are pulled on demand** (`getFileIconId`) with `refreshIcons` as the invalidation push, as in Step 6.
