#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "./polnoc_lexer.h"
#include "./polnoc_parser.h"

char *plc_read_file_into_memory(const char *plc_file_path, size_t *plc_file_size)
{
    FILE *fp = fopen(plc_file_path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "[ERROR] Could not read `%s`: `%s`\n", plc_file_path, strerror(errno));
        return NULL;
    }

    int ret = fseek(fp, 0, SEEK_END);
    if (ret < 0) {
        fprintf(stderr, "[ERROR] Could not seek to end of `%s`: `%s`\n", plc_file_path, strerror(errno));
        fclose(fp);
        return NULL;
    }

    size_t size = ftell(fp);
    if (size <= 0) {
        fprintf(stderr, "[ERROR] File `%s` Empty\n", plc_file_path);
        fclose(fp);
        return NULL;
    }

    rewind(fp); // Rewind the file pointer to the beginning

    char *buffer = (char *)malloc(size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "[ERROR] Memory Allocation Failed\n");
        fclose(fp);
        return NULL;
    }

    size_t bytes = fread(buffer, sizeof(*buffer), size, fp);
    if (bytes != (size_t)size) {
        fprintf(stderr, "fread() failed: Expected %ld bytes got %zu bytes\n", size, bytes);
        free(buffer);
        fclose(fp);
        return NULL;
    }
    buffer[bytes] = '\0'; // Null Terminate

    *plc_file_size = size;
    fclose(fp); // close file pointer
    return buffer;
}

const char *plc_shift_args(int *argc, char ***argv)
{
    const char *result = **argv;
    (*argc)--;
    (*argv)++;
    return result;
}

bool plc_dump_tokens(const Plc_Tokens *tokens)
{
    if (!tokens) return false;
    for (size_t i = 0; i < tokens->count; ++i) {
        Plc_Token token = tokens->contents[i];
        switch (token.type) {
        case PLC_TOKEN_NUMBER: {
            printf("[INFO] Type: %s, Token: %.2lf\n", plc_token_type_as_cstr(token.type), token.data.number);
        } break;

        case PLC_TOKEN_STRING:
        case PLC_TOKEN_DIV:
        case PLC_TOKEN_MULT:
        case PLC_TOKEN_MINUS:
        case PLC_TOKEN_PLUS:
            printf("[INFO] Type: %s, Token: %s\n", plc_token_type_as_cstr(token.type), token.data.string.contents);
            break;
        default:
            fprintf(stderr, "[PANIC] Unreachable TOKEN TYPE\n");
            return false;
        }
    }
    return true;
}

bool plc_free_tokens(Plc_Tokens *tokens)
{
    if (!tokens) return false;
    for (size_t i = 0; i < tokens->count; ++i) {
        Plc_Token *token = &tokens->contents[i];
        switch (token->type) {
        case PLC_TOKEN_STRING:
            dyn_array_delete(&token->data.string);
            break;
        case PLC_TOKEN_DIV:
        case PLC_TOKEN_MINUS:
        case PLC_TOKEN_PLUS:
        case PLC_TOKEN_MULT:
        case PLC_TOKEN_NUMBER:
        default:
            break;
        }
    }
    dyn_array_delete(tokens);
    return true;
}

int plc_repl(void)
{
    char code[256] = {0};
    Plc_Tokens tokens = {0};
    Plc_Tokens stack = {0};

    fprintf(stdout, "repl> ");
    while (fgets(code, 256, stdin)) {
        Plc_Lexer lexer = plc_lexer_init(code, strlen(code));

        if (!plc_lexer_tokenize(&lexer, &tokens)) break;
        if (!plc_parse_tokens(&tokens, &stack)) break;

        stack.count = 0;
        tokens.count = 0;

        fprintf(stdout, "repl> ");
    }
    return 0;
}

int main(int argc, char **argv)
{
    const char *program = plc_shift_args(&argc, &argv);
    if (argc <= 0) {
        fprintf(stdout, "%s Interpreter\n", program);
        return plc_repl();
    }

    const char *file_path = plc_shift_args(&argc, &argv);
    size_t code_len = 0;
    char *code = plc_read_file_into_memory(file_path, &code_len);
    Plc_Lexer lexer = plc_lexer_init(code, code_len);

    Plc_Tokens tokens = {0};
    Plc_Tokens stack = {0};
    if (!plc_lexer_tokenize(&lexer, &tokens)) {
        free(code);
        if (!plc_free_tokens(&tokens)) return 1;
        return 1;
    }

    if (!plc_parse_tokens(&tokens, &stack)) {
        free(code);
        if (!plc_free_tokens(&tokens)) return 1;
        if (!plc_free_tokens(&stack)) return 1;
        return 1;
    }

    free(code);
    if (!plc_free_tokens(&tokens)) return 1;
    if (!plc_free_tokens(&stack)) return 1;
    return 0;
}
