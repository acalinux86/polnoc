#include "./polnoc_lexer.h"
#include "./polnoc_parser.h"

// Private Functions
static inline bool _plc_is_opcode(char x)
{
    return x == '+' || x == '*' || x == '/' || x == '-';
}

static inline char _plc_lexer_peek(const Plc_Lexer *lexer, size_t ahead)
{
    return lexer->content[lexer->cursor + ahead];
}

static inline char _plc_lexer_advance(Plc_Lexer *lexer)
{
    return lexer->content[lexer->cursor++];
}

static inline bool _plc_lexer_end(const Plc_Lexer *lexer)
{
    return lexer->cursor == lexer->content_len;
}

static inline Plc_Token _plc_create_token_binop(Plc_TokenType type, char binop)
{
    Plc_Token token  = {0};
    token.type       = type;
    token.data.binop = binop;
    return token;
}

static inline Plc_Token _plc_create_token_number(Plc_TokenType type, double number)
{
    Plc_Token token   = {0};
    token.type        = type;
    token.data.number = number;
    return token;
}

static bool _plc_add_token(Plc_Tokens *tokens, Plc_Token token)
{
    if (!tokens) return false;
    dyn_array_append(tokens, token);
    return true;
}

// Public Functions
const char *plc_token_type_as_cstr(const Plc_TokenType type)
{
    switch (type) {
    case PLC_TOKEN_NUMBER: return "PLC_TOKEN_NUMBER";
    case PLC_TOKEN_STRING: return "PLC_TOKEN_STRING";
    case PLC_TOKEN_PLUS:   return "PLC_TOKEN_PLUS";
    case PLC_TOKEN_MINUS:  return "PLC_TOKEN_MINUS";
    case PLC_TOKEN_DIV:    return "PLC_TOKEN_DIV";
    case PLC_TOKEN_MULT:   return "PLC_TOKEN_MULT";
    default:
        fprintf(stderr, "[PANIC] Unreachable TOKEN TYPE\n");
        return NULL;
    }
}

Plc_Lexer plc_lexer_init(const char *source, size_t size)
{
    Plc_Lexer lexer   = {0};
    lexer.cursor      = 0;
    lexer.content     = source;
    lexer.content_len = size;
    return lexer;
}

bool plc_lexer_tokenize(Plc_Lexer *l, Plc_Tokens *tokens)
{
    if (!tokens)     return false;
    if (!l->content) return false;

    char c;
    while (!_plc_lexer_end(l)) {
        Plc_Token token = {0};
        c = _plc_lexer_peek(l,0);
        if (isalpha(c)) {
            dyn_array_append(&token.data.string, c);
            _plc_lexer_advance(l);

            while (!isspace(_plc_lexer_peek(l, 0)) && _plc_lexer_peek(l, 0) != '\0') {
                c = _plc_lexer_peek(l,0);
                dyn_array_append(&token.data.string, c);
                _plc_lexer_advance(l);
            }
            dyn_array_append(&token.data.string, '\0');

            token.type = PLC_TOKEN_STRING;
            dyn_array_append(tokens, token);
        } else if (isspace(c)) {
            _plc_lexer_advance(l);
            dyn_array_delete(&token.data.string);
        } else {
            if (_plc_is_opcode(c)) {
                _plc_lexer_advance(l);
                if (c == '+') {
                    Plc_Token token = _plc_create_token_binop(PLC_TOKEN_PLUS, '+'); // '+'
                    if (!_plc_add_token(tokens, token)) return false; // Append token plus
                } else if (c == '-') {
                    Plc_Token token = _plc_create_token_binop(PLC_TOKEN_MINUS, '-'); // '-'
                    if (!_plc_add_token(tokens, token)) return false; // Append token minus
                } else if (c == '*') {
                    Plc_Token token = _plc_create_token_binop(PLC_TOKEN_MULT, '-'); // '*'
                    if (!_plc_add_token(tokens, token)) return false; // Append token mult
                } else if (c == '/') {
                    Plc_Token token = _plc_create_token_binop(PLC_TOKEN_DIV, '/'); // '/'
                    if (!_plc_add_token(tokens, token)) return false; // Append token division
                } else {
                    // Unknown Opcode
                    return false;
                }
            } else if (isdigit(c)) {
                // Consume Rest of Number
                dyn_array_append(&token.data.string, c);
                _plc_lexer_advance(l);

                while (isdigit(_plc_lexer_peek(l,0)) || _plc_lexer_peek(l, 0) == '.') {
                    c = _plc_lexer_peek(l,0);
                    dyn_array_append(&token.data.string, c);
                    _plc_lexer_advance(l);
                }
                dyn_array_append(&token.data.string, '\0');

                const char *str_num = token.data.string.contents;
                double number = plc_parse_number(str_num);
                dyn_array_delete(&token.data.string);

                Plc_Token t = _plc_create_token_number(PLC_TOKEN_NUMBER, number);
                if (!_plc_add_token(tokens, t)) return false; // Append token Number
            }
        }
    }
    return true;
}
