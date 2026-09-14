import json
import py_compile
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read(relative_path: str) -> str:
    return (ROOT / relative_path).read_text(encoding="utf-8-sig")


class ContractTests(unittest.TestCase):
    def test_addon_identity_and_registered_mdid(self):
        config = json.loads(read("config.json"))
        self.assertEqual(config["addOnName"], "ShowaBridge")
        self.assertEqual(config["version"], "0.1.0")
        self.assertEqual(config["defaultLanguage"], "INT")
        self.assertEqual(config["languages"], ["INT"])

        fixed_resources = read("RFIX/AddOnFix.grc")
        self.assertRegex(fixed_resources, r"'MDID'\s+32500")
        self.assertRegex(fixed_resources, r"1109684023\s*/\*\s*Developer ID")
        self.assertRegex(fixed_resources, r"683480792\s*/\*\s*Local ID")

        localized_resources = read("RINT/AddOn.grc")
        self.assertIn('"ShowaBridge"', localized_resources)
        self.assertNotIn("ID_ADDON_MENU", localized_resources)
        self.assertFalse(any((ROOT / "RINT" / "ACLib").rglob("*.*")))

    def test_ping_command_contract_is_implemented_and_registered(self):
        source = read("Src/AddOnMain.cpp")
        ping_command = re.search(
            r"class PingCommand.*?^};",
            source,
            flags=re.DOTALL | re.MULTILINE,
        )
        self.assertIsNotNone(ping_command)
        ping_source = ping_command.group(0)

        self.assertIn('return "Ping";', ping_source)
        self.assertIn('return "ShowaBridge";', ping_source)
        self.assertIn('result.Add ("ok", true);', ping_source)
        self.assertIn('result.Add ("bridge", "ShowaBridge");', ping_source)
        self.assertIn('result.Add ("version", "0.1.0");', ping_source)
        self.assertIn('result.Add ("archicadVersion", 25);', ping_source)
        self.assertIn("ACAPI_Install_AddOnCommandHandler", source)
        self.assertIn("GS::NewOwned<PingCommand>", source)
        self.assertIn("InstantExecutionOnParallelThread", ping_source)
        self.assertIn("GSErrCode RegisterInterface (void)", source)
        self.assertNotIn("ACAPI_Register_Menu", source)
        self.assertNotIn("ACAPI_Install_MenuHandler", source)
        self.assertNotIn("ACAPI_Element_Create", ping_source)
        self.assertNotIn("ACAPI_CallUndoableCommand", ping_source)

    def test_python_smoke_test_matches_command_and_response(self):
        script = ROOT / "scripts" / "01_test_ping.py"
        py_compile.compile(str(script), doraise=True)
        source = script.read_text(encoding="utf-8")
        self.assertIn('AddOnCommandId("ShowaBridge", "Ping")', source)
        for field in ("ok", "bridge", "version", "archicadVersion"):
            self.assertIn(field, source)

    def test_windows_build_script_pins_ac25_toolchain_inputs(self):
        source = read("build_windows.ps1")
        self.assertIn("Visual Studio 18 2026", source)
        self.assertIn("v142", source)
        self.assertIn("API Development Kit 25.3002/Support", source)
        self.assertIn("GRAPHISOFT/ARCHICAD 25", source)
        self.assertIn("AC_VERSION=25", source)


if __name__ == "__main__":
    unittest.main()
