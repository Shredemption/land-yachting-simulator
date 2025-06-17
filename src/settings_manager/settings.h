#pragma once

#include <jsoncons/json.hpp>

enum class debugOverlay
{
    None,
    FPS,
    Physics
};

struct SettingsStruct
{
    struct Debug
    {
        bool wireframeMode = false;
        debugOverlay debugOverlay = debugOverlay::None;
    } debug;

    struct Video
    {
        bool fullscreen = true;
        bool vSync = true;
        float lodDistance = 50.0f;
        float waterFrameRate = 30.0f;
    } video;

    struct Input
    {
        float mouseSensitivity = 5.0f;
        float controllerCamSensitivity = 5.0f;
    } input;
};

namespace jsoncons
{
    using Json = jsoncons::json;

    template <>
    struct json_type_traits<Json, debugOverlay>
    {
        static bool is(const Json &j) noexcept
        {
            return j.is_string();
        }

        static debugOverlay as(const Json &j)
        {
            const std::string &s = j.template as<std::string>();
            if (s == "None")
                return debugOverlay::None;
            if (s == "FPS")
                return debugOverlay::FPS;
            if (s == "Physics")
                return debugOverlay::Physics;
            throw std::runtime_error("Invalid debugOverlay enum value: " + s);
        }

        static Json to_json(const debugOverlay &val)
        {
            switch (val)
            {
            case debugOverlay::None:
                return Json("None");
            case debugOverlay::FPS:
                return Json("FPS");
            case debugOverlay::Physics:
                return Json("Physics");
            default:
                return Json("Unknown");
            }
        }
    };

    template <>
    struct json_type_traits<Json, SettingsStruct::Debug>
    {
        static bool is(const Json &j) noexcept
        {
            return j.is_object();
        }

        static SettingsStruct::Debug as(const Json &j)
        {
            SettingsStruct::Debug s;
            if (j.contains("wireframeMode"))
                s.wireframeMode = j["wireframeMode"].template as<bool>();
            if (j.contains("debugOverlay"))
                s.debugOverlay = j["debugOverlay"].template as<debugOverlay>();
            return s;
        }

        static Json to_json(const SettingsStruct::Debug &s)
        {
            Json j;
            j["wireframeMode"] = s.wireframeMode;
            j["debugOverlay"] = s.debugOverlay;
            return j;
        }
    };

    template <>
    struct json_type_traits<Json, SettingsStruct::Video>
    {
        static bool is(const Json &j) noexcept
        {
            return j.is_object();
        }

        static SettingsStruct::Video as(const Json &j)
        {
            SettingsStruct::Video s;
            if (j.contains("fullscreen"))
                s.fullscreen = j["fullscreen"].template as<bool>();
            if (j.contains("vsync"))
                s.vSync = j["vsync"].template as<bool>();
            if (j.contains("lodDistance"))
                s.lodDistance = j["lodDistance"].template as<float>();
            if (j.contains("waterFrameRate"))
                s.waterFrameRate = j["waterFrameRate"].template as<float>();
            return s;
        }

        static Json to_json(const SettingsStruct::Video &s)
        {
            Json j;
            j["fullscreen"] = s.fullscreen;
            j["vsync"] = s.vSync;
            j["lodDistance"] = s.lodDistance;
            j["waterFrameRate"] = s.waterFrameRate;
            return j;
        }
    };

    template <>
    struct json_type_traits<Json, SettingsStruct::Input>
    {
        static bool is(const Json &j) noexcept
        {
            return j.is_object();
        }

        static SettingsStruct::Input as(const Json &j)
        {
            SettingsStruct::Input s;
            if (j.contains("mouseSensitiviy"))
                s.mouseSensitivity = j["mouseSensitiviy"].template as<float>();
            if (j.contains("controllerCamSensitivity"))
                s.controllerCamSensitivity = j["controllerCamSensitivity"].template as<float>();
            return s;
        }

        static Json to_json(const SettingsStruct::Input &s)
        {
            Json j;
            j["mouseSensitivity"] = s.mouseSensitivity;
            j["controllerCamSensitivity"] = s.controllerCamSensitivity;
            return j;
        }
    };

    template <>
    struct json_type_traits<Json, SettingsStruct>
    {
        static bool is(const Json &j) noexcept
        {
            return j.is_object();
        }

        static SettingsStruct as(const Json &j)
        {
            SettingsStruct s;
            if (j.contains("debug"))
                s.debug = j["debug"].template as<SettingsStruct::Debug>();
            if (j.contains("video"))
                s.video = j["video"].template as<SettingsStruct::Video>();
            if (j.contains("input"))
                s.input = j["input"].template as<SettingsStruct::Input>();
            return s;
        }

        static Json to_json(const SettingsStruct &s)
        {
            Json j;
            j["debug"] = s.debug;
            j["video"] = s.video;
            j["input"] = s.input;
            return j;
        }
    };
};