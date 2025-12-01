#ifndef POLNOC_PARSER_H
#define POLNOC_PARSER_H

#include <stdbool.h>

#include "./dyn_array.h"

double plc_parse_number(const char *number_string);
bool plc_parse_tokens(const Plc_Tokens *tokens, Plc_Tokens *stack);

#endif // POLNOC_PARSER_H
