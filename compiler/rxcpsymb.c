/*
 * Symbol and Scope Infrastructure
 */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "platform.h"
#include "rxcpmain.h"
#include "../avl_tree/avl_tree.h"
#include "rxcpdary.h"

#ifndef NUTF8
#include "utf.h"
#endif

/* Frees a symbol */
static void symbol_free(Symbol *symbol);

/* Internal Tree node structure */
struct symbol_wrapper {
    char *index;
    Symbol *value;
    struct avl_tree_node index_node;
};

#define GET_INDEX(i) avl_tree_entry((i), struct symbol_wrapper, index_node)->index

#define GET_VALUE(i) avl_tree_entry((i), struct symbol_wrapper, index_node)->value

static int compare_node_node(const struct avl_tree_node *node1,
                             const struct avl_tree_node *node2)
{
    char* n1 = GET_INDEX(node1);
    char* n2 = GET_INDEX(node2);
    return strcmp(n1,n2);
}

static int compare_node_value(const void *value,
                              const struct avl_tree_node *nodeptr) {
    char* node = GET_INDEX(nodeptr);
    return strcmp((char*)value,node);
}

/* Adds a symbol to tree */
/* Returns 0 on success, 1 on duplicate */
static int add_symbol_to_tree(struct avl_tree_node **root, Symbol *value) {

    struct symbol_wrapper *i = malloc(sizeof(struct symbol_wrapper));
    i->index = value->name;
    i->value = value;
    if (avl_tree_insert(root, &i->index_node, compare_node_node)) {
        /* Duplicate! */
        free(i);
        return 1;
    }
    return 0;
}

// Search for a symbol
// Returns Symbol if found or null
static Symbol* src_symbol(struct avl_tree_node *root, char* index) {
    struct avl_tree_node *result;

    result = avl_tree_lookup(root, index, compare_node_value);

    if (result) return GET_VALUE(result);
    else return NULL;
}

/* Scope Factory */
Scope *scp_f(Scope *parent, ASTNode *node) {
    Scope *scope = (Scope*) malloc(sizeof(Scope));
    scope->defining_node = node;
    node->scope = scope;
    scope->parent = parent;
    scope->symbols_tree = 0;
    scope->num_registers = 1; /* r0 is always available as a temp register */
    scope->free_registers_array = dpa_f();
    scope->child_array  = dpa_f();
    if (scope->parent) dpa_add((dpa*)(parent->child_array), scope);

    return scope;
}

/* Calls the handler for each symbol in scope */
void scp_4all(Scope *scope, symbol_worker worker, void *payload) {
    struct symbol_wrapper *i;

    if (scope->symbols_tree) {
        /* This walks the tree in sort order - do not alter list! */
        avl_tree_for_each_in_order(i, scope->symbols_tree, struct symbol_wrapper,
                                       index_node) {
            worker(i->value, payload);
        }
    }
}

/* Frees scope and all its symbols */
void scp_free(Scope *scope) {
    struct symbol_wrapper *i;

    if (scope->symbols_tree) {
        /* This walks the tree in post order which allows each node be freed */
        avl_tree_for_each_in_postorder(i, scope->symbols_tree,
                                       struct symbol_wrapper,
                                       index_node) {
            symbol_free(i->value);
            free(i);
        }
        scope->symbols_tree = 0; /* Pedantic ... */
    }
    free_dpa(scope->child_array);
    free_dpa(scope->free_registers_array);
    free(scope);
}

/* Get a free register from scope */
int get_reg(Scope *scope) {
    dpa *free_array;
    int reg;

    free_array = (dpa*)(scope->free_registers_array);
    /* Check the free list */
    if (free_array->size) {
        free_array->size--;
        reg = (int)(size_t)(free_array->pointers[free_array->size]);
    }
    else {
        reg = (int)((scope->num_registers)++);
    }

//    printf("get %d - ", reg);
//    {int i; for (i=0; i<free_array->size; i++) printf("%d ",(int)(size_t)free_array->pointers[i]);printf("\n");}

    return reg;
}

/* Return a no longer used register to the scope */
void ret_reg(Scope *scope, int reg) {
    size_t i;
    dpa *free_array;
    free_array = (dpa*)(scope->free_registers_array);

//    printf("free %d - ", reg);
//    {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}

    for (i=0; i<free_array->size; i++) {
        if (reg == (size_t)free_array->pointers[i]) {
            printf(" ... already freed\n");
            return;
        }
    }
    dpa_ado(free_array, (void*)(size_t)reg);
}

/* Get number of free register from scope - returns the start of a sequence
 * n, n+1, n+2, ... n+number */
