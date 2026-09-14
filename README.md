# ShowaBridge v0.1 for Archicad 25

`ShowaBridge/Ping` is a read-only custom Add-On command. It verifies the native C++ Add-On path from Archicad's Python Connection without creating or modifying model elements.

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
.\build_windows.ps1
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

## v0.1 safety boundary

This version registers only `ShowaBridge/Ping`. It has no menu command, does not call `ACAPI_Element_Create`, and does not change the BIM model.
