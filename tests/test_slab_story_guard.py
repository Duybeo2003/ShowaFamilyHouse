"""Exercise the actual script with a simulated Archicad connection."""
import runpy
import sys
import types
import unittest
from pathlib import Path
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "06_test_create_slab.py"


class StoryGuardTests(unittest.TestCase):
    def run_guard_case(self, stories):
        writes = []
        story_iter = iter(stories)

        def execute(name, parameters=None):
            if name == "GetStories":
                return next(story_iter)
            if name == "GetElementCounts":
                return {"ok": True, "slabs": 1}
            if name == "GetElementInfo":
                return {"ok": False, "errorCode": -10201}
            if name == "CreateSlab":
                if not parameters["dryRun"]:
                    writes.append(dict(parameters))
                    raise AssertionError("Unsafe create reached")
                return {"ok": True, "created": False,
                        "slabCountBefore": 1, "slabCountAfter": 1}
            raise AssertionError(name)

        connection = types.SimpleNamespace(
            commands=types.SimpleNamespace(ExecuteAddOnCommand=execute),
            types=types.SimpleNamespace(AddOnCommandId=lambda namespace, name: name),
        )
        module = types.ModuleType("archicad")
        module.ACConnection = types.SimpleNamespace(connect=lambda: connection)
        with patch.dict(sys.modules, {"archicad": module}):
            with self.assertRaisesRegex(RuntimeError, "Ground Floor"):
                runpy.run_path(str(SCRIPT), run_name="__main__")
        self.assertEqual(writes, [])

    def test_wrong_story_stops_before_create(self):
        self.run_guard_case([{"ok": True, "activeStory": 2}])

    def test_story_read_error_stops_before_create(self):
        self.run_guard_case([{"ok": False}])

    def test_story_changed_after_dry_run_stops_before_create(self):
        self.run_guard_case([{"ok": True, "activeStory": 0},
                             {"ok": True, "activeStory": 2}])


if __name__ == "__main__":
    unittest.main()
