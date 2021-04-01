INTERNAL_FUNCTION inline b32 HasComponent(game_object* Obj, 
                                          u32 ComponentType, 
                                          u32* OutComponentMask = 0)
{
    u32 ComponentMask = 1 << ComponentType;
    
    b32 Result = BoolFlag(Obj->ComponentsMask, ComponentMask);
    
    if(OutComponentMask)
    {
        *OutComponentMask = ComponentMask;
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline component* GetComponent(game_object* Obj, u32 ComponentType)
{
    component* Result = 0;
    
    if(HasComponent(Obj, ComponentType))
    {
        Result = &Obj->Components[ComponentType];
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline component* AddComponent(game_object* Obj, u32 ComponentType)
{
    component* Result = 0;
    
    u32 ComponentMask;
    
    if(HasComponent(Obj, ComponentType, &ComponentMask))
    {
        // NOTE(Dima): Component has already been added
        Result = GetComponent(Obj, ComponentType);
    }
    else
    {
        Obj->ComponentsMask |= ComponentMask;
        
        Result = GetComponent(Obj, ComponentType);
        Result->FreeSize = 0;
        Result->Free = 0;
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline void FreeComponentsData(component* Component)
{
    if(Component->FreeSize)
    {
        free(Component->Free);
    }
    
    Component->FreeSize = 0;
    Component->Free = 0;
}

INTERNAL_FUNCTION inline void RemoveComponent(game_object* Obj, u32 ComponentType)
{
    u32 ComponentMask;
    
    if(HasComponent(Obj, ComponentType, & ComponentMask))
    {
        Obj->ComponentsMask &= ~ComponentMask;
        
        // NOTE(Dima): Freing components data
        component* Component = GetComponent(Obj, ComponentType);
        FreeComponentsData(Component);
    }
}

INTERNAL_FUNCTION inline void RemoveAllComponents(game_object* Obj)
{
    for(int ComponentIndex = 0;
        ComponentIndex < Component_Count;
        ComponentIndex++)
    {
        RemoveComponent(Obj, ComponentIndex);
    }
}

#define GetComp(obj, type) ((type*)(&(GetComponent(obj, Component_##type))->Component))
