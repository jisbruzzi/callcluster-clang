#include "complexity_analysis.h"
#include <stdio.h>
#include <stdlib.h>

struct Node {
    int holis;
};
typedef struct Node Node;

struct Operation {
    enum CXCursorKind Kind;
    struct Operation* Tail;
    //properties for CompoundStatement
    Node* CompoundLast;
};
typedef struct Operation Operation;

typedef struct {
    Operation* OpStack;
    Node* Entry;
    Node* Exit;
} Visit;

Node* Node_create()
{
    Node* ret = malloc(sizeof(Node));
    return ret;
}
void Node_addEdge(Node* from, Node* to)
{
    printf("from %p to %p\n",from,to);
}

void Visit_push(Visit* v){
    Operation* new = malloc(sizeof(Operation));
    new->Kind=0;
    new->CompoundLast=NULL;
    new->Tail = v->OpStack;
    v->OpStack = new;
}

Node* Visit_getLast(Visit* v)
{
    if(v->OpStack==NULL){
        return v->Entry;
    }else{
        switch (v->OpStack->Kind)
        {
        case CXCursor_CompoundStmt:
            return v->OpStack->CompoundLast;
        default:
            return v->Entry;
        }
    }
}
        
void Visit_enter(Visit* v, enum CXCursorKind k)
{
    // print operation type
    CXString s = clang_getCursorKindSpelling(k);
    printf("enter: %s\n",clang_getCString(s));
    clang_disposeString(s);

    if(k == CXCursor_CompoundStmt){
        Node* last = Visit_getLast(v);
        Visit_push(v);
        v->OpStack->Kind = CXCursor_CompoundStmt;
        v->OpStack->CompoundLast = last;
    }
}

void Visit_pop(Visit* v){
    Operation* old = v->OpStack;
    v->OpStack = v->OpStack->Tail;
    free(old);
}

void Visit_exit(Visit* v)
{
    if(v->OpStack != NULL){

        CXString s = clang_getCursorKindSpelling(v->OpStack->Kind);
        printf("exit: %s\n",clang_getCString(s));
        clang_disposeString(s);

        if(v->OpStack->Kind==CXCursor_CompoundStmt){
            Node* last = v->OpStack->CompoundLast;
            Visit_pop(v);
            if(v->OpStack!=NULL){
                v->OpStack->CompoundLast = last;
            }
        }else{
            Visit_pop(v);
        }
    }
}

void Visit_expression(Visit* v)
{
    printf("e\n");
    if(v->OpStack->Kind==CXCursor_CompoundStmt){
        Node* new = Node_create();
        Node_addEdge(v->OpStack->CompoundLast,new);
        v->OpStack->CompoundLast = new;
    }
}


enum CXChildVisitResult general_visitor (CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    Visit* visit = (Visit*) client_data;
    enum CXCursorKind kind = clang_getCursorKind(cursor);
    if(clang_isStatement(kind) && kind == CXCursor_CompoundStmt){

        Visit_enter(visit,kind);
        clang_visitChildren(cursor, general_visitor, (CXClientData) visit);
        Visit_exit(visit);


        return CXChildVisit_Continue;
    }else{
        if(clang_isStatement(clang_getCursorKind(parent))){
            Visit_expression(visit);
        }

        return CXChildVisit_Recurse;
    }
}

Visit* Visit_create()
{
    Visit* ret = malloc(sizeof(Visit));
    ret->OpStack = NULL;
    ret->Entry = malloc(sizeof(Node));
    ret->Exit = malloc(sizeof(Node));
    return ret;
}

unsigned int Visit_get_complexity(Visit* v)
{
    return 2;
}

void Visit_dispose(Visit* v)
{
    free(v);
}

unsigned int get_complexity(CXCursor c)
{
    printf("-----\n");
    Visit* v = Visit_create();
    clang_visitChildren(c, general_visitor, (CXClientData) v);
    unsigned int complexity = Visit_get_complexity(v);
    Visit_dispose(v);
    return complexity;
}