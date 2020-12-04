GLOBAL_VARIABLE opengl_state OpenGL;

char* ReadFileAndNullTerminate(char* Path)
{
    if(Path == 0)
    {
        return 0;
    }
    
    SDL_RWops* File = SDL_RWFromFile(Path, "r");
    
    char* Text = 0;
    if(File)
    {
        i64 FileSize = SDL_RWsize(File);
        
        Text = (char*)malloc(FileSize + 1);
        
        SDL_RWread(File, Text, FileSize, 1);
        
        Text[FileSize] = 0;
        
        SDL_RWclose(File);
    }
    
    return(Text);
}

INTERNAL_FUNCTION GLuint OpenGLLoadProgram(char* VertexFilePath, 
                                           char* FragmentFilePath, 
                                           char* GeometryFilePath = 0) 
{
    char* VertexSource = ReadFileAndNullTerminate(VertexFilePath);
    char* FragmentSource = ReadFileAndNullTerminate(FragmentFilePath);
    char* GeometrySource = ReadFileAndNullTerminate(GeometryFilePath);
    
    char InfoLog[1024];
	int Success;
	
	GLuint VertexShader;
	GLuint FragmentShader;
	GLuint GeometryShader;
    GLuint Program;
    
	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VertexSource, 0);
	glCompileShader(VertexShader);
	
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(VertexShader, sizeof(InfoLog), 0, InfoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Error while loading vertex shader(%s)\n%s\n", 
                     VertexFilePath, InfoLog);
    }
    
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FragmentSource, 0);
	glCompileShader(FragmentShader);
    
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(FragmentShader, sizeof(InfoLog), 0, InfoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Error while loading fragment shader(%s)\n%s\n", 
                     FragmentFilePath, InfoLog);
    }
    
    if(GeometrySource){
        GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(GeometryShader, 1, &GeometrySource, 0);
        glCompileShader(GeometryShader);
        
        glGetShaderiv(GeometryShader, GL_COMPILE_STATUS, &Success);
        if (!Success) {
            glGetShaderInfoLog(GeometryShader, sizeof(InfoLog), 0, InfoLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                         "Error while loading geometry shader(%s)\n%s\n", 
                         GeometryFilePath, InfoLog);
        }
    }
    
	Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	if(GeometrySource){
        glAttachShader(Program, GeometryShader);
    }
    glLinkProgram(Program);
    
	glGetProgramiv(Program, GL_LINK_STATUS, &Success);
	if (!Success)
    {
		glGetProgramInfoLog(Program, sizeof(InfoLog), 0, InfoLog);
		//TODO(dima): Logging
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Error while linking shader program\n%s\n", InfoLog);
        
        Program = -1;
	}
    
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
    if(GeometrySource){
        glDeleteShader(GeometryShader);
    }
    
    free(VertexSource);
    free(FragmentSource);
    if(GeometrySource)
    {
        free(GeometrySource);
    }
    
    return(Program);
}