int get_regs(Scope *scope, size_t number) {
    dpa *free_array;
    int reg, r, top, i;
    size_t seq;

    free_array = (dpa*)(scope->free_registers_array);

//    printf("get %d regs - ", (int)number);
//    {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}

    /* Check the free list - how many could be used */
    if (free_array->size) {
        i = (int)free_array->size - 1;
        top = (int)(size_t)(free_array->pointers[i]);
        for (seq=1, i--; i>=0; i--) {
            r = (int)(size_t)(free_array->pointers[i]);
            if (r == top - 1) {
                /* Part of the sequence */
                top--;
                seq++;
                if (seq >= number) {
                    /* We have enough registers to reuse */
                    reg = top; /* Result is the beginning of the sequence */
                    /* Now remove them from the free list */
                    free_array->size -= number;
//                    printf("get %d-%d\n", reg, reg+(int)number - 1);
                    return reg;
                }
            }
            else break;
        }
        /* seq is now the number of registers which could be used in the free list */
        /* top is the first register which may be useful */
        /* Can we use these plus some new ones */
        r = (int)(size_t)(free_array->pointers[free_array->size - 1]) + 1;
        if (r == (int)(scope->num_registers)) {
            /* Yes we can because the next unused register adds to the sequence */
            reg = top; /* Result is the beginning of the sequence */
            /* Now remove them from the free list */
            free_array->size -= seq;
            /* Now assign some brand ne ones */
            scope->num_registers += number - seq;
//            printf("get %d-%d\n", reg, reg+(int)number - 1);
            return reg;
        }
        /* No we can't so just assign new ones */
    }

    reg = (int)(scope->num_registers); /* Assign brand-new registers */
    scope->num_registers += number;
//    printf("get %d-%d\n", reg, reg+(int)number - 1);
    return reg;
}

/* Return no longer used registers to the scope, starting from reg
 * reg, reg+1, ... reg+number */
void ret_regs(Scope *scope, int reg, size_t number) {
    dpa *free_array;
    size_t j, i;
    free_array = (dpa*)(scope->free_registers_array);

//    printf("free %d-%d - ", reg, reg + (int)number - 1);
//    {int ii; for (ii=1; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}

    for (j=0; j<number; j++) {
        for (i=0; i<free_array->size; i++)
            if (reg == (size_t)free_array->pointers[i]) {
//                printf(" ... %d already freed\n",reg);
                break;
            }
        dpa_ado(free_array, (void*)(size_t)reg);
        reg++;
    }
}

char* type_nm(ValueType type) {
    switch (type) {
        case TP_BOOLEAN: return "Boolean";
        case TP_INTEGER: return "Integer";
        case TP_FLOAT: return "Float";
        case TP_STRING: return "String";
        case TP_OBJECT: return "Object";
        default: return "Unknown";
    }
}

Scope* scp_chd(Scope *scope, size_t index) {
    return (Scope*)((dpa*)(scope->child_array))->pointers[index];
}

size_t scp_noch(Scope *scope) {
    return ((dpa*)(scope->child_array))->size;
}

/* Symbol Factory - define a symbol */
/* Returns NULL if the symbol is a duplicate */
Symbol *sym_f(Scope *scope, ASTNode *node) {
    char *c;
    Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));

    symbol->scope = scope;
    symbol->type = TP_UNKNOWN;
    symbol->register_num = -1;
    symbol->name = (char*)malloc(node->node_string_length + 1);
    memcpy(symbol->name, node->node_string, node->node_string_length);
    symbol->name[node->node_string_length] = 0;
    symbol->register_type = 'r';
    symbol->is_constant = 0;
    symbol->is_function = 0;

    /* Uppercase symbol name */
#ifdef NUTF8
    for (c = symbol->name ; *c; ++c) *c = (char)toupper(*c);
#else
    utf8upr(symbol->name);
#endif
    symbol->ast_node_array = dpa_f();

    /* Returns 1 on duplicate */
    if (add_symbol_to_tree((struct avl_tree_node **)&(scope->symbols_tree),
            symbol)) {
        symbol_free(symbol);
        return NULL;
    }

    return symbol;
}

/* Resolve a Symbol - including parent scope */
Symbol *sym_rslv(Scope *scope, ASTNode *node) {
    Symbol *result;
    char *c;
    /* Sadly we are making a null terminated string */
    char *name = (char*)malloc(node->node_string_length + 1);
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;

    /* Uppercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)toupper(*c);
#else
    utf8upr(name);
#endif

    /* Look for the symbol - looking up in each parent scope */
    do {
        result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
        if (result) {
            free(name);
            return result;
        }
        scope = scope->parent;
    } while (scope);
    free(name);
    return 0;
}

/* Local Resolve a Symbol - current scope only */
Symbol *sym_lrsv(Scope *scope, ASTNode *node) {
    Symbol *result = 0;
    char *c;
    /* Sadly we are making a null terminated string */
    char *name = (char*)malloc(node->node_string_length + 1);
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;

    /* Uppercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)toupper(*c);
#else
    utf8upr(name);
#endif

    result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
    free(name);
    return result;
}

/* Frees a symbol */
static void symbol_free(Symbol *symbol) {
    size_t i;
    free(symbol->name);

    /* Free SymbolNode Connectors */
    for (i=0; i < ((dpa*)(symbol->ast_node_array))->size; i++) {
        free(((dpa*)(symbol->ast_node_array))->pointers[i]);
    }
    free_dpa(symbol->ast_node_array);

    free(symbol);
}

/* Returns the index'th SymbolNode connector attached to a symbol */
SymbolNode* sym_trnd(Symbol *symbol, size_t index) {
    return (SymbolNode*)((dpa*)(symbol->ast_node_array))->pointers[index];
}

/* Connect a ASTNode to a Symbol */
void sym_adnd(Symbol *symbol, ASTNode* node, unsigned int readAccess,
              unsigned int writeAccess) {
    SymbolNode *connector = malloc(sizeof(SymbolNode));
    connector->symbol = symbol;
    connector->node = node;
    connector->readUsage = readAccess;
    connector->writeUsage = writeAccess;

    dpa_add((dpa*)(symbol->ast_node_array), connector);
    node->symbol = connector;
}

/* Returns the number of AST nodes connected to a symbol */
size_t sym_nond(Symbol *symbol) {
    return ((dpa*)(symbol->ast_node_array))->size;
}
