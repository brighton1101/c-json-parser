/*
MIT License

Copyright (c) 2021 Brighton Balfrey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef CJSON_H
#define CJSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define bool int
#define true 1
#define false 0

#define STR_END '\0'

/* Starting capacity of token lists */
#define JSON_JSONTOKEN_LIST_START_CAP 10

/* Expansion factor for lists */
#define JSON_JSONTOKEN_LIST_EXPANSION(prev_cap) (2*prev_cap)

/** Forward declaration of tokens and list to hold tokens */
typedef struct json_jsontoken json_jsontoken;
typedef struct json_jsontoken_list json_jsontoken_list;
typedef struct json_parser json_parser;

/** Available types of tokens */
typedef enum {
    JSON_NUL = 0,  /** null */
    JSON_OBJ = 1,  /** object */
    JSON_ARR = 2,  /** array */
    JSON_STR = 3,  /** string */
    JSON_BOO = 4,  /** boolean */
    JSON_INT = 5,  /** integer */
    JSON_FLO = 6,  /** float */
    JSON_OUT = 7,  /** outer wrapper */
} json_jsontoken_type;

/** Token definition */
struct json_jsontoken {
    json_jsontoken_type type;
    json_jsontoken* parent;
    json_jsontoken_list* children;
    int start_in; /** start index of token */
    int end_in; /** End index of token */
    bool error; /** 1 if error exists, 0 otherwise **/
};

/** List definition */
struct json_jsontoken_list {
    json_jsontoken** tokens;
    int length;
    int capacity;
};

/* Representing the parser */
struct json_parser {
    json_jsontoken_list* all_tokens;
    char* input;
    int start;
    int curr;
    int end;
};

/** Forward definitions */
json_parser* json_parser_create(char *input_source);
void json_parser_cleanup(json_parser *parser);
json_jsontoken* json_jsontoken_create(json_jsontoken_type type, json_jsontoken *parent);
json_jsontoken_list* json_jsontoken_list_create(int capacity);
void json_jsontoken_list_append(json_jsontoken_list *list, json_jsontoken *t);
bool json_parsearr(json_parser *parser, json_jsontoken *parent);
bool json_parseobj(json_parser *parser, json_jsontoken *parent);
bool json_parsestr(json_parser *parser, json_jsontoken *parent);
bool json_parsenum(json_parser *parser, json_jsontoken *parent);
bool json_parsebool(json_parser *parser, json_jsontoken *parent);
bool json_parsenull(json_parser *parser, json_jsontoken *parent);

/** Implementation */

bool
json_iswhitespace(char c) {
    switch (c) {
        case 0x20:
        case 0x0c:
        case 0x0a:
        case 0x0d:
        case 0x09:
        case 0x0b:
            return true;
        default:
            return false;
    }
}

int
json_isnumericalishchar(char c) {
    switch (c) {
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return 1;
        default:
            return 0;
    }
}

json_jsontoken_list*
json_jsontoken_list_create(int capacity)
{
    json_jsontoken_list* list;
    list = (json_jsontoken_list*) malloc(sizeof(json_jsontoken_list));
    list->capacity = capacity;
    list->tokens = (json_jsontoken**) malloc(sizeof(json_jsontoken*) * capacity);
    list->length = 0;
    return list;
}

void
json_jsontoken_list_append
(json_jsontoken_list *list, json_jsontoken *t)
{
    if (list->length == list->capacity) {
        list->capacity = JSON_JSONTOKEN_LIST_EXPANSION(list->capacity);
        list->tokens = (json_jsontoken**)
            realloc(list->tokens, sizeof(json_jsontoken*) * list->capacity);
    }
    list->tokens[list->length++] = t;
}

json_jsontoken*
json_jsontoken_create(json_jsontoken_type type, json_jsontoken *parent)
{
    json_jsontoken *token = (json_jsontoken*) malloc(sizeof(json_jsontoken));
    token->type = type;
    token->parent = parent;
    token->children = json_jsontoken_list_create(JSON_JSONTOKEN_LIST_START_CAP);
    token->error = false;
    return token;
}

json_parser*
json_parser_create(char *input_source)
{
    json_parser *parser = (json_parser*) malloc(sizeof(json_parser));
    parser->all_tokens = json_jsontoken_list_create(JSON_JSONTOKEN_LIST_START_CAP);
    parser->start = 0;
    parser->input = input_source;
    parser->curr = 0;
    json_jsontoken *outer = json_jsontoken_create(JSON_OUT, NULL);
    json_jsontoken_list_append(
        parser->all_tokens,
        outer
    );
    return parser;
}