INTERNAL_FUNCTION opengl_shader OpenGLLoadShader(char* ShaderName, 
                                                 char* VertexFilePath, 
                                                 char* FragmentFilePath, 
                                                 char* GeometryFilePath = 0)
{
    opengl_shader Result = {};
    
    Result.ID = OpenGLLoadProgram( VertexFilePath, 
                                  FragmentFilePath,
                                  GeometryFilePath);
    
    if(Result.ID == -1)
    {
        SDL_Log("%s shader loaded successfully!\n", ShaderName);
    }
    
    CopyStringsSafe(Result.Name, ArrayCount(Result.Name), ShaderName);
    
    // NOTE(Dima): Loading attributes
    OPENGL_LOAD_ATTRIB(Position);
    OPENGL_LOAD_ATTRIB(TexCoords);
    OPENGL_LOAD_ATTRIB(Normal);
    OPENGL_LOAD_ATTRIB(Color);
    
    // NOTE(Dima): Loading uniforms
    OPENGL_LOAD_UNIFORM(Projection);
    OPENGL_LOAD_UNIFORM(View);
    OPENGL_LOAD_UNIFORM(Model);
    
    OPENGL_LOAD_UNIFORM(MultColor);
    OPENGL_LOAD_UNIFORM(TexDiffuse);
    
    OPENGL_LOAD_UNIFORM(IsImage);
    OPENGL_LOAD_UNIFORM(Image);
    OPENGL_LOAD_UNIFORM(GlyphsColors);
    OPENGL_LOAD_UNIFORM(GlyphsTypes);
    OPENGL_LOAD_UNIFORM(Is3D);
    OPENGL_LOAD_UNIFORM(IsBatch);
    OPENGL_LOAD_UNIFORM(GlyphsTransforms);
    OPENGL_LOAD_UNIFORM(GlyphsIndicesToTransforms);
    OPENGL_LOAD_UNIFORM(GlyphDefaultProjectionMatrixIndex);
    OPENGL_LOAD_UNIFORM(GlyphOrthoMatrixIndex);
    OPENGL_LOAD_UNIFORM(GlyphPerspMatrixIndex);
    
    return(Result);
}

INTERNAL_FUNCTION void OpenGLDeleteShader(opengl_shader* Shader)
{
    glDeleteProgram(Shader->ID);
    Shader->ID = -1;
}

