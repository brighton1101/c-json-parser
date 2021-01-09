#include "../cjson.h"
#include <stdlib.h>
#include <stdio.h>

char* json_readfile(char *path)
{
    FILE *file = fopen(path, "r");
    char *res;
    unsigned int i = 0;
    int c;

    if (file == NULL) return NULL;
    fseek(file, 0, SEEK_END);
    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    res = malloc(f_size * sizeof(char));

    while ((c = fgetc(file)) != EOF)
    {
        res[i++] = (char) c;
    }

    res[i] = '\0';
    fclose(file);
    return res;
}

int main()
{
    char *reddit = json_readfile("./javascript_reddit.json");
    json_parser *p = json_parser_create(reddit);
    bool res = json_parseobj(p, p->all_tokens->tokens[0]);


    /* Get first child of outer token object (which should be outer json object */
    json_jsontoken *obj = p->all_tokens->tokens[0]->children->tokens[0];
    printf("%d\n", obj->children->length);

    /* Print available keys in first level */
    for (int i = 0; i < obj->children->length; i++) {
        printf("key %d: ", i);
        for (int j = obj->children->tokens[i]->start_in; j < obj->children->tokens[i]->end_in; j++) {
            printf("%c", p->input[j]);
        }
        printf("\n");
    }
    /*
        Notice that obj has two children: `kind` and `data`. And since objs' type is `JSON_OBJ`, we
        know that it's children are keys. Each key will have children of its own. Print out the child
        of the data key.
     */
    json_jsontoken* datkey = obj->children->tokens[1]->children->tokens[0];
    for (int j = datkey->start_in; j < datkey->end_in; j++)
        printf("%c", p->input[j]);
    printf("\n");
    json_parser_cleanup(p);
    free(reddit);
}