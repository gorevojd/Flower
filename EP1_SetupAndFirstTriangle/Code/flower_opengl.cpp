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

INTERNAL_FUNCTION GLuint OpenGLLoadProgram(char* ShaderName, 
                                           char* VertexFilePath, 
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
	if (!Success) {
		glGetProgramInfoLog(Program, sizeof(InfoLog), 0, InfoLog);
		//TODO(dima): Logging
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Error while linking shader program\n%s\n", InfoLog);
	}
    else{
        SDL_Log("Shader %s loaded successfully!\n", ShaderName);
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

inline b32 ArrayIsValid(GLint Arr){
    b32 Result = 1;
    
    if(Arr == -1){
        Result = 0;
    }
    
    return(Result);
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

void OpenGLInit()
{
    glewInit();
    
    SDL_GL_SetSwapInterval(1);
    
    OpenGL.StdShader = OpenGLLoadProgram("Standard",
                                         "../Data/Shaders/std.vs",
                                         "../Data/Shaders/std.fs");
}

void OpenGLFree()
{
    glDeleteProgram(OpenGL.StdShader);
}

void OpenGLRender(int WindowWidth, int WindowHeight, f64 Time)
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    float LerpFactor1 = Sin(Time) * 0.5f + 0.5f;
    float LerpFactor2 = Cos(Time) * 0.5f + 0.5f;
    float LerpFactor3 = Sin(Time + 1.57f) * 0.5f + 0.5f;
    
    v3 Color1 = Lerp(V3(1.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 1.0f), LerpFactor1);
    v3 Color2 = Lerp(V3(0.0f, 1.0f, 0.0f), V3(1.0f, 0.0f, 1.0f), LerpFactor2);
    v3 Color3 = Lerp(V3(0.0f, 0.0f, 1.0f), V3(1.0f, 1.0f, 0.0f), LerpFactor3);
    
    float VertexData[] = {
        -0.5f, -0.5f, 0.0f, Color1.r, Color1.g, Color1.b,
        0.0f, 0.5f, 0.0f, Color2.r, Color2.g, Color2.b,
        0.5f, -0.5f, 0.0f, Color3.r, Color3.g, Color3.b,
    };
    
    GLuint TriVAO;
    GLuint TriVBO;
    
    // NOTE(Dima): Init VAO and VBO
    glGenVertexArrays(1, &TriVAO);
    glGenBuffers(1, &TriVBO);
    
    glBindVertexArray(TriVAO);
    glBindBuffer(GL_ARRAY_BUFFER, TriVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STREAM_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3, 
                          GL_FLOAT, 
                          GL_FALSE,
                          6 * sizeof(float), 
                          0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3, 
                          GL_FLOAT, 
                          GL_FALSE,
                          6 * sizeof(float), 
                          (void*)(3 * sizeof(float)));
    
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    //m44 ProjectionMatrix = OrthographicProjection(WindowWidth, WindowHeight);
    m44 ProjectionMatrix = Identity();
    
    // NOTE(Dima): Render
    glBindVertexArray(TriVAO);
    glBindBuffer(GL_ARRAY_BUFFER, TriVBO);
    
    glUseProgram(OpenGL.StdShader);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUseProgram(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glDeleteBuffers(1, &TriVBO);
    glDeleteVertexArrays(1, &TriVAO);
}
