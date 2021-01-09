# Single pass JSON parser in C

Creates a tree representing a JSON object built with tokens:
```

c```
struct json_jsontoken {
    json_jsontoken_type type;
    json_jsontoken* parent;
    json_jsontoken_list* children;
    int start_in; /** start index of token in input string */
    int end_in; /** End index of token in input string */
    bool error; /** 1 if error exists, 0 otherwise **/
};
```

- No dependencies outside of `stdlib`
- No complicated datastructures
- No copying of values around

Tests using [Catch2](https://github.com/catchorg/Catch2)
