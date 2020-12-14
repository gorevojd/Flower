struct node
{
    char Name[128];
    
    v3 Offset;
    quat Rotation;
    v3 Scale;
    
    node* Parent;
    
    node* Next;
    node* Prev;
    
    node* ChildFirst;
    node* ChildEnd;
};

struct world
{
    memory_arena* Arena;
    
    node* RootNode;
    
    node ChildUse;
    node ChildFree;
};

#define DLIST_INSERT_AFTER(elem, after, next, prev) \
(elem)->##next = (after)->##next; \
(elem)->##prev = after; \
\
(elem)->##next##->##prev = elem; \
(elem)->##prev##->##next = elem;

#define DLIST_INSERT_BEFORE(elem, before, next, prev) \
(elem)->##next = after; \
(elem)->##prev = (after)->##prev; \
\
(elem)->##next##->##prev = elem; \
(elem)->##prev##->##next = elem;

#define DLIST_REMOVE(elem, next, prev) \
(elem)->##prev##->##next = (elem)->##next; \
(elem)->##next##->##prev = (elem)->##prev;

#define DLIST_REFLECT(elem, next, prev) \
(elem)->##next = elem; \
(elem)->##prev = elem;

#define DLIST_EMPTY(elem, next) ((elem)->##next == elem)

INTERNAL_FUNCTION inline node* AllocateNode(world* World)
{
    if(DLIST_EMPTY(&World->ChildFree, Next))
    {
        int NumberOfNewFreeNodes = 1024;
        node* NewFreeNodes = PushArray(World->Arena, node, NumberOfNewFreeNodes);
        
        for(int NodeIndex = 0;
            NodeIndex < NumberOfNewFreeNodes;
            NodeIndex++)
        {
            node* CurNode = &NewFreeNodes[NodeIndex];
            
            DLIST_INSERT_AFTER(CurNode, &World->ChildFree, Next, Prev);
        }
    }
    
    node* Result = World->ChildFree.Next;
    
    // NOTE(Dima): Remove from free list
    DLIST_REMOVE(Result, Next, Prev);
    
    // NOTE(Dima): Inserting to use list
    DLIST_INSERT_AFTER(Result, &World->ChildUse, Next, Prev);
    
    return(Result);
}

INTERNAL_FUNCTION world* CreateWorld(memory_arena* Arena)
{
    // NOTE(Dima): Init world structure
    world* World = PushStruct(Arena, world);
    World->Arena = Arena;
    
    // NOTE(Dima): Init nodes
    DLIST_REFLECT(&World->ChildUse, Next, Prev);
    DLIST_REFLECT(&World->ChildFree, Next, Prev);
    
    return(World);
}