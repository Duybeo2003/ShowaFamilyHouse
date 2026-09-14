#include "APIEnvir.h"
#include "ACAPinc.h"

#include "ObjectState.hpp"
#include "ResourceIds.hpp"
#include "RS.hpp"
#include "GetElementInfoCommand.hpp"


namespace {

constexpr GSResID AddOnInfoID = ID_ADDON_INFO;
constexpr Int32 AddOnNameID = 1;
constexpr Int32 AddOnDescriptionID = 2;


class PingCommand final : public API_AddOnCommand {
public:
	GS::String GetName () const override
	{
		return "Ping";
	}

	GS::String GetNamespace () const override
	{
		return "ShowaBridge";
	}

	GS::Optional<GS::UniString> GetSchemaDefinitions () const override
	{
		return GS::NoValue;
	}

	GS::Optional<GS::UniString> GetInputParametersSchema () const override
	{
		return GS::NoValue;
	}

	GS::Optional<GS::UniString> GetResponseSchema () const override
	{
		return "{ "
			   "\"type\": \"object\", "
			   "\"properties\": { "
			       "\"ok\": { \"type\": \"boolean\" }, "
			       "\"bridge\": { \"type\": \"string\" }, "
			       "\"version\": { \"type\": \"string\" }, "
			       "\"archicadVersion\": { \"type\": \"integer\" }"
			   " }, "
			   "\"additionalProperties\": false, "
			   "\"required\": [ \"ok\", \"bridge\", \"version\", \"archicadVersion\" ]"
			   " }";
	}

	API_AddOnCommandExecutionPolicy GetExecutionPolicy () const override
	{
		return API_AddOnCommandExecutionPolicy::InstantExecutionOnParallelThread;
	}

	GS::ObjectState Execute (const GS::ObjectState& /*parameters*/, GS::ProcessControl& /*processControl*/) const override
	{
		GS::ObjectState result;
		result.Add ("ok", true);
		result.Add ("bridge", "ShowaBridge");
		result.Add ("version", "0.1.0");
		result.Add ("archicadVersion", 25);
		return result;
	}

	void OnResponseValidationFailed (const GS::ObjectState& /*response*/) const override
	{
	}
};

}


namespace {

static GSErrCode ReadElementCount (
    const API_ElemTypeID typeId,
    Int32& count
)
{
    GS::Array<API_Guid> elements;
    const GSErrCode err = ACAPI_Element_GetElemList (typeId, &elements);

    if (err == NoError)
        count = static_cast<Int32> (elements.GetSize ());

    return err;
}


class GetElementCountsCommand final : public API_AddOnCommand
{
public:
    GS::String GetName () const override
    {
        return "GetElementCounts";
    }

    GS::String GetNamespace () const override
    {
        return "ShowaBridge";
    }

    GS::Optional<GS::UniString> GetSchemaDefinitions () const override
    {
        return GS::NoValue;
    }

    GS::Optional<GS::UniString> GetInputParametersSchema () const override
    {
        return GS::UniString ("{}");
    }

    GS::Optional<GS::UniString> GetResponseSchema () const override
    {
        return GS::UniString (R"({
            "type": "object",
            "properties": {
                "ok":        { "type": "boolean" },
                "errorCode": { "type": "integer" },
                "walls":     { "type": "integer" },
                "slabs":     { "type": "integer" },
                "doors":     { "type": "integer" },
                "windows":   { "type": "integer" },
                "roofs":     { "type": "integer" },
                "zones":     { "type": "integer" },
                "stairs":    { "type": "integer" },
                "openings":  { "type": "integer" }
            },
            "required": [
                "ok", "errorCode", "walls", "slabs",
                "doors", "windows", "roofs", "zones",
                "stairs", "openings"
            ],
            "additionalProperties": false
        })");
    }

    API_AddOnCommandExecutionPolicy GetExecutionPolicy () const override
    {
        return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread;
    }

