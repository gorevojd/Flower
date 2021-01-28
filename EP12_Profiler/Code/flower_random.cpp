#include "flower_random.h"

struct sample_point_coord
{
    int x;
    int y;
};

INTERNAL_FUNCTION inline sample_point_coord GetSampleCoord(v2 Sample, 
                                                           f32 CellDim)
{
    sample_point_coord Result = {};
    
    Result.x = std::floor(Sample.x / CellDim);
    Result.y = std::floor(Sample.y / CellDim);
    
    return(Result);
}

INTERNAL_FUNCTION void AddSampleToGrid(std::vector<v2>& Samples,
                                       std::vector<int>& Grid,
                                       std::vector<v2>& Active,
                                       int GridSizeX,
                                       f32 CellDim,
                                       v2 Sample)
{
    int SampleIndex = Samples.size();
    
    sample_point_coord Coord = GetSampleCoord(Sample,
                                              CellDim);
    
    Grid[Coord.y * GridSizeX + Coord.x] = SampleIndex;
    Samples.push_back(Sample);
    Active.push_back(Sample);
}

INTERNAL_FUNCTION b32 SamplePointIsValid(v2 Point, 
                                         f32 CellSize,
                                         int GridSizeX,
                                         int GridSizeY,
                                         std::vector<int>& Grid,
                                         std::vector<v2>& Samples)
{
    b32 Result = false;
    
    f32 r = CellSize * F_SQRT_TWO;
    f32 SqrR = r * r;
    
    int CoordX;
    int CoordY;
    sample_point_coord Coord = GetSampleCoord(Point,
                                              CellSize);
    
    if(Coord.x >= 0 && Coord.x < GridSizeX &&
       Coord.y >= 0 && Coord.y < GridSizeY)
    {
        b32 IsFarEnough = true;
        
        // NOTE(Dima): Iterating through all valid nearby samples
        for(int y = std::max(Coord.y - 2, 0);
            y <= std::min(Coord.y + 2, GridSizeY - 1);
            y++)
        {
            if(IsFarEnough)
            {
                for(int x = std::max(Coord.x - 2, 0);
                    x <= std::min(Coord.x + 2, GridSizeX - 1);
                    x++)
                {
                    int IndexInGrid = y * GridSizeX + x;
                    
                    int SampleIndex = Grid[IndexInGrid];
                    
                    // NOTE(Dima): If sample exist here
                    if(SampleIndex != -1)
                    {
                        // NOTE(Dima): Getting sample
                        v2 GridSample = Samples[SampleIndex];
                        
                        // NOTE(Dima): Check distance to sample
                        v2 Offset = GridSample - Point;
                        f32 SqDistanceToSample = Offset.x * Offset.x + Offset.y * Offset.y;
                        
                        if(SqDistanceToSample < SqrR)
                        {
                            IsFarEnough = false;
                            break;
                        }
                    }
                }
            }
            else
            {
                break;
            }
        }
        
        // NOTE(Dima): If point is adequately far enough from all current samples - insert it
        if(IsFarEnough)
        {
            Result = true;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION std::vector<v2> GenerateBlueNoise(v2 GridSize, f32 CellSize, 
                                                    int Seed = 123,
                                                    int CountBeforeRejection = 30, 
                                                    b32 BoundToCeil = false)
{
    f32 r = CellSize * F_SQRT_TWO;
    f32 SqrR = r * r;
    
    random_generation Random = SeedRandom(Seed);
    
    f32 GridSizeX_ = GridSize.x / CellSize;
    f32 GridSizeY_ = GridSize.y / CellSize;
    
    int GridSizeX = std::floor(GridSizeX_);
    int GridSizeY = std::floor(GridSizeY_);
    
    if(BoundToCeil)
    {
        GridSizeX = std::ceil(GridSizeX_);
        GridSizeY = std::ceil(GridSizeY_);
    }
    
    // NOTE(Dima): Initializing background grid
    std::vector<int> Grid = std::vector<int>(GridSizeX * GridSizeY, -1);
    
    v2 RandomSample = GridSize * 0.5f;
    
    std::vector<v2> Result;
    Result.reserve(GridSizeX * GridSizeY);
    
    std::vector<v2> Active;
    Active.reserve(GridSizeX * GridSizeY / 4);
    
    // NOTE(Dima): Inserting initial sample
    AddSampleToGrid(Result, Grid, Active,
                    GridSizeX,
                    CellSize,
                    RandomSample);
    
    while(Active.size() > 0)
    {
        b32 AtLeastOneFound = false;
        
        int CheckIndex = RandomBetweenU32(&Random, 0, Active.size());
        v2 CheckPoint = Active[CheckIndex];
        
        // NOTE(Dima): Generating CountBeforeRejection nearby points
        for(int k = 0; k < CountBeforeRejection; k++)
        {
            // NOTE(Dima): Generating point
            f32 Rad = RandomBetweenFloats(&Random, r, 2.0f * r);
            f32 Angle = RandomUnilateral(&Random) * 2.0f * F_PI;
            
            v2 NearbyPoint = CheckPoint + V2(Cos(Angle), Sin(Angle)) * Rad;
            
            if(SamplePointIsValid(NearbyPoint,
                                  CellSize,
                                  GridSizeX,
                                  GridSizeY,
                                  Grid,
                                  Result))
            {
                // NOTE(Dima): Inserting initial sample
                AddSampleToGrid(Result, Grid, Active,
                                GridSizeX, 
                                CellSize,
                                NearbyPoint);
                
                AtLeastOneFound = true;
            }
        }
        
        // NOTE(Dima): If after CountBeforeRejection attempts we did not found the point - remove it
        if(!AtLeastOneFound)
        {
            std::swap(Active[CheckIndex], Active[Active.size() - 1]);
            Active.pop_back();
        }
    } // NOTE(Dima): While loop
    
    return(Result);
}