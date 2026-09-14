"""Read-only test: inspect existing slabs, or a GUID passed on the command line."""
import json
import math
import sys
from uuid import UUID

from archicad import ACConnection


def main():
    connection = ACConnection.connect()
    if connection is None:
        raise RuntimeError("Cannot connect to Archicad")

    def execute(name, parameters=None):
        return connection.commands.ExecuteAddOnCommand(
            connection.types.AddOnCommandId("ShowaBridge", name), parameters
        )

    before = execute("GetElementCounts")
    assert before["ok"], before
    guids = sys.argv[1:] or [
        item.elementId.guid
        for item in connection.commands.GetElementsByType("Slab")
    ]
    if not guids:
        raise RuntimeError("No slab available for the read-only geometry test")
    stories = execute("GetStories")
    assert stories["ok"], stories
    levels = {s["index"]: s["elevationMeters"] * 1000 for s in stories["stories"]}
    for guid in guids:
        info = execute("GetElementInfo", {"guid": guid})
        assert info["ok"] and info["errorCode"] == 0, info
        assert UUID(info["guid"]) == UUID(guid), info
        assert info["elementType"] == "Slab", info
        assert math.isfinite(info["thicknessMm"]) and info["thicknessMm"] > 0, info
        assert math.isclose(info["storyElevationMm"], levels[info["storyIndex"]], abs_tol=1e-6)
        assert math.isclose(info["referenceElevationMm"],
                            info["storyElevationMm"] + info["levelOffsetMm"], abs_tol=1e-6)
        polygon = info["polygon"]
        points = polygon["coordinates"]
        start = 0
        for end in polygon["contourEnds"]:
            assert start + 3 <= end <= len(points), polygon
            assert points[start] == points[end - 1], polygon
            start = end
        assert start == len(points) and start > 0, polygon
        for point in points:
            assert math.isfinite(point["xMm"]) and math.isfinite(point["yMm"])
        print(json.dumps(info, indent=2))

    for guid in ("invalid", "00000000-0000-0000-0000-000000000000"):
        error = execute("GetElementInfo", {"guid": guid})
        assert not error["ok"] and error["errorCode"] == -10201, error
    walls = connection.commands.GetElementsByType("Wall")
    if walls:
        error = execute("GetElementInfo", {"guid": walls[0].elementId.guid})
        assert not error["ok"] and error["errorCode"] == -10202, error
    assert execute("GetElementCounts") == before, "Read-only command changed counts"
    print("SHOWA BRIDGE GET ELEMENT INFO: OK")


if __name__ == "__main__":
    main()
