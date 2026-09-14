import json

from archicad import ACConnection


connection = ACConnection.connect()
if connection is None:
    raise RuntimeError("Cannot connect to a running Archicad instance.")

commands = connection.commands
types = connection.types

command_id = types.AddOnCommandId("ShowaBridge", "Ping")
response = commands.ExecuteAddOnCommand(command_id)

expected = {
    "ok": True,
    "bridge": "ShowaBridge",
    "version": "0.1.0",
    "archicadVersion": 25,
}

if response != expected:
    raise AssertionError(
        "Unexpected ShowaBridge/Ping response:\n"
        f"Expected: {json.dumps(expected, ensure_ascii=False, indent=2)}\n"
        f"Actual:   {json.dumps(response, ensure_ascii=False, indent=2)}"
    )

print("SHOWA BRIDGE PING: OK")
print(json.dumps(response, ensure_ascii=False, indent=2))