    GS::ObjectState Execute (
        const GS::ObjectState& /* parameters */,
        GS::ProcessControl& /* processControl */
    ) const override
    {
        Int32 walls = 0;
        Int32 slabs = 0;
        Int32 doors = 0;
        Int32 windows = 0;
        Int32 roofs = 0;
        Int32 zones = 0;
        Int32 stairs = 0;
        Int32 openings = 0;

        GSErrCode firstError = NoError;

        const auto collect = [&firstError] (
            const API_ElemTypeID typeId,
            Int32& count
        ) {
            const GSErrCode err = ReadElementCount (typeId, count);

            if (firstError == NoError && err != NoError)
                firstError = err;
        };

        collect (API_WallID, walls);
        collect (API_SlabID, slabs);
        collect (API_DoorID, doors);
        collect (API_WindowID, windows);
        collect (API_RoofID, roofs);
        collect (API_ZoneID, zones);
        collect (API_StairID, stairs);
        collect (API_OpeningID, openings);

        GS::ObjectState response;
        response.Add ("ok", firstError == NoError);
        response.Add ("errorCode", static_cast<Int32> (firstError));
        response.Add ("walls", walls);
        response.Add ("slabs", slabs);
        response.Add ("doors", doors);
        response.Add ("windows", windows);
        response.Add ("roofs", roofs);
        response.Add ("zones", zones);
        response.Add ("stairs", stairs);
        response.Add ("openings", openings);

        return response;
    }

    void OnResponseValidationFailed (
        const GS::ObjectState& /* response */
    ) const override
    {
    }
};

} // namespace

namespace {

class GetProjectInfoCommand final : public API_AddOnCommand
{
public:
    GS::String GetName () const override
    {
        return "GetProjectInfo";
    }

    GS::String GetNamespace () const override
    {
        return "ShowaBridge";
    }

    GS::Optional<GS::UniString> GetSchemaDefinitions () const override
    {
        return GS::NoValue;
    }

    GS::Optional<GS::UniString> GetInputParametersSchema () const override
    {
        return GS::UniString ("{}");
    }

    GS::Optional<GS::UniString> GetResponseSchema () const override
    {
        return GS::UniString (R"({
            "type": "object",
            "properties": {
                "ok":          { "type": "boolean" },
                "errorCode":   { "type": "integer" },
                "untitled":    { "type": "boolean" },
                "teamwork":    { "type": "boolean" },
                "userId":      { "type": "integer" },
                "projectName": { "type": "string" },
                "projectPath": { "type": "string" }
            },
            "required": [
                "ok",
                "errorCode",
                "untitled",
                "teamwork",
                "userId",
                "projectName",
                "projectPath"
            ],
            "additionalProperties": false
        })");
    }

    API_AddOnCommandExecutionPolicy GetExecutionPolicy () const override
    {
        return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread;
    }

    GS::ObjectState Execute (
        const GS::ObjectState& /* parameters */,
        GS::ProcessControl& /* processControl */
    ) const override
    {
        API_ProjectInfo projectInfo {};
        const GSErrCode err = ACAPI_Environment (
            APIEnv_ProjectID,
            &projectInfo
        );

        GS::UniString projectName;
        GS::UniString projectPath;

        if (err == NoError) {
            if (projectInfo.projectName != nullptr)
                projectName = *projectInfo.projectName;

            if (projectInfo.projectPath != nullptr)
                projectPath = *projectInfo.projectPath;
        }

        GS::ObjectState response;
        response.Add ("ok", err == NoError);
        response.Add ("errorCode", static_cast<Int32> (err));
        response.Add ("untitled", err == NoError ? projectInfo.untitled : false);
        response.Add ("teamwork", err == NoError ? projectInfo.teamwork : false);
        response.Add (
            "userId",
            err == NoError ? static_cast<Int32> (projectInfo.userId) : 0
        );
        response.Add ("projectName", projectName);
        response.Add ("projectPath", projectPath);

        return response;
    }

    void OnResponseValidationFailed (
        const GS::ObjectState& /* response */
    ) const override
    {
    }
};

} // namespace

namespace {

class GetStoriesCommand final : public API_AddOnCommand
{
public:
    GS::String GetName () const override
    {
        return "GetStories";
    }

    GS::String GetNamespace () const override
    {
        return "ShowaBridge";
    }