INTERNAL_FUNCTION inline b32 ArrayIsValid(GLint Arr){
    b32 Result = 1;
    
    if(Arr == -1){
        Result = 0;
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline b32 OpenGLShouldInitImage(image* Image)
{
    b32 Result = Image->ApiHandle == 0;
    
    return(Result);
}

INTERNAL_FUNCTION GLuint OpenGLInitImage(image* Image)
{
    GLuint TexOpenGL = 0;
    
    if(OpenGLShouldInitImage(Image))
    {
        glGenTextures(1, &TexOpenGL);
        glBindTexture(GL_TEXTURE_2D, TexOpenGL);
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     Image->Width,
                     Image->Height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     Image->Pixels);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        Image->ApiHandle = TexOpenGL;
    }
    
    return(TexOpenGL);
}

INTERNAL_FUNCTION void InitAttribFloat(GLint AttrLoc, 
                                       int ComponentCount,
                                       size_t Stride,
                                       size_t Offset)
{
    if(ArrayIsValid(AttrLoc)){
        glEnableVertexAttribArray(AttrLoc);
        glVertexAttribPointer(AttrLoc,
                              ComponentCount, 
                              GL_FLOAT, 
                              GL_FALSE,
                              Stride, 
                              (GLvoid*)(Offset));
    }
}

INTERNAL_FUNCTION void OpenGLCreateAndBindTextureBuffer(GLuint* Buffer,
                                                        GLuint* TBO,
                                                        mi Size,
                                                        void* Data,
                                                        GLuint DataFormat,
                                                        int TextureUnitIndex,
                                                        GLuint UniformLoc)
{
    glGenBuffers(1, Buffer);
    glGenTextures(1, TBO);
    
    glBindBuffer(GL_TEXTURE_BUFFER, *Buffer);
    glBufferData(GL_TEXTURE_BUFFER, 
                 Size, Data,
                 GL_STREAM_DRAW);
    
    glActiveTexture(GL_TEXTURE0 + TextureUnitIndex);
    glBindTexture(GL_TEXTURE_BUFFER, *TBO);
    glTexBuffer(GL_TEXTURE_BUFFER, DataFormat, *Buffer);
    
    glUniform1i(UniformLoc, TextureUnitIndex);
}

INTERNAL_FUNCTION void OpenGLInit()
{
    glewInit();
    
    SDL_GL_SetSwapInterval(0);
    
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    
    OpenGL.StdShader = OpenGLLoadShader("Standard",
                                        "../Data/Shaders/std.vs",
                                        "../Data/Shaders/std.fs");
    
    OpenGL.UIRectShader = OpenGLLoadShader("UIRect",
                                           "../Data/Shaders/ui_rect.vs",
                                           "../Data/Shaders/ui_rect.fs");
    
}

INTERNAL_FUNCTION void OpenGLFree()
{
    OpenGLDeleteShader(&OpenGL.StdShader);
}

INTERNAL_FUNCTION mesh_handles OpenGLAllocateMesh(mesh* Mesh, opengl_shader* Shader)
{
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    
    // NOTE(Dima): Init VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * Mesh->VertexCount, Mesh->Vertices, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Mesh->IndexCount, Mesh->Indices, GL_STREAM_DRAW);
    
    // NOTE(Dima): Position
    if(OpenGLAttribIsValid(Shader->PositionAttr))
    {
        glEnableVertexAttribArray(Shader->PositionAttr);
        glVertexAttribPointer(Shader->PositionAttr,
                              3, 
                              GL_FLOAT, 
                              GL_FALSE,
                              11 * sizeof(float), 
                              0);
    }
    
    // NOTE(Dima): TexCoords
    if(OpenGLAttribIsValid(Shader->TexCoordsAttr))
    {
        glEnableVertexAttribArray(Shader->TexCoordsAttr);
        glVertexAttribPointer(Shader->TexCoordsAttr,
                              2, 
                              GL_FLOAT, 
                              GL_FALSE,
                              11 * sizeof(float), 
                              (void*)(3 * sizeof(float)));
    }
    
    // NOTE(Dima): Normal
    if(OpenGLAttribIsValid(Shader->NormalAttr))
    {
        glEnableVertexAttribArray(Shader->NormalAttr);
        glVertexAttribPointer(Shader->NormalAttr,
                              3, 
                              GL_FLOAT, 
                              GL_FALSE,
                              11 * sizeof(float), 
                              (void*)(5 * sizeof(float)));
    }
    
    // NOTE(Dima): Color
    if(OpenGLAttribIsValid(Shader->ColorAttr))
    {
        glEnableVertexAttribArray(Shader->ColorAttr);
        glVertexAttribPointer(Shader->ColorAttr,
                              3, 
                              GL_FLOAT, 
                              GL_FALSE,
                              11* sizeof(float), 
                              (void*)(8 * sizeof(float)));
    }
    
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    mesh_handles Result = {};
    Result.ArrayObject = VAO;
    Result.BufferObject = VBO;
    Result.ElementBufferObject = EBO;
    
    return(Result);
}

INTERNAL_FUNCTION void OpenGLRenderMesh(opengl_shader* Shader, 
                                        v3 P, image* Image, 
                                        mesh* Mesh, 
                                        m44* View, 
                                        m44* Projection, 
                                        v3 Color)
{
    if(!MeshInitialized(Mesh))
    {
        Mesh->ApiHandles = OpenGLAllocateMesh(Mesh, &OpenGL.StdShader);
    }
    
    // NOTE(Dima): Render
    glBindVertexArray(Mesh->ApiHandles.ArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->ApiHandles.BufferObject);
    
    glUseProgram(OpenGL.StdShader.ID);
    
    m44 Model = TranslationMatrix(P);
    
    glUniform3f(Shader->MultColorLoc, Color.r, Color.g, Color.b);
    glUniformMatrix4fv(Shader->ProjectionLoc, 1, GL_TRUE, Projection->e);
    glUniformMatrix4fv(Shader->ViewLoc, 1, GL_TRUE, View->e);
    glUniformMatrix4fv(Shader->ModelLoc, 1, GL_TRUE, Model.e);
    
    OpenGLInitImage(Image);
    glUniform1i(Shader->TexDiffuseLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Image->ApiHandle);
    
    glDrawElements(GL_TRIANGLES, Mesh->IndexCount, GL_UNSIGNED_INT, 0);
    
    glUseProgram(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

INTERNAL_FUNCTION void OpenGLRenderImage(opengl_shader* Shader,
                                         image* Image, v2 P, 
                                         f32 Width, f32 Height, 
                                         v4 C, m44* Projection)
{
    glDisable(GL_DEPTH_TEST);
    
    GLfloat QuadData[] = {
        P.x, P.y, 0.0f, 0.0f,
        P.x + Width, P.y, 1.0f, 0.0f, 
        P.x + Width, P.y + Height, 1.0f, 1.0f,
        P.x, P.y + Height, 0.0f, 1.0f,
    };
    
    GLuint QuadIndices[] = 
    {
        0, 1, 2,
        0, 2, 3,
    };
    
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadData), QuadData, GL_STREAM_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QuadIndices), QuadIndices, GL_STREAM_DRAW);
    
    // NOTE(Dima): Position
    if(OpenGLAttribIsValid(OpenGL.UIRectShader.PositionAttr))
    {
        glEnableVertexAttribArray(OpenGL.UIRectShader.PositionAttr);
        glVertexAttribPointer(OpenGL.UIRectShader.PositionAttr,
                              2, 
                              GL_FLOAT, 
                              GL_FALSE,
                              4* sizeof(float), 
                              0);
    }
    
    // NOTE(Dima): TexCoords
    if(OpenGLAttribIsValid(OpenGL.UIRectShader.TexCoordsAttr))
    {
        glEnableVertexAttribArray(OpenGL.UIRectShader.TexCoordsAttr);
        glVertexAttribPointer(OpenGL.UIRectShader.TexCoordsAttr,
                              2, 
                              GL_FLOAT, 
                              GL_FALSE,
                              4* sizeof(float), 
                              (void*)(2 * sizeof(float)));
    }
    
    
    glUseProgram(OpenGL.UIRectShader.ID);
    
    m44 TransformsBuffer[] = {
        *Projection,
        Identity(),
    };
    
    u16 IndicesToTransform[1] = {1};
    
    // NOTE(Dima): Creating and binding Glyphs transforms buffer
    GLuint TransBuffer, TransTBO;
    OpenGLCreateAndBindTextureBuffer(&TransBuffer, &TransTBO,
                                     sizeof(TransformsBuffer),
                                     &TransformsBuffer[0],
                                     GL_RGBA32F,
                                     3, Shader->GlyphsTransformsLoc);
    
    
    // NOTE(Dima): Creating and binding indices to transforms buffer
    GLuint IndexToTranBuffer, IndexToTranTBO;
    OpenGLCreateAndBindTextureBuffer(&IndexToTranBuffer, &IndexToTranTBO,
                                     sizeof(IndicesToTransform),
                                     &IndicesToTransform[0],
                                     GL_R16UI,
                                     4, Shader->GlyphsIndicesToTransformsLoc);
    
    glUniform1i(Shader->GlyphDefaultProjectionMatrixIndexLoc, 0);
    glUniform1i(Shader->GlyphOrthoMatrixIndexLoc, 0);
    glUniform1i(Shader->GlyphPerspMatrixIndexLoc, 0);
    
    glUniformMatrix4fv(Shader->ProjectionLoc, 1, GL_TRUE, Projection->e);
    glUniform4f(Shader->MultColorLoc, C.r, C.g, C.b, C.a);
    glUniform1i(Shader->IsBatchLoc, false);
    glUniform1i(Shader->Is3DLoc, false);
    
    b32 IsImage = Image != 0;
    if(IsImage)
    {
        OpenGLInitImage(Image);
        
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(Shader->ImageLoc, 0);
        glBindTexture(GL_TEXTURE_2D, Image->ApiHandle);
    }
    glUniform1i(Shader->IsImageLoc, IsImage);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glUseProgram(0);
    
    glBindVertexArray(0);
    
    // NOTE(Dima): Free everything we need
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    
    glDeleteBuffers(1, &TransBuffer); 
    glDeleteTextures(1, &TransTBO);
    
    glDeleteBuffers(1, &IndexToTranBuffer); 
    glDeleteTextures(1, &IndexToTranTBO);
    
    glEnable(GL_DEPTH_TEST);
}

