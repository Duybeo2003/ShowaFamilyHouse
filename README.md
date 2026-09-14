# ShowaBridge v0.1 for Archicad 25

Private C++ bridge for Archicad 25, called from the Archicad Python Connection.
Commands: `Ping`, `GetElementCounts`, `GetProjectInfo`, `GetStories`,
`CreateWall`, `CreateSlab`, and `GetElementInfo` (slab inspection).

Expected response:

```json
{
  "ok": true,
  "bridge": "ShowaBridge",
  "version": "0.1.0",
  "archicadVersion": 25
}
```

## 1. Apply this source overlay

Extract the ZIP to a temporary location. Open the resulting
`ShowaBridge-v0.1-ac25-source-overlay` folder, then copy **its contents** into:

```text
U:\My_Home\ShowaFamilyHouse\ShowaAddonTemplate-ac25
```

Allow Windows to replace files with the same names. Do not copy the outer overlay folder itself into the clone. Keep the existing `Tools` folder from the cloned Graphisoft CMake template. The build script stops with a clear error if that submodule is missing.

The `AddOnCommandTest` reference folder from the input archive is not required by this implementation.

## 2. Build

Open PowerShell in `ShowaAddonTemplate-ac25` and run:

```powershell
.\build_windows.bat
```

The script uses the already verified toolchain:

- Visual Studio 18 2026 generator
- MSVC v142 toolset
- Archicad 25 API Development Kit 25.3002
- Archicad 25 `LP_XMLConverter.exe`
- x64 Debug configuration

Expected output path:

```text
Build-v142-lpxml\Debug\ShowaBridge.apx
```

The Graphisoft static library may emit `LNK4099` about `API_c.pdb`; the earlier template build showed that warning while still completing successfully.

## 3. Load the Add-On

1. In Archicad 25, open **Options > Add-On Manager**.
2. Select **Add New Add-On** and choose `Build-v142-lpxml\Debug\ShowaBridge.apx`.
3. Confirm that ShowaBridge is loaded. Restart Archicad if it asks you to.

Registered MDID:

- Developer ID: `1109684023`
- Local ID: `683480792`

## 4. Test from Python Palette

Add this package's `scripts` folder to the Archicad Python Palette, then run:

```text
01_test_ping.py
```

Success output begins with:

```text
SHOWA BRIDGE PING: OK
```

## Slab test and verified results (2026-09-15)

Run `scripts/06_test_create_slab.py` with the **0. Ground Floor plan open**.
It requires one existing slab, checks the active story both before dry-run and
immediately before creation, then creates one 2000 x 1500 mm slab, thickness
200 mm, at (15000, 2000) mm. Inputs are mm; C++ converts to metres.
Close any modal dialog before running. A story guard cannot prevent every
Archicad modal dialog or a change of window during execution.

Verified in the live project before adding GetElementInfo:

- Dry-run: slabs 1 -> 1, created=false.
- Create: slabs 1 -> 2, created=true, errorCode=0.
- Returned GUID: `2A9BB665-9B3E-43E7-A408-B84023A6B08D`.
- Undo: slabs 2 -> 1; walls remained 4. This GUID is now historical.
- Incorrect expected count, nonexistent story, zero width, negative depth,
  and zero thickness were rejected without changing counts.

Creating on Story 0 while Story 2 was visible caused the Information dialog
and Python error 4001 **after the slab had been created**. After such an error,
close the dialog and count elements before retrying; never assume rollback.
Use Ctrl+Z once after a successful creation test and verify the baseline.

## GetElementInfo (read-only, slab support)

Input: `{"guid":"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"}`.
Success returns `ok`, `errorCode`, `message`, `guid`, `elementType`,
`storyIndex`, `thicknessMm`, `levelOffsetMm`, `storyElevationMm`,
`referenceElevationMm`, `referencePlaneLocation`, and `polygon`.
The absolute reference elevation is the story elevation plus the slab's
reference-plane offset; it is not necessarily the slab top elevation.
Reference-plane codes follow AC25: 0=top, 1=core top, 2=core bottom, 3=bottom.

Polygon `coordinates` contain xMm/yMm and each contour's repeated closing
point. `contourEnds` and arc beginIndex/endIndex retain Archicad's **1-based**
indices; JSON array positions are 0-based. Arc angles are radians.
Errors return only ok/errorCode/message/guid; no fabricated geometry.
Invalid/null GUID: -10201; unsupported element type: -10202;
missing story: -10203; incomplete memo: -10204. API errors are passed through.

After reloading the new APX, run `scripts/07_test_get_element_info.py` to
inspect existing slabs without changing the model. From a terminal, optionally
pass a slab GUID as its argument. Live verification of this new command is
pending reload of the APX; the earlier CreateSlab results above do not verify it.

Use the repository's `scripts` folder in Python Palette so it runs the current
tests rather than older copies in the parent project's scripts folder.

The updated creation test requires GetElementInfo to be installed before any
write. It reads back the new GUID and checks all four corners, closure, absence
of arcs, story 0, and the requested 200 mm thickness. A geometry failure after
creation leaves the slab available for inspection; Undo once before retrying.

If the loaded Debug APX is locked (LNK1168), link to a separate staging directory:

```powershell
cmake --build Build-v142-lpxml --config Debug -- /p:OutDir=U:\My_Home\ShowaFamilyHouse\ShowaAddonTemplate-ac25\Build-v142-lpxml\Pending\
```

The staged binary is `Build-v142-lpxml/Pending/ShowaBridge.apx`. Reload this
binary in Add-On Manager (replace the existing ShowaBridge entry rather than
loading duplicate IDs). If Archicad requires a restart, handle project saving
explicitly before restarting. The Debug binary remains the previously loaded
version until rebuilt after it is unloaded.
