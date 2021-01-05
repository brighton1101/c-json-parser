#ifndef CJSON_H
#define CJSON_H

#define bool int
#define true 1
#define false 0

#define STR_END '\0'

typedef enum {
    JSON_NUL = 0,  // null
    JSON_OBJ = 1,  // object
    JSON_ARR = 2,  // array
    JSON_STR = 3,  // string
    JSON_BOO = 4,  // boolean
    JSON_INT = 5,  // integer
    JSON_FLO = 6,  // float
    JSON_OUT = 7,  // outer wrapper
} json_jsontoken_type;

typedef struct json_jsontoken json_jsontoken;
typedef struct json_jsontoken_list json_jsontoken_list;

struct json_jsontoken {
    json_jsontoken_type type;
    json_jsontoken* parent;
    json_jsontoken_list* children;
    int start_in; /** start index of token */
    int end_in; /** End index of token */
    bool error; /** 1 if error exists, 0 otherwise **/
};

json_jsontoken* json_jsontoken_create(json_jsontoken_type type, json_jsontoken *parent);

#define JSON_JSONTOKEN_LIST_START_CAP 10
struct json_jsontoken_list {
    json_jsontoken** tokens;
    int length;
    int capacity;
};

json_jsontoken_list* json_jsontoken_list_create(int capacity);
void json_jsontoken_list_append(json_jsontoken_list *list, json_jsontoken *t);

typedef struct json_parser json_parser;
struct json_parser {
    json_jsontoken_list* all_tokens;
    char* input;
    int start;
    int curr;
    int end;
};
json_parser* json_parser_create(char *input_source);

bool json_parsearr(json_parser *parser, json_jsontoken *parent);
bool json_parseobj(json_parser *parser, json_jsontoken *parent);
bool json_parsestr(json_parser *parser, json_jsontoken *parent);
bool json_parsenum(json_parser *parser, json_jsontoken *parent);
bool json_parsebool(json_parser *parser, json_jsontoken *parent);
bool json_parsenull(json_parser *parser, json_jsontoken *parent);

#endif /* CJSON_H */