bool
json_parsestr(json_parser *parser, json_jsontoken *parent)
{
    json_jsontoken *strtoken = json_jsontoken_create(JSON_STR, parent);
    json_jsontoken_list_append(
        parser->all_tokens,
        strtoken
    );
    if (parser->input[parser->curr++] != '"') {
        strtoken->start_in = parser->curr - 1;
        strtoken->end_in = -1;
        parent->error = true;
        return false;
    }
    char curr_c;
    int slshd = 0;
    strtoken->start_in = parser->curr;
    strtoken->type = JSON_STR;
    while (1) {
        curr_c = parser->input[parser->curr++];
        if (curr_c == STR_END) {
            parent->error = true;
            return false;
        }
        else if (curr_c == '\"' && !slshd) {
            json_jsontoken_list_append(
                parent->children,
                strtoken
            );
            strtoken->end_in = parser->curr - 1;
            return true;
        }
        else if (curr_c == '\\') slshd = 2;
        else slshd = 1;
        --slshd;
    }
}

bool
json_parsebool(json_parser *parser, json_jsontoken *parent)
{
    json_jsontoken *booltoken = json_jsontoken_create(JSON_BOO, parent);
    json_jsontoken_list_append(
        parser->all_tokens,
        booltoken
    );
    bool is_truthy = false;
    char curr_c = parser->input[parser->curr++];
    switch (curr_c) {
        case 't':
            is_truthy = true;
        case 'f':
            break;
        default:
            parent->error = true;
            return false;
    }
    booltoken->start_in = parser->curr-1;
    unsigned int i = 0;
    char* false_exp = "alse";
    char* true_exp = "rue";
    for (; i < 4; i++) {
        curr_c = parser->input[parser->curr++];
        if (i == 3 && is_truthy) {
            parser->curr--;
            break;
        }
        else if (curr_c == STR_END ||
            (is_truthy && curr_c != true_exp[i]) ||
            (!is_truthy && curr_c != false_exp[i])) {
            parent->error = true;
            return false;
        }
    }
    json_jsontoken_list_append(
        parent->children,
        booltoken
    );
    booltoken->end_in = parser->curr;
    return true;
}

bool
json_parsenum(json_parser *parser, json_jsontoken *parent)
{
    json_jsontoken *numtoken = json_jsontoken_create(JSON_INT, parent);
    json_jsontoken_list_append(
        parser->all_tokens,
        numtoken
    );
    numtoken->start_in = parser->curr;
    bool is_first = true;
    bool seen_dec = false;
    bool seen_e = false;
    bool seen_neg = false;
    bool seen_neg_after_e = false;
    while (1) {
        char curr_c = parser->input[parser->curr++];
        if (
            (is_first && json_iswhitespace(curr_c)) || 
            curr_c == STR_END
        ) {
            parent->error = true;
            return false;
        }
        else if (json_iswhitespace(curr_c) || curr_c == ']' || curr_c == '}' || curr_c == ',') {
            parser->curr--;
            break;
        }
        switch (curr_c) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                break;
            case '.':
                if (seen_dec) {
                    parent->error = true;
                    return false;
                }
                seen_dec = true;
                break;
            case 'E':
            case 'e':
                if (seen_e) {
                    parent->error = true;
                    return false;
                }
                seen_e = true;
                break;
            case '-':
                if (is_first) {
                    seen_neg = true;
                    break;
                }
                else if ((seen_neg && !seen_e) || 
                    (seen_e && seen_neg_after_e)) {
                    parent->error = true;
                    return false;
                } else if (seen_e) {
                    seen_neg_after_e = true;
                    break;
                } else {
                    parent->error = true;
                    return false;
                }
            default:
                parent->error = true;
                return false;
        }
        is_first = false;
    }
    if (seen_dec) numtoken->type = JSON_FLO;
    numtoken->end_in = parser->curr;
    json_jsontoken_list_append(
        parent->children,
        numtoken
    );
    return true;
}

bool
json_parsenull(json_parser *parser, json_jsontoken *parent)
{
    json_jsontoken *nulltoken = json_jsontoken_create(JSON_NUL, parent);
    json_jsontoken_list_append(
        parser->all_tokens,
        nulltoken
    );
    char *expected = "null";
    nulltoken->start_in = parser->curr;
    for (int i = 0; i < 4; i++) {
        if (expected[i] != parser->input[parser->curr++]) {
            parent->error = true;
            return false;
        }
    }
    nulltoken->end_in = parser->curr;
    json_jsontoken_list_append(
        parent->children,
        nulltoken
    );
    return true;
}

