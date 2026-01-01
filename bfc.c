// gcc -Wall -Wextra bfc.c -o bfc
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

typedef enum
{
    GO,     // >
    BACK,   // <
    INC,    // +
    DEC,    // -
    PRINT,  // .
    GETC,   // ,
    LOOP_B, // [
    LOOP_E, // ]
    INVALID
} token_kind;

typedef struct
{
    int data;
    bool visited;
} Cell;

// celulas
typedef struct
{
    Cell *cells;
    size_t size;
    size_t cap;
} Flow;

typedef struct
{
    char symbol;
    token_kind kind;
} Token;

typedef struct Tokenizer Tokenizer;
struct Tokenizer
{
    Token *tokens;
    size_t size;
    size_t cap;
};

typedef struct
{
    size_t body_start;
    size_t counter_index;
} Loop;

typedef struct
{
    Loop *loops;
    size_t size;
    size_t cap;
    int index; // which loop we are in
    bool running;
} LoopStack;

void free_tokens(Tokenizer *t)
{
    free(t->tokens);
    t->tokens = NULL;
    t->size = 0;
    t->cap = 0;
}

void free_flow(Flow *f)
{
    free(f->cells);
    f->cells = NULL;
    f->size = 0;
    f->cap = 0;
}

void free_loops(LoopStack *ls)
{
    free(ls->loops);
    ls->loops = NULL;
    ls->size = 0;
    ls->cap = 0;
    ls->index = 0;
    ls->running = false;
}

void loop_append(LoopStack *l_stack, Loop loop)
{
    if (l_stack->cap == l_stack->size)
    {
        size_t new_cap = l_stack->cap ? l_stack->cap * 2 : 1024;
        Loop *tmp = realloc(l_stack->loops, new_cap * sizeof(Loop));
        if (!tmp)
        {
            free_loops(l_stack);
            assert("NOT POSSIBLE TO REALLOC LOOPS");
        }

        l_stack->loops = tmp;
        l_stack->cap = new_cap;
    }

    l_stack->loops[l_stack->size++] = loop;
    l_stack->index++; // overflow?
}

// token should be *token?
void token_append(Tokenizer *tokenizer, Token token)
{
    if (tokenizer->cap == tokenizer->size)
    {
        size_t new_cap = tokenizer->cap ? tokenizer->cap * 2 : 1024;
        Token *tmp = realloc(tokenizer->tokens, new_cap * sizeof(Token));
        if (!tmp)
        {
            free_tokens(tokenizer);
            assert("NOT POSSIBLE TO REALLOC TOKENS");
        }

        tokenizer->tokens = tmp;
        tokenizer->cap = new_cap;
    }

    tokenizer->tokens[tokenizer->size++] = token;
}

void read_tokens(Tokenizer *t, const char *file_path)
{
    FILE *f;
    int c;

    f = fopen(file_path, "r");
    if (!f)
    {
        perror("fopen");
        free_tokens(t);
    }

    while ((c = fgetc(f)) != EOF)
    {
        Token token = {0};
        token.symbol = c;

        switch (c)
        {
        case '>':
            token.kind = GO;
            break;
        case '<':
            token.kind = BACK;
            break;
        case '+':
            token.kind = INC;
            break;
        case '-':
            token.kind = DEC;
            break;
        case '.':
            token.kind = PRINT;
            break;
        case ',':
            token.kind = GETC;
            break;
        case '[':
            token.kind = LOOP_B;
            break;
        case ']':
            token.kind = LOOP_E;
            break;
        default:
            continue;
        }

        token_append(t, token);
    }

    if (fclose(f) != 0)
    {
        perror("fclose");
        free_tokens(t);
        assert("NOT POSSIBLE TO PROPERLY CLOSE FILE");
    }
}

void print_tokenizer(Tokenizer t, bool all)
{
    printf("Tokenizer size: %zu, cap: %zu\n", t.size, t.cap);

    if (!all)
        return;
    for (size_t i = 0; i < t.size; i++)
    {
        printf("Token symbol: %c\nToken kind: %d\n\n", t.tokens[i].symbol, t.tokens[i].kind);
    }
}