INTERNAL_FUNCTION void OpenGLRenderGlyph(v2 P, v2 Dim,
                                         v2 MinUV, v2 MaxUV)
{
    
}

INTERNAL_FUNCTION void OpenGLRenderImage3D()
{
    
}

INTERNAL_FUNCTION void OpenGLRenderCommands(render_params* RenderParams, 
                                            render_commands* Commands)
{
    
    for(int CommandIndex = 0;
        CommandIndex < Commands->CommandCount;
        CommandIndex++)
    {
        render_command_header* Header = &Commands->CommandHeaders[CommandIndex];
        
        switch(Header->CommandType)
        {
            case RenderCommand_Image:
            {
                render_command_image* ImageCommand = GetRenderCommand(Commands, CommandIndex, render_command_image);
                
                OpenGLRenderImage(&OpenGL.UIRectShader,
                                  ImageCommand->Image,
                                  ImageCommand->P,
                                  ImageCommand->Dim.x,
                                  ImageCommand->Dim.y,
                                  ImageCommand->C,
                                  RenderParams->ScreenOrthoProjection);
            } break;
            
            case RenderCommand_Image3D:
            {
                render_command_image* ImageCommand = GetRenderCommand(Commands, CommandIndex, render_command_image);
                
                OpenGLRenderImage3D();
            } break;
            
            case RenderCommand_Mesh:
            {
                render_command_mesh* MeshCommand = GetRenderCommand(Commands, CommandIndex, render_command_mesh);
                
                OpenGLRenderMesh(&OpenGL.StdShader,
                                 MeshCommand->P, 
                                 RenderParams->CubeTexture, 
                                 RenderParams->Cube, 
                                 RenderParams->View, 
                                 RenderParams->Projection, 
                                 MeshCommand->C);
            }break;
        }
    }
    
}

