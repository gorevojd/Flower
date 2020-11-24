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
    
    if(OpenGLShouldInitImage(Image)){
        
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

INTERNAL_FUNCTION void OpenGLInit()
{
    glewInit();
    
    SDL_GL_SetSwapInterval(1);
    
    OpenGL.StdShader = OpenGLLoadShader("Standard",
                                        "../Data/Shaders/std.vs",
                                        "../Data/Shaders/std.fs");
    
    glEnable(GL_DEPTH_TEST);
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

INTERNAL_FUNCTION void OpenGLRenderMesh(v3 P, image* Image, mesh* Mesh, const m44& View, const m44& Projection, v3 Color)
{
    if(!MeshInitialized(Mesh))
    {
        Mesh->ApiHandles = OpenGLAllocateMesh(Mesh, &OpenGL.StdShader);
    }
    
    // NOTE(Dima): Render
    glBindVertexArray(Mesh->ApiHandles.ArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->ApiHandles.BufferObject);
    
    glUseProgram(OpenGL.StdShader.ID);
    
    GLint DiffuseLoc = glGetUniformLocation(OpenGL.StdShader.ID, "TexDiffuse");
    GLint MultColorLoc = glGetUniformLocation(OpenGL.StdShader.ID, "MultColor");
    
    m44 Model = TranslationMatrix(P);
    
    GLint ProjectionLoc = glGetUniformLocation(OpenGL.StdShader.ID, "Projection");
    GLint ViewLoc = glGetUniformLocation(OpenGL.StdShader.ID, "View");
    GLint ModelLoc = glGetUniformLocation(OpenGL.StdShader.ID, "Model");
    
    glUniform1i(DiffuseLoc, 0);
    glUniform3f(MultColorLoc, Color.r, Color.g, Color.b);
    glUniformMatrix4fv(ProjectionLoc, 1, GL_TRUE, Projection.e);
    glUniformMatrix4fv(ViewLoc, 1, GL_TRUE, View.e);
    glUniformMatrix4fv(ModelLoc, 1, GL_TRUE, Model.e);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Image->ApiHandle);
    
    glDrawElements(GL_TRIANGLES, Mesh->IndexCount, GL_UNSIGNED_INT, 0);
    
    glUseProgram(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

INTERNAL_FUNCTION void OpenGLRender(render_params* RenderParams)
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    OpenGLInitImage(RenderParams->CubeTexture);
    
    render_commands* Commands = RenderParams->Commands;
    
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
            } break;
            
            case RenderCommand_Mesh:
            {
                render_command_mesh* MeshCommand = GetRenderCommand(Commands, CommandIndex, render_command_mesh);
                
                OpenGLRenderMesh(MeshCommand->P, 
                                 RenderParams->CubeTexture, 
                                 RenderParams->Cube, 
                                 *RenderParams->View, 
                                 *RenderParams->Projection, 
                                 MeshCommand->C);
            }break;
        }
    }
}