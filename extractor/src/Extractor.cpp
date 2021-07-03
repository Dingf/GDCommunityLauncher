#include <filesystem>
#include "Extractor.h"

Napi::Value ExtractARZ(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if ((info.Length() != 2) || (!info[0].IsString()) || (!info[1].IsString()))
    {
        Napi::TypeError error = Napi::TypeError::New(env, "ExtractARZ() expects 2 argument(s):\n\tstring gamePath, string outputPath");
        error.ThrowAsJavaScriptException();
        return Napi::Value::From(env, false);
    }

    std::filesystem::path gamePath = std::string(info[0].As<Napi::String>());
    std::filesystem::path outputPath = std::string(info[1].As<Napi::String>());

    std::filesystem::path toolPath = gamePath / "ArchiveTool.exe";
    std::filesystem::path basePath = gamePath / "database" / "database.arz";
    std::filesystem::path GDX1Path = gamePath / "gdx1" / "database" / "GDX1.arz";
    std::filesystem::path GDX2Path = gamePath / "gdx2" / "database" / "GDX2.arz";
    std::filesystem::path GDCLPath = gamePath / "mods" / "GrimLeagueS02_HC" / "database" / "GrimLeagueS02_HC.arz";  //TODO: Update for S3 mod

    // Extract Base Game
    std::string cmd = "\"\"" + toolPath.string() + "\" \"" + basePath.string() + "\" -database \"" + outputPath.string() + "\"\"";
    if (system(cmd.c_str()) != EXIT_SUCCESS)
    {
        Napi::TypeError error = Napi::TypeError::New(env, "Failed to extract the Grim Dawn database");
        error.ThrowAsJavaScriptException();
        return Napi::Value::From(env, false);
    }

    // Extract AoM
    cmd = "\"\"" + toolPath.string() + "\" \"" + GDX1Path.string() + "\" -database \"" + outputPath.string() + "\"\"";
    if (system(cmd.c_str()) != EXIT_SUCCESS)
    {
        Napi::TypeError error = Napi::TypeError::New(env, "Failed to extract the Ashes of Malmouth database");
        error.ThrowAsJavaScriptException();
        return Napi::Value::From(env, false);
    }

    // Extract FG
    cmd = "\"\"" + toolPath.string() + "\" \"" + GDX2Path.string() + "\" -database \"" + outputPath.string() + "\"\"";
    if (system(cmd.c_str()) != EXIT_SUCCESS)
    {
        Napi::TypeError error = Napi::TypeError::New(env, "Failed to extract the Forgotten Gods database");
        error.ThrowAsJavaScriptException();
        return Napi::Value::From(env, false);
    }

    // Extract GrimLeague Mod
    cmd = "\"\"" + toolPath.string() + "\" \"" + GDCLPath.string() + "\" -database \"" + outputPath.string() + "\"\"";
    if (system(cmd.c_str()) != EXIT_SUCCESS)
    {
        Napi::TypeError error = Napi::TypeError::New(env, "Failed to extract the GrimLeague Mod database");
        error.ThrowAsJavaScriptException();
        return Napi::Value::From(env, false);
    }

    return Napi::Value::From(env, true);
}

Napi::Value ExtractARC(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if ((info.Length() != 3) || (!info[0].IsString()) || (!info[1].IsString()) || (!info[2].IsString()))
    {
        Napi::TypeError error = Napi::TypeError::New(env, "ExtractARC() expects 3 argument(s):\n\tstring name, string gamePath, string outputPath");
        error.ThrowAsJavaScriptException();
        return Napi::Value::From(env, false);
    }

    std::string arcName = std::string(info[0].As<Napi::String>()) + ".arc";
    std::filesystem::path gamePath = std::string(info[1].As<Napi::String>());
    std::filesystem::path outputPath = std::string(info[2].As<Napi::String>());

    std::filesystem::path toolPath = gamePath / "ArchiveTool.exe";
    std::filesystem::path basePath = gamePath / "resources" / arcName;
    std::filesystem::path GDX1Path = gamePath / "gdx1" / "resources" / arcName;
    std::filesystem::path GDX2Path = gamePath / "gdx2" / "resources" / arcName;
    std::filesystem::path GDCLPath = gamePath / "mods" / "GrimLeagueS02_HC" / "resources" / arcName;  //TODO: Update for S3 mod

    // Extract Base Game
    std::string cmd = "\"\"" + toolPath.string() + "\" \"" + basePath.string() + "\" -extract \"" + outputPath.string() + "\"\"";
    if (system(cmd.c_str()) != EXIT_SUCCESS)
    {
        Napi::TypeError error = Napi::TypeError::New(env, "Failed to extract the Grim Dawn archive");
        error.ThrowAsJavaScriptException();
        return Napi::Value::From(env, false);
    }

    // Extract AoM
    cmd = "\"\"" + toolPath.string() + "\" \"" + GDX1Path.string() + "\" -extract \"" + outputPath.string() + "\"\"";
    if (system(cmd.c_str()) != EXIT_SUCCESS)
    {
        Napi::TypeError error = Napi::TypeError::New(env, "Failed to extract the Ashes of Malmouth archive");
        error.ThrowAsJavaScriptException();
        return Napi::Value::From(env, false);
    }

    // Extract FG
    cmd = "\"\"" + toolPath.string() + "\" \"" + GDX2Path.string() + "\" -extract \"" + outputPath.string() + "\"\"";
    if (system(cmd.c_str()) != EXIT_SUCCESS)
    {
        Napi::TypeError error = Napi::TypeError::New(env, "Failed to extract the Forgotten Gods archive");
        error.ThrowAsJavaScriptException();
        return Napi::Value::From(env, false);
    }

    // Extract GrimLeague Mod
    if (std::filesystem::is_regular_file(GDCLPath))
    {
        cmd = "\"\"" + toolPath.string() + "\" \"" + GDCLPath.string() + "\" -extract \"" + outputPath.string() + "\"\"";
        if (system(cmd.c_str()) != EXIT_SUCCESS)
        {
            Napi::TypeError error = Napi::TypeError::New(env, "Failed to extract the GrimLeague Mod archive");
            error.ThrowAsJavaScriptException();
            return Napi::Value::From(env, false);
        }
    }

    return Napi::Value::From(env, true);
}

Napi::Object Extractor::InitClass(Napi::Env& env, Napi::Object& exports)
{
    exports.Set("ExtractARZ", Napi::Function::New(env, ExtractARZ));
    exports.Set("ExtractARC", Napi::Function::New(env, ExtractARC));

    return exports;
}