bool
json_parsearr(json_parser *parser, json_jsontoken *parent)
{
    json_jsontoken *arrtoken = json_jsontoken_create(JSON_ARR, parent);
    json_jsontoken_list_append(
        parser->all_tokens,
        arrtoken
    );
    arrtoken->start_in = parser->curr;
    char curr_c = parser->input[parser->curr++];
    if (curr_c != '[') {
        parent->error = true;
        return false;
    }
    bool needs_comma = false;
    bool err_seen = false;
    while (1) {
        curr_c = parser->input[parser->curr++];
        if (json_iswhitespace(curr_c)) {
            continue;
        } else if (curr_c == ']') {
            break;
        } else if (curr_c == STR_END) {
            err_seen = true;
        } else if (curr_c == ',') {
            if (!needs_comma)
                err_seen = true;
            needs_comma = false;
        } else {
            parser->curr--;
            if (json_isnumericalishchar(curr_c)) {
                if (!json_parsenum(parser, arrtoken))
                    err_seen = true;
            } else if (curr_c == '\"') {
                if (!json_parsestr(parser, arrtoken))
                    err_seen = true;
            } else if (curr_c == 'n') {
                if (!json_parsenull(parser, arrtoken))
                    err_seen = true;
            } else if (curr_c == 't' || curr_c == 'f') {
                if (!json_parsebool(parser, arrtoken))
                    err_seen = true;
            } else if (curr_c == '{') {
                if (!json_parseobj(parser, arrtoken))
                    err_seen = true;
            } else if (curr_c == '[') {
                if (!json_parsearr(parser, arrtoken))
                    err_seen = true;
            } else {
                err_seen = true;
            }
            needs_comma = true;
        }
        if (err_seen) {
            parent->error = true;
            return false;
        }
    }
    json_jsontoken_list_append(
        parent->children,
        arrtoken
    );
    arrtoken->end_in = parser->curr;
    return true;
}

bool
json_parseobj(json_parser *parser, json_jsontoken *parent)
{
    json_jsontoken *objtoken = json_jsontoken_create(JSON_OBJ, parent);
    json_jsontoken_list_append(
        parser->all_tokens,
        objtoken
    );
    objtoken->start_in = parser->curr;
    char curr_c = parser->input[parser->curr++];
    if (curr_c != '{') {
        parent->error = true;
        return false;
    }
    bool is_key = true;
    bool needs_comma = false;
    bool err_seen = false;
    json_jsontoken* last_key;
    while (1) {
        curr_c = parser->input[parser->curr++];
        if (json_iswhitespace(curr_c))
            continue;
        else if (curr_c == '}')
            break;
        else if (curr_c == STR_END)
            err_seen = true;
        else if (curr_c == ':') {
            if (!is_key)
                err_seen = true;
            is_key = false;
        } else if (curr_c == ',') {
            if (!needs_comma)
                err_seen = true;
            needs_comma = false;
        } else if (is_key) {
            parser->curr--;
            if (!json_parsestr(parser, objtoken))
                err_seen = true;
            last_key = parser->all_tokens->tokens[parser->all_tokens->length - 1];
        } else {
            parser->curr--;
            if (json_isnumericalishchar(curr_c)) {
                if (!json_parsenum(parser, last_key))
                    err_seen = true;
            } else if (curr_c == '\"') {
                if (!json_parsestr(parser, last_key))
                    err_seen = true;
            } else if (curr_c == 'n') {
                if (!json_parsenull(parser, last_key))
                    err_seen = true;
            } else if (curr_c == 't' || curr_c == 'f') {
                if (!json_parsebool(parser, last_key))
                    err_seen = true;
            } else if (curr_c == '{') {
                if (!json_parseobj(parser, last_key))
                    err_seen = true;
            } else if (curr_c == '[') {
                if (!json_parsearr(parser, last_key))
                    err_seen = true;
            } else {
                err_seen = true;
            }
            is_key = true;
            needs_comma = true;
        }
        if (err_seen) {
            parent->error = true;
            return false;
        }
    }
    json_jsontoken_list_append(
        parent->children,
        objtoken
    );
    objtoken->end_in = parser->curr;
    return true;
}

void
json_parser_cleanup(json_parser *parser)
{
    /** Each token has several blocks of memory allocated:
        - the childrens' token list container
        - the childrens' token list struct
        - the token itself
    */
    for (int i = 0; i < parser->all_tokens->length; i++) {
        json_jsontoken *token = parser->all_tokens->tokens[i];
        free(token->children->tokens);
        free(token->children);
        free(token);
    }
    /** Parser follows the same pattern. */
    free(parser->all_tokens->tokens);
    free(parser->all_tokens);
    free(parser);
}

#ifdef __cplusplus
}
#endif

#endif /* CJSON_H */