#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define INIT_ARR 1024

// celulas
typedef struct
{
    char *celula;
    size_t size;
    size_t count;
} flow;

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
        Token *tmp = realloc(tokenizer->tokens, new_cap * 2);
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
    for (size_t i = 0; i < t.size; i++)
    {
        printf("Token symbol: %c\nToken kind: %d\n", t.tokens[i].symbol, t.tokens[i].kind);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "uso: %s arquivo\n", argv[0]);
        return 1;
    }

    Tokenizer t;
    t.tokens = NULL;
    t.size = 0;
    t.cap = 0;

    read_tokens(&t, argv[1]);

    print_tokenizer(t);

    free_tokens(&t);
    return 0;
}