    GS::Optional<GS::UniString> GetSchemaDefinitions () const override
    {
        return GS::NoValue;
    }

    GS::Optional<GS::UniString> GetInputParametersSchema () const override
    {
        return GS::UniString ("{}");
    }

    GS::Optional<GS::UniString> GetResponseSchema () const override
    {
        return GS::UniString (R"({
            "type": "object",
            "properties": {
                "ok": {
                    "type": "boolean"
                },
                "errorCode": {
                    "type": "integer"
                },
                "firstStory": {
                    "type": "integer"
                },
                "lastStory": {
                    "type": "integer"
                },
                "activeStory": {
                    "type": "integer"
                },
                "stories": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "index": {
                                "type": "integer"
                            },
                            "floorId": {
                                "type": "integer"
                            },
                            "name": {
                                "type": "string"
                            },
                            "elevationMeters": {
                                "type": "number"
                            },
                            "active": {
                                "type": "boolean"
                            }
                        },
                        "required": [
                            "index",
                            "floorId",
                            "name",
                            "elevationMeters",
                            "active"
                        ],
                        "additionalProperties": false
                    }
                }
            },
            "required": [
                "ok",
                "errorCode",
                "firstStory",
                "lastStory",
                "activeStory",
                "stories"
            ],
            "additionalProperties": false
        })");
    }

    API_AddOnCommandExecutionPolicy GetExecutionPolicy () const override
    {
        return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread;
    }

    GS::ObjectState Execute (
        const GS::ObjectState& /* parameters */,
        GS::ProcessControl& /* processControl */
    ) const override
    {
        API_StoryInfo storyInfo {};

        const GSErrCode err = ACAPI_Environment (
            APIEnv_GetStorySettingsID,
            &storyInfo,
            nullptr
        );

        GS::ObjectState response;

        response.Add ("ok", err == NoError);
        response.Add ("errorCode", static_cast<Int32> (err));

        response.Add (
            "firstStory",
            err == NoError ? static_cast<Int32> (storyInfo.firstStory) : 0
        );

        response.Add (
            "lastStory",
            err == NoError ? static_cast<Int32> (storyInfo.lastStory) : 0
        );

        response.Add (
            "activeStory",
            err == NoError ? static_cast<Int32> (storyInfo.actStory) : 0
        );

        const auto addStory =
            response.AddList<GS::ObjectState> ("stories");

        if (err == NoError && storyInfo.data != nullptr) {
            const Int32 storyCount =
                static_cast<Int32> (
                    storyInfo.lastStory - storyInfo.firstStory
                ) + 1;

            for (Int32 offset = 0; offset < storyCount; ++offset) {
                const API_StoryType& story =
                    (*storyInfo.data)[offset];

                GS::ObjectState storyState;

                storyState.Add (
                    "index",
                    static_cast<Int32> (story.index)
                );

                storyState.Add (
                    "floorId",
                    static_cast<Int32> (story.floorId)
                );

                storyState.Add (
                    "name",
                    GS::UniString (story.uName)
                );

                storyState.Add (
                    "elevationMeters",
                    story.level
                );

                storyState.Add (
                    "active",
                    story.index == storyInfo.actStory
                );

                addStory (storyState);
            }
        }

        if (storyInfo.data != nullptr)
            BMKillHandle ((GSHandle*) &storyInfo.data);

        return response;
    }

    void OnResponseValidationFailed (
        const GS::ObjectState& /* response */
    ) const override
    {
    }
};

} // namespace

namespace {

class CreateWallCommand final : public API_AddOnCommand
{
public:
    GS::String GetName () const override
    {
        return "CreateWall";
    }

    GS::String GetNamespace () const override
    {
        return "ShowaBridge";
    }

    GS::Optional<GS::UniString> GetSchemaDefinitions () const override
    {
        return GS::NoValue;
    }

