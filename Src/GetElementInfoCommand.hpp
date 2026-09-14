#pragma once

#include "ACAPinc.h"
#include "ObjectState.hpp"
#include <cctype>
#include <cstring>

// Read-only slab inspection. All exported lengths are millimetres.
class GetElementInfoCommand final : public API_AddOnCommand {
public:
    GS::String GetName () const override { return "GetElementInfo"; }
    GS::String GetNamespace () const override { return "ShowaBridge"; }
    GS::Optional<GS::UniString> GetSchemaDefinitions () const override { return GS::NoValue; }
    GS::Optional<GS::UniString> GetInputParametersSchema () const override
    {
        return GS::UniString (R"({"type":"object","properties":{"guid":{"type":"string"}},
            "required":["guid"],"additionalProperties":false})");
    }
    GS::Optional<GS::UniString> GetResponseSchema () const override
    {
        return GS::UniString (R"({"type":"object","properties":{
            "ok":{"type":"boolean"},"errorCode":{"type":"integer"},
            "message":{"type":"string"},"guid":{"type":"string"},
            "elementType":{"type":"string"},"storyIndex":{"type":"integer"},
            "thicknessMm":{"type":"number"},"levelOffsetMm":{"type":"number"},
            "storyElevationMm":{"type":"number"},"referenceElevationMm":{"type":"number"},
            "referencePlaneLocation":{"type":"integer"},
            "polygon":{"type":"object","properties":{
                "coordinates":{"type":"array","items":{"type":"object","properties":{
                    "xMm":{"type":"number"},"yMm":{"type":"number"}},
                    "required":["xMm","yMm"],"additionalProperties":false}},
                "contourEnds":{"type":"array","items":{"type":"integer"}},
                "arcs":{"type":"array","items":{"type":"object","properties":{
                    "beginIndex":{"type":"integer"},"endIndex":{"type":"integer"},
                    "angleRadians":{"type":"number"}},
                    "required":["beginIndex","endIndex","angleRadians"],"additionalProperties":false}}
            },"required":["coordinates","contourEnds","arcs"],"additionalProperties":false}
        },"required":["ok","errorCode","message","guid"],"additionalProperties":false})");
    }
    API_AddOnCommandExecutionPolicy GetExecutionPolicy () const override
    {
        return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread;
    }
    GS::ObjectState Execute (const GS::ObjectState& parameters, GS::ProcessControl&) const override
    {
        GS::String text;
        if (!parameters.Get ("guid", text))
            return Error (-10201, "Missing GUID.");
        const char* chars = text.ToCStr ();
        if (std::strlen (chars) != 36)
            return Error (-10201, "GUID must use the 36-character UUID format.");
        for (int i = 0; i < 36; ++i) {
            const bool separator = i == 8 || i == 13 || i == 18 || i == 23;
            if (separator ? chars[i] != '-' : !std::isxdigit (static_cast<unsigned char> (chars[i])))
                return Error (-10201, "Invalid GUID format.");
        }
        API_Element element {};
        element.header.guid = APIGuidFromString (chars);
        if (element.header.guid == APINULLGuid)
            return Error (-10201, "Null GUID is not an element.");
        GSErrCode err = ACAPI_Element_Get (&element);
        if (err != NoError)
            return Error (err, "Unable to read element; it may have been deleted.");
        if (element.header.typeID != API_SlabID)
            return Error (-10202, "GetElementInfo currently supports slabs only.");

        API_StoryInfo stories {};
        err = ACAPI_Environment (APIEnv_GetStorySettingsID, &stories, nullptr);
        double storyLevel = 0.0;
        bool found = false;
        if (err == NoError && stories.data != nullptr) {
            for (Int32 i = 0; i <= stories.lastStory - stories.firstStory; ++i) {
                if ((*stories.data)[i].index == element.header.floorInd) {
                    storyLevel = (*stories.data)[i].level;
                    found = true;
                    break;
                }
            }
        }
        if (stories.data != nullptr)
            BMKillHandle (reinterpret_cast<GSHandle*> (&stories.data));
        if (err != NoError || !found)
            return Error (err != NoError ? err : -10203, "Unable to resolve element story.");

        struct MemoOwner {
            API_ElementMemo value {};
            ~MemoOwner () { ACAPI_DisposeElemMemoHdls (&value); }
        } memo;
        err = ACAPI_Element_GetMemo (element.header.guid, &memo.value, APIMemoMask_Polygon);
        if (err != NoError)
            return Error (err, "Unable to read slab polygon.");
        const API_Polygon& poly = element.slab.poly;
        if (memo.value.coords == nullptr || memo.value.pends == nullptr ||
            (poly.nArcs > 0 && memo.value.parcs == nullptr))
            return Error (-10204, "Incomplete slab polygon memo.");

        GS::ObjectState polygon;
        const auto addCoordinate = polygon.AddList<GS::ObjectState> ("coordinates");
        for (Int32 i = 1; i <= poly.nCoords; ++i) {
            GS::ObjectState point;
            point.Add ("xMm", (*memo.value.coords)[i].x * 1000.0);
            point.Add ("yMm", (*memo.value.coords)[i].y * 1000.0);
            addCoordinate (point);
        }
        const auto addEnd = polygon.AddList<Int32> ("contourEnds");
        for (Int32 i = 1; i <= poly.nSubPolys; ++i)
            addEnd ((*memo.value.pends)[i]);
        const auto addArc = polygon.AddList<GS::ObjectState> ("arcs");
        for (Int32 i = 0; i < poly.nArcs; ++i) {
            GS::ObjectState arc;
            arc.Add ("beginIndex", (*memo.value.parcs)[i].begIndex);
            arc.Add ("endIndex", (*memo.value.parcs)[i].endIndex);
            arc.Add ("angleRadians", (*memo.value.parcs)[i].arcAngle);
            addArc (arc);
        }
        GS::ObjectState response;
        response.Add ("ok", true);
        response.Add ("errorCode", 0);
        response.Add ("message", "Slab geometry read successfully.");
        response.Add ("guid", APIGuidToString (element.header.guid));
        response.Add ("elementType", "Slab");
        response.Add ("storyIndex", static_cast<Int32> (element.header.floorInd));
        response.Add ("thicknessMm", element.slab.thickness * 1000.0);
        response.Add ("levelOffsetMm", element.slab.level * 1000.0);
        response.Add ("storyElevationMm", storyLevel * 1000.0);
        response.Add ("referenceElevationMm", (storyLevel + element.slab.level) * 1000.0);
        response.Add ("referencePlaneLocation", static_cast<Int32> (element.slab.referencePlaneLocation));
        response.Add ("polygon", polygon);
        return response;
    }
    void OnResponseValidationFailed (const GS::ObjectState&) const override {}
private:
    static GS::ObjectState Error (Int32 code, const char* message)
    {
        GS::ObjectState result;
        result.Add ("ok", false);
        result.Add ("errorCode", code);
        result.Add ("message", message);
        result.Add ("guid", "");
        return result;
    }
};
