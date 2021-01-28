#include <iostream>
#include <fstream>

#include <vector>
#include <set>

#include "flower_defines.h"

#define PREPROCESSOR_JSON_IMPLEMENTATION
#include "preprocessor_json.h"

struct parsed_scene
{
    std::string Name;
    std::string File;
    
    std::string InitMethod;
    std::string UpdateMethod;
    std::string OnGUIMethod;
};

struct parse_scene_context
{
    json_tokenizer Tokenizer;
    
    std::vector<parsed_scene> ParsedScenes;
};

parse_scene_context InitParseSceneContext(char* FileName)
{
    parse_scene_context Result = {};
    
    Result.Tokenizer = InitTokenizer(FileName);
    
    json_token ParsedToken;
    GetToken(&Result.Tokenizer, &ParsedToken);
    
    Assert(ParsedToken.Type == JsonToken_OpenBlock);
    
    return(Result);
}

inline b32 StringsAreEqual(const char* A, const char* B)
{
    b32 Result = strcmp(A, B) == 0;
    
    return(Result);
}

void ParseSceneField(parse_scene_context* Context, parsed_scene* Scene)
{
    json_tokenizer* Tokenz = &Context->Tokenizer;
    
    // NOTE(Dima): Read scene file name
    std::string FieldName = JsonReadField(Tokenz);
    if(StringsAreEqual(FieldName.c_str(), "FileName"))
    {
        Scene->File = JsonReadString(Tokenz);
    }
    else if(StringsAreEqual(FieldName.c_str(), "Method_OnGUI"))
    {
        b32 HasOnGUI = JsonReadBool(Tokenz);
        if(HasOnGUI)
        {
            Scene->OnGUIMethod = Scene->Name + "_OnGUI";
        }
    }
    
    b32 IsEndOfBlock = ProcessEndOfField(Tokenz);
}

void ParseScene(parse_scene_context* Context)
{
    json_tokenizer* Tokenz = &Context->Tokenizer;
    
    parsed_scene Scene;
    
    // NOTE(Dima): Read scene name
    Scene.Name = JsonReadString(Tokenz);
    Scene.InitMethod = Scene.Name + "_Init";
    Scene.UpdateMethod = Scene.Name + "_Update";
    Scene.OnGUIMethod = "0";
    
    RequireToken(Tokenz, JsonToken_Colon);
    RequireToken(Tokenz, JsonToken_OpenBlock);
    
    while(CanParseNextField(Tokenz))
    {
        // NOTE(Dima): This should be empty body here
        ParseSceneField(Context, &Scene);
    }
    
    RequireToken(Tokenz, JsonToken_CloseBlock);
    
    Context->ParsedScenes.push_back(Scene);
    
    b32 IsEndOfBlock = ProcessEndOfField(Tokenz);
    
}

void ParseScenes(parse_scene_context* Context)
{
    while(CanParseNextField(&Context->Tokenizer))
    {
        ParseScene(Context);
    }
}

void OutputScenesMeta(parse_scene_context* Context, const char* FileName)
{
    std::ofstream OutFile(FileName, std::ofstream::trunc | std::ofstream::out);
    
    OutFile << "/*" << std::endl;
    OutFile << "\t This file was generated automatically." << std::endl;
    OutFile << "\t Do not modify it until you know what you're doing. :)" << std::endl;
    OutFile << "\t It contains information about a game modes, which." << std::endl;
    OutFile << "\t will be used by the engine. This data was extracted by." << std::endl;
    OutFile << "\t parser (preprocessor.cpp). modes.json was parsed." << std::endl;
    OutFile << "\t Modes.json contains an info about the game modes." << std::endl;
    OutFile << "\t You can add and modify your game modes there." << std::endl;
    OutFile << "\t I wish you all the best! 07 Jan 2021" << std::endl;
    OutFile << "*/" << std::endl;
    
    OutFile << "#ifndef META_SCENES_H" << std::endl;
    OutFile << "#define META_SCENES_H" << std::endl;
    OutFile << std::endl;
    
    // NOTE(Dima): Building set of file names
    std::set<std::string> FileNames;
    for(int SceneIndex = 0;
        SceneIndex < Context->ParsedScenes.size();
        SceneIndex++)
    {
        parsed_scene* Scene = &Context->ParsedScenes[SceneIndex];
        
        FileNames.insert(Scene->File);
    }
    
    // NOTE(Dima): Printing include file lines
    for(auto& FileName: FileNames)
    {
        OutFile << "#include \"" << FileName << "\"" << std::endl;
    }
    
    OutFile << std::endl;
    
    // NOTE(Dima): Printing scene names
    OutFile << "GLOBAL_VARIABLE const char* MetaScene_Names[] = " << std::endl;
    OutFile << "{" << std::endl;
    for(int SceneIndex = 0;
        SceneIndex < Context->ParsedScenes.size();
        SceneIndex++)
    {
        parsed_scene* Scene = &Context->ParsedScenes[SceneIndex];
        
        OutFile << "\t\"" << Scene->Name << "\"," << std::endl;
    }
    OutFile << "};" << std::endl << std::endl;
    
    // NOTE(Dima): Printing init functions
    OutFile << "GLOBAL_VARIABLE scene_init* MetaScene_InitFunctions[] = " << std::endl;
    OutFile << "{" << std::endl;
    for(int SceneIndex = 0;
        SceneIndex < Context->ParsedScenes.size();
        SceneIndex++)
    {
        parsed_scene* Scene = &Context->ParsedScenes[SceneIndex];
        
        OutFile << "\t" << Scene->InitMethod << "," << std::endl;
    }
    OutFile << "};" << std::endl << std::endl;
    
    // NOTE(Dima): Printing update functions
    OutFile << "GLOBAL_VARIABLE scene_update* MetaScene_UpdateFunctions[] = " << std::endl;
    OutFile << "{" << std::endl;
    for(int SceneIndex = 0;
        SceneIndex < Context->ParsedScenes.size();
        SceneIndex++)
    {
        parsed_scene* Scene = &Context->ParsedScenes[SceneIndex];
        
        OutFile << "\t" << Scene->UpdateMethod << "," << std::endl;
    }
    OutFile << "};" << std::endl << std::endl;
    
    
    // NOTE(Dima): Printing OnGUI functions
    OutFile << "GLOBAL_VARIABLE scene_ongui* MetaScene_OnGUIFunctions[] = " << std::endl;
    OutFile << "{" << std::endl;
    for(int SceneIndex = 0;
        SceneIndex < Context->ParsedScenes.size();
        SceneIndex++)
    {
        parsed_scene* Scene = &Context->ParsedScenes[SceneIndex];
        
        OutFile << "\t" << Scene->OnGUIMethod << "," << std::endl;
    }
    OutFile << "};" << std::endl << std::endl;
    
    
    // NOTE(Dima): Printing end of file
    OutFile << "#endif //META_GAME_MODES_H" << std::endl;
    
    OutFile.close();
}

int main(int ArgsCount, char** Args)
{
    int ModesFileArgIndex = 1;
    char* ModesFileName = Args[ModesFileArgIndex];
    
    // NOTE(Dima): Parsing scenes
    parse_scene_context SceneCtx = InitParseSceneContext(ModesFileName);
    
    ParseScenes(&SceneCtx);
    OutputScenesMeta(&SceneCtx, "../Code/meta_scenes.cpp");
    std::cout << "Parsing " << ModesFileName << " scenes successfully!" << std::endl;
    
    return(0);
}