    GS::Optional<GS::UniString> GetInputParametersSchema () const override
    {
        return GS::UniString (R"({
            "type": "object",
            "properties": {
                "startXmm": {
                    "type": "number"
                },
                "startYmm": {
                    "type": "number"
                },
                "endXmm": {
                    "type": "number"
                },
                "endYmm": {
                    "type": "number"
                },
                "heightMm": {
                    "type": "number"
                },
                "thicknessMm": {
                    "type": "number"
                },
                "storyIndex": {
                    "type": "integer"
                },
                "expectedWallCount": {
                    "type": "integer"
                },
                "dryRun": {
                    "type": "boolean"
                }
            },
            "required": [
                "startXmm",
                "startYmm",
                "endXmm",
                "endYmm",
                "heightMm",
                "thicknessMm",
                "storyIndex",
                "expectedWallCount",
                "dryRun"
            ],
            "additionalProperties": false
        })");
    }

    GS::Optional<GS::UniString> GetResponseSchema () const override
    {
        return GS::UniString (R"({
            "type": "object",
            "properties": {
                "ok": {
                    "type": "boolean"
                },
                "created": {
                    "type": "boolean"
                },
                "errorCode": {
                    "type": "integer"
                },
                "message": {
                    "type": "string"
                },
                "guid": {
                    "type": "string"
                },
                "wallCountBefore": {
                    "type": "integer"
                },
                "wallCountAfter": {
                    "type": "integer"
                }
            },
            "required": [
                "ok",
                "created",
                "errorCode",
                "message",
                "guid",
                "wallCountBefore",
                "wallCountAfter"
            ],
            "additionalProperties": false
        })");
    }

    API_AddOnCommandExecutionPolicy GetExecutionPolicy () const override
    {
        return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread;
    }

    GS::ObjectState Execute (
        const GS::ObjectState& parameters,
        GS::ProcessControl& /* processControl */
    ) const override
    {
        double startXmm = 0.0;
        double startYmm = 0.0;
        double endXmm = 0.0;
        double endYmm = 0.0;
        double heightMm = 0.0;
        double thicknessMm = 0.0;

        Int32 storyIndex = 0;
        Int32 expectedWallCount = 0;
        bool dryRun = false;

        if (!parameters.Get ("startXmm", startXmm) ||
            !parameters.Get ("startYmm", startYmm) ||
            !parameters.Get ("endXmm", endXmm) ||
            !parameters.Get ("endYmm", endYmm) ||
            !parameters.Get ("heightMm", heightMm) ||
            !parameters.Get ("thicknessMm", thicknessMm) ||
            !parameters.Get ("storyIndex", storyIndex) ||
            !parameters.Get ("expectedWallCount", expectedWallCount) ||
            !parameters.Get ("dryRun", dryRun)) {
            return MakeResponse (
                false,
                false,
                -10001,
                GS::UniString ("Invalid or missing input parameters."),
                GS::UniString (),
                0,
                0
            );
        }

        const double deltaXmm = endXmm - startXmm;
        const double deltaYmm = endYmm - startYmm;
        const double lengthSquaredMm =
            deltaXmm * deltaXmm + deltaYmm * deltaYmm;

        if (lengthSquaredMm < 1.0 ||
            heightMm < 100.0 ||
            heightMm > 20000.0 ||
            thicknessMm < 10.0 ||
            thicknessMm > 2000.0 ||
            expectedWallCount < 0) {
            return MakeResponse (
                false,
                false,
                -10002,
                GS::UniString ("Wall dimensions or expected count are invalid."),
                GS::UniString (),
                0,
                0
            );
        }

        API_StoryInfo storyInfo {};

        const GSErrCode storyError = ACAPI_Environment (
            APIEnv_GetStorySettingsID,
            &storyInfo,
            nullptr
        );

        if (storyError != NoError) {
            if (storyInfo.data != nullptr)
                BMKillHandle ((GSHandle*) &storyInfo.data);

            return MakeResponse (
                false,
                false,
                static_cast<Int32> (storyError),
                GS::UniString ("Unable to read story settings."),
                GS::UniString (),
                0,
                0
            );
        }

        bool storyFound = false;

        if (storyInfo.data != nullptr) {
            const Int32 storyCount =
                static_cast<Int32> (
                    storyInfo.lastStory - storyInfo.firstStory
                ) + 1;

            for (Int32 offset = 0; offset < storyCount; ++offset) {
                if (
                    static_cast<Int32> (
                        (*storyInfo.data)[offset].index
                    ) == storyIndex
                ) {
                    storyFound = true;
                    break;
                }
            }
        }

        if (storyInfo.data != nullptr)
            BMKillHandle ((GSHandle*) &storyInfo.data);

        if (!storyFound) {
            return MakeResponse (
                false,
                false,
                -10003,
                GS::UniString ("Requested story does not exist."),
                GS::UniString (),
                0,
                0
            );
        }

        GS::Array<API_Guid> wallsBefore;

        const GSErrCode countError = ACAPI_Element_GetElemList (
            API_WallID,
            &wallsBefore
        );

        if (countError != NoError) {
            return MakeResponse (
                false,
                false,
                static_cast<Int32> (countError),
                GS::UniString ("Unable to count existing walls."),
                GS::UniString (),
                0,
                0
            );
        }

        const Int32 wallCountBefore =
            static_cast<Int32> (wallsBefore.GetSize ());

        if (wallCountBefore != expectedWallCount) {
            return MakeResponse (
                false,
                false,
                -10004,
                GS::UniString ("Wall count precondition failed."),
                GS::UniString (),
                wallCountBefore,
                wallCountBefore
            );
        }

        API_Element wallElement {};
        wallElement.header.typeID = API_ElemTypeID::API_WallID;

        const GSErrCode defaultsError =
            ACAPI_Element_GetDefaults (&wallElement, nullptr);

        if (defaultsError != NoError) {
            return MakeResponse (
                false,
                false,
                static_cast<Int32> (defaultsError),
                GS::UniString ("Unable to read default Wall settings."),
                GS::UniString (),
                wallCountBefore,
                wallCountBefore
            );
        }

        wallElement.header.floorInd =
            static_cast<short> (storyIndex);

        wallElement.wall.begC = {
            startXmm / 1000.0,
            startYmm / 1000.0
        };

        wallElement.wall.endC = {
            endXmm / 1000.0,
            endYmm / 1000.0
        };

        wallElement.wall.angle = 0.0;
        wallElement.wall.relativeTopStory = 0;
        wallElement.wall.height = heightMm / 1000.0;
        wallElement.wall.thickness = thicknessMm / 1000.0;

        if (dryRun) {
            return MakeResponse (
                true,
                false,
                0,
                GS::UniString ("Validation passed. No wall was created."),
                GS::UniString (),
                wallCountBefore,
                wallCountBefore
            );
        }

        const GSErrCode createError =
            ACAPI_CallUndoableCommand (
                "ShowaBridge Create Wall",
                [&] () -> GSErrCode {
                    return ACAPI_Element_Create (
                        &wallElement,
                        nullptr
                    );
                }
            );

        if (createError != NoError) {
            return MakeResponse (
                false,
                false,
                static_cast<Int32> (createError),
                GS::UniString ("Archicad failed to create the wall."),
                GS::UniString (),
                wallCountBefore,
                wallCountBefore
            );
        }

        Int32 wallCountAfter = wallCountBefore + 1;
        GS::Array<API_Guid> wallsAfter;

        if (
            ACAPI_Element_GetElemList (
                API_WallID,
                &wallsAfter
            ) == NoError
        ) {
            wallCountAfter =
                static_cast<Int32> (wallsAfter.GetSize ());
        }

        return MakeResponse (
            true,
            true,
            0,
            GS::UniString ("Wall created successfully."),
            APIGuidToString (wallElement.header.guid),
            wallCountBefore,
            wallCountAfter
        );
    }

    void OnResponseValidationFailed (
        const GS::ObjectState& /* response */
    ) const override
    {
    }