INTERNAL_FUNCTION void OpenGLRenderGlyphBuffer(render_params* RenderParams, 
                                               glyph_buffer* GlyphBuffer, 
                                               b32 Is3DRender,
                                               b32 UseDepthTest)
{
    opengl_shader* Shader = &OpenGL.UIRectShader;
    
    if(UseDepthTest == false)
    {
        glDisable(GL_DEPTH_TEST);
    }
    
    glUseProgram(OpenGL.UIRectShader.ID);
    
    glUniform4f(Shader->MultColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    glUniform1i(Shader->IsBatchLoc, true);
    glUniform1i(Shader->Is3DLoc, Is3DRender);
    
    glUniform1i(Shader->GlyphDefaultProjectionMatrixIndexLoc, GlyphBuffer->GlyphDefaultProjectionMatrixIndex);
    glUniform1i(Shader->GlyphOrthoMatrixIndexLoc, GlyphBuffer->GlyphOrthoMatrixIndex);
    glUniform1i(Shader->GlyphPerspMatrixIndexLoc, GlyphBuffer->GlyphPerspMatrixIndex);
    
    b32 IsImage = RenderParams->FontAtlas != 0;
    if(IsImage)
    {
        OpenGLInitImage(RenderParams->FontAtlas);
        
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(Shader->ImageLoc, 0);
        glBindTexture(GL_TEXTURE_2D, RenderParams->FontAtlas->ApiHandle);
    }
    glUniform1i(Shader->IsImageLoc, IsImage);
    
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(glyph_vertex) * GlyphBuffer->GlyphCount * 4,
                 &GlyphBuffer->GlyphVertices[0], 
                 GL_STREAM_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 sizeof(u32) * GlyphBuffer->GlyphCount * 6, 
                 &GlyphBuffer->GlyphIndices[0], 
                 GL_STREAM_DRAW);
    
    
    // NOTE(Dima): Creating and binding colors buffer
    GLuint ColorsBuffer, ColorsTBO;
    OpenGLCreateAndBindTextureBuffer(&ColorsBuffer, &ColorsTBO,
                                     sizeof(u32) * GlyphBuffer->GlyphCount,
                                     &GlyphBuffer->GlyphColors[0],
                                     GL_R32UI,
                                     1, Shader->GlyphsColorsLoc);
    
    // NOTE(Dima): Creating and binding geometry types buffer
    GLuint TypesBuffer, TypesTBO;
    OpenGLCreateAndBindTextureBuffer(&TypesBuffer, &TypesTBO,
                                     sizeof(u8) * GlyphBuffer->GlyphCount,
                                     &GlyphBuffer->GlyphTypes[0],
                                     GL_R8UI,
                                     2, Shader->GlyphsTypesLoc);
    
    // NOTE(Dima): Creating and binding Glyphs transforms buffer
    GLuint TransBuffer, TransTBO;
    OpenGLCreateAndBindTextureBuffer(&TransBuffer, &TransTBO,
                                     sizeof(m44) * GlyphBuffer->GlyphTransformsCount,
                                     &GlyphBuffer->GlyphTransforms[0],
                                     GL_RGBA32F,
                                     3, Shader->GlyphsTransformsLoc);
    
    // NOTE(Dima): Creating and binding indices to transforms buffer
    GLuint IndexToTranBuffer, IndexToTranTBO;
    OpenGLCreateAndBindTextureBuffer(&IndexToTranBuffer, &IndexToTranTBO,
                                     sizeof(u16) * GlyphBuffer->GlyphCount,
                                     &GlyphBuffer->GlyphIndicesToTransforms[0],
                                     GL_R16UI,
                                     4, Shader->GlyphsIndicesToTransformsLoc);
    
    // NOTE(Dima): Position
    if(OpenGLAttribIsValid(OpenGL.UIRectShader.PositionAttr))
    {
        glEnableVertexAttribArray(OpenGL.UIRectShader.PositionAttr);
        glVertexAttribPointer(OpenGL.UIRectShader.PositionAttr,
                              2, 
                              GL_FLOAT, 
                              GL_FALSE,
                              4* sizeof(float), 
                              0);
    }
    
    // NOTE(Dima): TexCoords
    if(OpenGLAttribIsValid(OpenGL.UIRectShader.TexCoordsAttr))
    {
        glEnableVertexAttribArray(OpenGL.UIRectShader.TexCoordsAttr);
        glVertexAttribPointer(OpenGL.UIRectShader.TexCoordsAttr,
                              2, 
                              GL_FLOAT, 
                              GL_FALSE,
                              4* sizeof(float), 
                              (void*)(2 * sizeof(float)));
    }
    
    glDrawElements(GL_TRIANGLES, GlyphBuffer->GlyphCount * 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    
    glDeleteBuffers(1, &ColorsBuffer);
    glDeleteTextures(1, &ColorsTBO);
    
    glDeleteBuffers(1, &TypesBuffer);
    glDeleteTextures(1, &TypesTBO);
    
    glDeleteBuffers(1, &TransBuffer);
    glDeleteTextures(1, &TransTBO);
    
    glDeleteBuffers(1, &IndexToTranBuffer);
    glDeleteTextures(1, &IndexToTranTBO);
    
    glUseProgram(0);
    
    glEnable(GL_DEPTH_TEST);
}

INTERNAL_FUNCTION void OpenGLRender(render_params* RenderParams)
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glViewport(0, 0, 
               RenderParams->WndDims.Width,
               RenderParams->WndDims.Height);
    
    render_commands* Commands = RenderParams->Commands;
    
    OpenGLRenderCommands(RenderParams, Commands);
    
    OpenGLRenderGlyphBuffer(RenderParams, &Commands->Glyphs3D, true, true);
    OpenGLRenderGlyphBuffer(RenderParams, &Commands->Glyphs2D, false, false);
}