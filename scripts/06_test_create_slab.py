import json

from archicad import ACConnection


BASELINE_SLAB_COUNT = 1

connection = ACConnection.connect()

if connection is None:
    raise RuntimeError("Cannot connect to Archicad")

commands = connection.commands
types = connection.types


def execute(command_name, parameters=None):
    command_id = types.AddOnCommandId(
        "ShowaBridge",
        command_name,
    )

    if parameters is None:
        return commands.ExecuteAddOnCommand(command_id)

    return commands.ExecuteAddOnCommand(
        command_id,
        parameters,
    )


counts_before = execute("GetElementCounts")

if not counts_before.get("ok", False):
    raise RuntimeError(
        "Cannot read element counts:\n"
        + json.dumps(counts_before, indent=2)
    )

if counts_before["slabs"] != BASELINE_SLAB_COUNT:
    raise RuntimeError(
        f"Safety stop: expected {BASELINE_SLAB_COUNT} slab, "
        f"but found {counts_before['slabs']}. "
        "Do not run the create test again before Undo."
    )

parameters = {
    "originXmm": 15000.0,
    "originYmm": 2000.0,
    "widthMm": 2000.0,
    "depthMm": 1500.0,
    "thicknessMm": 200.0,
    "storyIndex": 0,
    "expectedSlabCount": BASELINE_SLAB_COUNT,
    "dryRun": True,
}

dry_run_response = execute(
    "CreateSlab",
    parameters,
)

if (
    not dry_run_response.get("ok", False)
    or dry_run_response.get("created", True)
):
    raise RuntimeError(
        "CreateSlab dry-run failed:\n"
        + json.dumps(dry_run_response, indent=2)
    )

if (
    dry_run_response.get("slabCountBefore") != BASELINE_SLAB_COUNT
    or dry_run_response.get("slabCountAfter") != BASELINE_SLAB_COUNT
):
    raise RuntimeError(
        "CreateSlab dry-run changed or misreported the slab count:\n"
        + json.dumps(dry_run_response, indent=2)
    )

counts_after_dry_run = execute("GetElementCounts")

if counts_after_dry_run["slabs"] != BASELINE_SLAB_COUNT:
    raise RuntimeError(
        "Safety stop: dry-run changed the model. "
        f"Expected {BASELINE_SLAB_COUNT} slab, "
        f"but found {counts_after_dry_run['slabs']}."
    )

print("SHOWA BRIDGE CREATE SLAB DRY-RUN: OK")
print(json.dumps(dry_run_response, indent=2))

parameters["dryRun"] = False

create_response = execute(
    "CreateSlab",
    parameters,
)

if (
    not create_response.get("ok", False)
    or not create_response.get("created", False)
):
    raise RuntimeError(
        "CreateSlab failed:\n"
        + json.dumps(create_response, indent=2)
    )

if not create_response.get("guid"):
    raise RuntimeError(
        "CreateSlab returned an empty GUID:\n"
        + json.dumps(create_response, indent=2)
    )

counts_after_create = execute("GetElementCounts")

if counts_after_create["slabs"] != BASELINE_SLAB_COUNT + 1:
    raise RuntimeError(
        f"Expected {BASELINE_SLAB_COUNT + 1} slabs, "
        f"but found {counts_after_create['slabs']}."
    )

if (
    create_response.get("slabCountBefore") != BASELINE_SLAB_COUNT
    or create_response.get("slabCountAfter") != BASELINE_SLAB_COUNT + 1
):
    raise RuntimeError(
        "CreateSlab returned unexpected slab counts:\n"
        + json.dumps(create_response, indent=2)
    )

print("SHOWA BRIDGE CREATE SLAB: OK")
print(json.dumps(create_response, indent=2))
print("ELEMENT COUNTS AFTER CREATE:")
print(json.dumps(counts_after_create, indent=2))
print("NEXT: Use Ctrl+Z once in Archicad, then run 02_test_element_counts.py")