private:
    static GS::ObjectState MakeResponse (
        bool ok,
        bool created,
        Int32 errorCode,
        const GS::UniString& message,
        const GS::UniString& guid,
        Int32 wallCountBefore,
        Int32 wallCountAfter
    )
    {
        GS::ObjectState response;

        response.Add ("ok", ok);
        response.Add ("created", created);
        response.Add ("errorCode", errorCode);
        response.Add ("message", message);
        response.Add ("guid", guid);
        response.Add ("wallCountBefore", wallCountBefore);
        response.Add ("wallCountAfter", wallCountAfter);

        return response;
    }
};

} // namespace

namespace {

class CreateSlabCommand final : public API_AddOnCommand
{
public:
    GS::String GetName () const override
    {
        return "CreateSlab";
    }

    GS::String GetNamespace () const override
    {
        return "ShowaBridge";
    }

    GS::Optional<GS::UniString> GetSchemaDefinitions () const override
    {
        return GS::NoValue;
    }

    GS::Optional<GS::UniString> GetInputParametersSchema () const override
    {
        return GS::UniString (R"({
            "type": "object",
            "properties": {
                "originXmm": { "type": "number" },
                "originYmm": { "type": "number" },
                "widthMm": { "type": "number" },
                "depthMm": { "type": "number" },
                "thicknessMm": { "type": "number" },
                "storyIndex": { "type": "integer" },
                "expectedSlabCount": { "type": "integer" },
                "dryRun": { "type": "boolean" }
            },
            "required": [
                "originXmm",
                "originYmm",
                "widthMm",
                "depthMm",
                "thicknessMm",
                "storyIndex",
                "expectedSlabCount",
                "dryRun"
            ],
            "additionalProperties": false
        })");
    }