void cells_inc(Flow *f, size_t flow_pos)
{
    if (f->cap == f->size - 1)
    {
        size_t new_cap = f->cap ? f->cap * 2 : 1024;
        Cell *tmp = realloc(f->cells, new_cap * sizeof(Cell));
        if (!tmp)
        {
            free_flow(f);
            assert("NOT POSSIBLE TO REALLOC CELLS");
        }

        f->cells = tmp;
        for (size_t i = f->cap; i < new_cap; i++)
        {
            f->cells[i].data = 0;
            f->cells[i].visited = false;
        }
        f->cap = new_cap;
    }

    if (flow_pos > f->cap)
    {
        assert("FLOW POSITION NOT VALID");
    }

    f->cells[flow_pos].data += 1;
}

void parse(Flow *f, const Tokenizer t)
{
    f->size = 1; // account for cell #0
    size_t flow_pos = 0;

    size_t token_pos = 0;

    int counter;

    // Set up loop stack
    LoopStack ls;
    ls.loops = NULL;
    ls.size = 0;
    ls.cap = 0;
    ls.index = -1;
    ls.running = false;

    while (token_pos < t.size)
    {
        char s = t.tokens[token_pos].symbol;
        // printf("token  %c at  %zu\n", s, token_pos);
        // printf("Current symbol: %c\n", s);
        if (ls.running && ls.index != -1 && f->cells != NULL)
        {
            // should end loop
            if (f->cells[flow_pos].visited && f->cells[flow_pos].data == 0)
            {
                ls.running = false;
                if (ls.index >= 0)
                    ls.index--;
            }

            // printf("Counter: %d\n", counter);
            // printf("loop index: %d, body start: %zu, counter index: %zu\n", ls.index, ls.loops[ls.index].body_start, ls.loops[ls.index].counter_index);
        }

        switch (s)
        {
        case '>':
            if (flow_pos != t.size - 1)
            {
                if (!f->cells[flow_pos].visited)
                {
                    f->size++;
                    f->cells[flow_pos].visited = true;
                }
                flow_pos++;
            }
            break;
        case '<':
            if (flow_pos != 0)
                flow_pos--;
            break;
        case '+':
            cells_inc(f, flow_pos);
            break;
        case '-':
            f->cells[flow_pos].data--;
            break;
        case '.':
            printf("%c", f->cells[flow_pos].data);
            break;
        case ',':
            // TODO
            break;
        case '[':
        {
            Loop new_loop;
            new_loop.body_start = token_pos + 1;
            // printf("flow_pos %zu\n", flow_pos);
            new_loop.counter_index = flow_pos;
            ls.running = true;
            loop_append(&ls, new_loop);
            // printf("\n");
            break;
        }
        break;
        case ']':
            if (ls.running)
            {
                token_pos = ls.loops[ls.index].body_start;
                // printf("\n");

                continue;
            } 
            break;
        default:
            continue;
        }

        // if (ls.index != -1)
        // {
        //     printf("loop index: %d, body start: %zu, counter index: %zu\n", ls.index, ls.loops[ls.index].body_start, ls.loops[ls.index].counter_index);
        // }

        s = '\0';
        token_pos++;
    }

    free_loops(&ls);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Use: %s [file]\n", argv[0]);
        return 1;
    }

    Tokenizer t;
    t.tokens = NULL;
    t.size = 0;
    t.cap = 0;
    read_tokens(&t, argv[1]);

    Flow f;
    f.cells = NULL;
    f.size = 0;
    f.cap = 0;

    parse(&f, t);

    printf("Flow size: %zu\n", f.size);
    for (size_t i = 0; i < f.size; i++)
    {
        printf("Cell #%zu = %d\n", i, f.cells[i].data);
    }

    print_tokenizer(t, false);

    free_tokens(&t);
    free_flow(&f);
    return 0;
}