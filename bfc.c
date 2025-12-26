#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define INIT_ARR 1024

// celulas
typedef struct
{
    int *cells;
    size_t size;
    size_t cap;
} Flow;

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

void free_tokens(Tokenizer *t)
{
    free(t->tokens);
    t->tokens = NULL;
    t->size = 0;
    t->cap = 0;
}

void token_append(Tokenizer *tokenizer, Token token)
{
    if (tokenizer->cap == tokenizer->size)
    {
        size_t new_cap = tokenizer->cap ? tokenizer->cap * 2 : 1024;
        Token *tmp = realloc(tokenizer->tokens, new_cap * sizeof(token));
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

void print_tokenizer(Tokenizer t)
{
    printf("Tokenizer size: %zu, cap: %zu\n", t.size, t.cap);
    // for (size_t i = 0; i < t.size; i++)
    // {
    //     printf("Token symbol: %c\nToken kind: %d\n\n", t.tokens[i].symbol, t.tokens[i].kind);
    // }
}

void free_flow(Flow *f)
{
    free(f->cells);
    f->cells = NULL;
    f->size = 0;
    f->cap = 0;
}

void cells_inc(Flow *f, size_t flow_pos)
{
    if (f->cap == f->size - 1)
    {
        size_t new_cap = f->cap ? f->cap * 2 : 1024;
        int *tmp = realloc(f->cells, new_cap * sizeof(int));
        if (!tmp)
        {
            free_flow(f);
            assert("NOT POSSIBLE TO REALLOC CELLS");
        }

        f->cells = tmp;
        for (size_t i = f->cap; i < new_cap; i++)
        {
            f->cells[i] = 0;
        }
        f->cap = new_cap;
    }

    if (flow_pos > f->cap)
    {
        assert("FLOW POSITION NOT VALID");
    }

    f->cells[flow_pos] += 1;
}

void parse(Flow *f, const Tokenizer t)
{
    f->size = 1; // account for cell #0
    size_t flow_pos = 0;
    size_t loop_b = 0;
    // TODO : for -> while
    for (size_t i = 0; i < t.size; i++)
    {
        char s = t.tokens[i].symbol;
        switch (s)
        {
        case '>':
            if (flow_pos != t.size - 1)
            {
                flow_pos++;
                f->size++;
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
            f->cells[flow_pos]--;
            break;
        case '.':
            printf("%c", f->cells[flow_pos]);
            break;
        case ',':
            // TODO
            break;
        case '[':
            loop_b = flow_pos + 1;
            break;
        case ']':
            flow_pos = loop_b;
            break;
        default:
            continue;
        }

        s = '\0';
    }
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
        printf("Cell #%zu = %zu\n", i, f.cells[i]);
    }

    print_tokenizer(t);

    free_tokens(&t);
    free_flow(&f);
    return 0;
}