    GS::Optional<GS::UniString> GetResponseSchema () const override
    {
        return GS::UniString (R"({
            "type": "object",
            "properties": {
                "ok": { "type": "boolean" },
                "created": { "type": "boolean" },
                "errorCode": { "type": "integer" },
                "message": { "type": "string" },
                "guid": { "type": "string" },
                "slabCountBefore": { "type": "integer" },
                "slabCountAfter": { "type": "integer" }
            },
            "required": [
                "ok",
                "created",
                "errorCode",
                "message",
                "guid",
                "slabCountBefore",
                "slabCountAfter"
            ],
            "additionalProperties": false
        })");
    }

    API_AddOnCommandExecutionPolicy GetExecutionPolicy () const override
    {
        return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread;
    }

    GS::ObjectState Execute (
        const GS::ObjectState& parameters,
        GS::ProcessControl& /* processControl */
    ) const override
    {
        double originXmm = 0.0;
        double originYmm = 0.0;
        double widthMm = 0.0;
        double depthMm = 0.0;
        double thicknessMm = 0.0;
        Int32 storyIndex = 0;
        Int32 expectedSlabCount = 0;
        bool dryRun = false;

        if (!parameters.Get ("originXmm", originXmm) ||
            !parameters.Get ("originYmm", originYmm) ||
            !parameters.Get ("widthMm", widthMm) ||
            !parameters.Get ("depthMm", depthMm) ||
            !parameters.Get ("thicknessMm", thicknessMm) ||
            !parameters.Get ("storyIndex", storyIndex) ||
            !parameters.Get ("expectedSlabCount", expectedSlabCount) ||
            !parameters.Get ("dryRun", dryRun)) {
            return MakeResponse (
                false,
                false,
                -10101,
                GS::UniString ("Invalid or missing input parameters."),
                GS::UniString (),
                0,
                0
            );
        }

        if (widthMm < 1.0 ||
            widthMm > 1000000.0 ||
            depthMm < 1.0 ||
            depthMm > 1000000.0 ||
            thicknessMm < 10.0 ||
            thicknessMm > 2000.0 ||
            expectedSlabCount < 0) {
            return MakeResponse (
                false,
                false,
                -10102,
                GS::UniString ("Slab dimensions or expected count are invalid."),
                GS::UniString (),
                0,
                0
            );
        }

        API_StoryInfo storyInfo {};
        const GSErrCode storyError = ACAPI_Environment (
            APIEnv_GetStorySettingsID,
            &storyInfo,
            nullptr
        );

        if (storyError != NoError) {
            if (storyInfo.data != nullptr)
                BMKillHandle ((GSHandle*) &storyInfo.data);

            return MakeResponse (
                false,
                false,
                static_cast<Int32> (storyError),
                GS::UniString ("Unable to read story settings."),
                GS::UniString (),
                0,
                0
            );
        }

        bool storyFound = false;

        if (storyInfo.data != nullptr) {
            const Int32 storyCount =
                static_cast<Int32> (
                    storyInfo.lastStory - storyInfo.firstStory
                ) + 1;

            for (Int32 offset = 0; offset < storyCount; ++offset) {
                if (static_cast<Int32> ((*storyInfo.data)[offset].index) == storyIndex) {
                    storyFound = true;
                    break;
                }
            }
        }

        if (storyInfo.data != nullptr)
            BMKillHandle ((GSHandle*) &storyInfo.data);

        if (!storyFound) {
            return MakeResponse (
                false,
                false,
                -10103,
                GS::UniString ("Requested story does not exist."),
                GS::UniString (),
                0,
                0
            );
        }

        GS::Array<API_Guid> slabsBefore;
        const GSErrCode countError = ACAPI_Element_GetElemList (
            API_SlabID,
            &slabsBefore
        );

        if (countError != NoError) {
            return MakeResponse (
                false,
                false,
                static_cast<Int32> (countError),
                GS::UniString ("Unable to count existing slabs."),
                GS::UniString (),
                0,
                0
            );
        }

        const Int32 slabCountBefore =
            static_cast<Int32> (slabsBefore.GetSize ());

        if (slabCountBefore != expectedSlabCount) {
            return MakeResponse (
                false,
                false,
                -10104,
                GS::UniString ("Slab count precondition failed."),
                GS::UniString (),
                slabCountBefore,
                slabCountBefore
            );
        }

        API_Element slabElement {};
        slabElement.header.typeID = API_ElemTypeID::API_SlabID;

        const GSErrCode defaultsError =
            ACAPI_Element_GetDefaults (&slabElement, nullptr);

        if (defaultsError != NoError) {
            return MakeResponse (
                false,
                false,
                static_cast<Int32> (defaultsError),
                GS::UniString ("Unable to read default Slab settings."),
                GS::UniString (),
                slabCountBefore,
                slabCountBefore
            );
        }

        slabElement.header.floorInd = static_cast<short> (storyIndex);
        slabElement.slab.thickness = thicknessMm / 1000.0;
        slabElement.slab.poly.nCoords = 5;
        slabElement.slab.poly.nSubPolys = 1;
        slabElement.slab.poly.nArcs = 0;

        API_ElementMemo memo {};
        memo.coords = reinterpret_cast<API_Coord**> (
            BMAllocateHandle (
                (slabElement.slab.poly.nCoords + 1) * sizeof (API_Coord),
                ALLOCATE_CLEAR,
                0
            )
        );
        memo.pends = reinterpret_cast<Int32**> (
            BMAllocateHandle (
                (slabElement.slab.poly.nSubPolys + 1) * sizeof (Int32),
                ALLOCATE_CLEAR,
                0
            )
        );
        memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (
            BMAllocateHandle (
                (slabElement.slab.poly.nCoords + 1) * sizeof (API_EdgeTrim),
                ALLOCATE_CLEAR,
                0
            )
        );
        memo.sideMaterials = reinterpret_cast<API_OverriddenAttribute*> (
            BMAllocatePtr (
                (slabElement.slab.poly.nCoords + 1) * sizeof (API_OverriddenAttribute),
                ALLOCATE_CLEAR,
                0
            )
        );

        if (memo.coords == nullptr ||
            memo.pends == nullptr ||
            memo.edgeTrims == nullptr ||
            memo.sideMaterials == nullptr) {
            ACAPI_DisposeElemMemoHdls (&memo);

            return MakeResponse (
                false,
                false,
                static_cast<Int32> (APIERR_MEMFULL),
                GS::UniString ("Not enough memory to prepare the slab polygon."),
                GS::UniString (),
                slabCountBefore,
                slabCountBefore
            );
        }

        const double originX = originXmm / 1000.0;
        const double originY = originYmm / 1000.0;
        const double width = widthMm / 1000.0;
        const double depth = depthMm / 1000.0;

        (*memo.coords)[1] = { originX, originY };
        (*memo.coords)[2] = { originX + width, originY };
        (*memo.coords)[3] = { originX + width, originY + depth };
        (*memo.coords)[4] = { originX, originY + depth };
        (*memo.coords)[5] = (*memo.coords)[1];
        (*memo.pends)[1] = slabElement.slab.poly.nCoords;

        for (Int32 coordinateIndex = 1;
             coordinateIndex <= slabElement.slab.poly.nCoords;
            ++coordinateIndex) {
            (*memo.edgeTrims)[coordinateIndex].sideType = APIEdgeTrim_Vertical;
            (*memo.edgeTrims)[coordinateIndex].sideAngle = 0.0;
            memo.sideMaterials[coordinateIndex] = slabElement.slab.sideMat;
        }

        if (dryRun) {
            ACAPI_DisposeElemMemoHdls (&memo);

            return MakeResponse (
                true,
                false,
                0,
                GS::UniString ("Validation passed. No slab was created."),
                GS::UniString (),
                slabCountBefore,
                slabCountBefore
            );
        }

        const GSErrCode createError = ACAPI_CallUndoableCommand (
            "ShowaBridge Create Slab",
            [&] () -> GSErrCode {
                return ACAPI_Element_Create (&slabElement, &memo);
            }
        );

        ACAPI_DisposeElemMemoHdls (&memo);

        if (createError != NoError) {
            return MakeResponse (
                false,
                false,
                static_cast<Int32> (createError),
                GS::UniString ("Archicad failed to create the slab."),
                GS::UniString (),
                slabCountBefore,
                slabCountBefore
            );
        }

        Int32 slabCountAfter = slabCountBefore + 1;
        GS::Array<API_Guid> slabsAfter;

        if (ACAPI_Element_GetElemList (API_SlabID, &slabsAfter) == NoError) {
            slabCountAfter = static_cast<Int32> (slabsAfter.GetSize ());
        }

        return MakeResponse (
            true,
            true,
            0,
            GS::UniString ("Slab created successfully."),
            APIGuidToString (slabElement.header.guid),
            slabCountBefore,
            slabCountAfter
        );
    }

    void OnResponseValidationFailed (
        const GS::ObjectState& /* response */
    ) const override
    {
    }

