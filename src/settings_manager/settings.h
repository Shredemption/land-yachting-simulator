#ifndef SETTINGS_H
#define SETTINGS_H

#include <jsoncons/json.hpp>

struct Settings
{
    struct Debug
    {
        bool wireframeMode = false;
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
            return s;
        }

        static Json to_json(const Settings::Debug &s)
        {
            Json j;
            j["wireframeMode"] = s.wireframeMode;
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

#endif