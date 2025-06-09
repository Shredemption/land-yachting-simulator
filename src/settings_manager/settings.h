#pragma once

#include <jsoncons/json.hpp>

enum class debugOverlay
{
    None,
    FPS,
    Physics
};

struct Settings
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
    } video;
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
    struct json_type_traits<Json, Settings::Debug>
    {
        static bool is(const Json &j) noexcept
        {
            return j.is_object();
        }

        static Settings::Debug as(const Json &j)
        {
            Settings::Debug s;
            if (j.contains("wireframeMode"))
                s.wireframeMode = j["wireframeMode"].template as<bool>();
            if (j.contains("debugOverlay"))
                s.debugOverlay = j["debugOverlay"].template as<debugOverlay>();
            return s;
        }

        static Json to_json(const Settings::Debug &s)
        {
            Json j;
            j["wireframeMode"] = s.wireframeMode;
            j["debugOverlay"] = s.debugOverlay;
            return j;
        }
    };

    template <>
    struct json_type_traits<Json, Settings::Video>
    {
        static bool is(const Json &j) noexcept
        {
            return j.is_object();
        }

        static Settings::Video as(const Json &j)
        {
            Settings::Video s;
            if (j.contains("fullscreen"))
                s.fullscreen = j["fullscreen"].template as<bool>();
            if (j.contains("vsync"))
                s.vSync = j["vsync"].template as<bool>();
            return s;
        }

        static Json to_json(const Settings::Video &s)
        {
            Json j;
            j["fullscreen"] = s.fullscreen;
            j["vsync"] = s.vSync;
            return j;
        }
    };

    template <>
    struct json_type_traits<Json, Settings>
    {
        static bool is(const Json &j) noexcept
        {
            return j.is_object();
        }

        static Settings as(const Json &j)
        {
            Settings s;
            if (j.contains("debug"))
                s.debug = j["debug"].template as<Settings::Debug>();
            if (j.contains("video"))
                s.video = j["video"].template as<Settings::Video>();
            return s;
        }

        static Json to_json(const Settings &s)
        {
            Json j;
            j["debug"] = s.debug;
            j["video"] = s.video;
            return j;
        }
    };
};