private:
    static GS::ObjectState MakeResponse (
        bool ok,
        bool created,
        Int32 errorCode,
        const GS::UniString& message,
        const GS::UniString& guid,
        Int32 slabCountBefore,
        Int32 slabCountAfter
    )
    {
        GS::ObjectState response;
        response.Add ("ok", ok);
        response.Add ("created", created);
        response.Add ("errorCode", errorCode);
        response.Add ("message", message);
        response.Add ("guid", guid);
        response.Add ("slabCountBefore", slabCountBefore);
        response.Add ("slabCountAfter", slabCountAfter);
        return response;
    }
};

} // namespace

API_AddonType CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, AddOnInfoID, AddOnNameID, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, AddOnInfoID, AddOnDescriptionID, ACAPI_GetOwnResModule ());
	return APIAddon_Preload;
}


GSErrCode RegisterInterface (void)
{
	return NoError;
}


GSErrCode Initialize (void)
{
    GSErrCode err = ACAPI_Install_AddOnCommandHandler (
        GS::NewOwned<PingCommand> ()
    );

    if (err != NoError)
        return err;

    err = ACAPI_Install_AddOnCommandHandler (
        GS::NewOwned<GetElementCountsCommand> ()
    );

    if (err != NoError)
        return err;

    err = ACAPI_Install_AddOnCommandHandler (
        GS::NewOwned<GetProjectInfoCommand> ()
    );

    if (err != NoError)
        return err;

    err = ACAPI_Install_AddOnCommandHandler (
        GS::NewOwned<GetStoriesCommand> ()
    );

    if (err != NoError)
        return err;

    err = ACAPI_Install_AddOnCommandHandler (
        GS::NewOwned<CreateWallCommand> ()
    );

    if (err != NoError)
        return err;

    err = ACAPI_Install_AddOnCommandHandler (
        GS::NewOwned<CreateSlabCommand> ()
    );
    if (err != NoError)
        return err;
    return ACAPI_Install_AddOnCommandHandler (
        GS::NewOwned<GetElementInfoCommand> ()
    );
}


GSErrCode FreeData (void)
{
	return NoError;
}
