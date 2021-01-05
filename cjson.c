#include "cjson.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h> /*for testing*/

/** reference: https://en.cppreference.com/w/cpp/string/byte/isspace */
bool json_iswhitespace(char c) {
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
    list->tokens = malloc(sizeof(json_jsontoken*) * capacity);
    list->length = 0;
    return list;
}

void
json_jsontoken_list_append
(json_jsontoken_list *list, json_jsontoken *t)
{
    if (list->length == list->capacity) {
        list->capacity *= 2;
        list->tokens =
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
    parser->all_tokens = json_jsontoken_list_create(10);
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

/*
Given that a str is detected
  - parse/create str token
  - add token to parents children
NOTE:
this assumes the char array looks something like this:
{'"', 'a', 'b', 'c', '"'}
 */
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
        json_jsontoken_list_append(
            parent->children,
            strtoken
        );
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
        else if (curr_c == '"' && !slshd) {
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
    parser->curr++;
    booltoken->end_in = parser->curr - 1;
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
    bool is_first = false;
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
        else if (json_iswhitespace(curr_c)) {
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
                continue;
            case '.':
                if (seen_dec) {
                    parent->error = true;
                    return false;
                }
                seen_dec = true;
                continue;
            case 'E':
            case 'e':
                if (seen_e) {
                    parent->error = true;
                    return false;
                }
                seen_e = true;
            case '-':
                if (is_first) {
                    seen_neg = true;
                    continue;
                }
                else if ((seen_neg && !seen_e) || 
                    (seen_e && seen_neg_after_e)) {
                    parent->error = true;
                    return false;
                } else if (seen_e) {
                    seen_neg_after_e = true;
                    continue;
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
    int i = 0;
    for (i; i < 4; i++) {
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
            if (json_isnumericalishchar(curr_c)) {
                parser->curr--;
                if (!json_parsenum(parser, arrtoken))
                    err_seen = true;
            } else if (curr_c == '\"') {
                parser->curr--;
                if (!json_parsestr(parser, arrtoken))
                    err_seen = true;
            } else if (curr_c == 'n') {
                parser->curr--;
                if (!json_parsenull(parser, arrtoken))
                    err_seen = true;
            } else if (curr_c == 't' || curr_c == 'f') {
                parser->curr--;
                if (!json_parsenull(parser, arrtoken))
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
    
}

int main() {
    char *test = "[\n\tnull,\n\t\"Hello world\"]";
    json_parser *parser = json_parser_create(test);
    bool res = json_parsearr(parser, parser->all_tokens->tokens[0]);
    printf("res: %d \n", res);
    printf("%d %d \n", parser->all_tokens->tokens[1]->start_in, parser->all_tokens->tokens[1]->end_in);
    printf("%s \n", parser->input + parser->all_tokens->tokens[1]->start_in);
    printf("%d \n", parser->all_tokens->length);
    char* dest = (char*) malloc(sizeof(char) * (parser->all_tokens->tokens[3]->end_in-parser->all_tokens->tokens[3]->start_in+1));
    char *out = strncpy(dest, test+parser->all_tokens->tokens[3]->start_in, parser->all_tokens->tokens[3]->end_in-parser->all_tokens->tokens[3]->start_in);
    printf("%s \n", dest);
    printf("%d %d \n", parser->all_tokens->tokens[3]->start_in, parser->all_tokens->tokens[3]->end_in);
}