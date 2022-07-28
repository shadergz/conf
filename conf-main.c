#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

typedef enum
{
    CONF_E_OK = 0,
    CONF_E_OPEN_FILE,
    CONF_E_TABLE_NOT_FOUND,
    CONF_E_TABLE_END_NOT_FOUND,
    CONF_E_ATTR_NOT_FOUND,
    CONF_E_VALUE_NOT_FOUND

} conf_err_e;


typedef struct 
{
    const char *confname;

    size_t fileLine;

    conf_err_e actualStatus;

    FILE *file;

} conf_t;


conf_err_e conf_open(const char *path, conf_t *conf)
{
    assert(conf && path);
    
    conf->file = fopen(path, "r");
    if (conf->file == NULL)
        return CONF_E_OPEN_FILE;

    conf->confname = strdup(path);

    return CONF_E_OK;
}

conf_err_e conf_close(conf_t *conf)
{
    assert(conf);

    free((char*)conf->confname);
    fclose(conf->file);

    return CONF_E_OK;
}

const char *conf_err_to_str(conf_err_e err)
{
    static const char *conf_err_list[] = 
    {
        "everything is ok",
        "can't open for read the file associeted with the path",
        "table not found",
        "']' token not found at end of the table name",
        "attribute not found",
        "attribute value not found",
        NULL
    };

    return conf_err_list[err];
}

conf_err_e conf_parser(const char *table, const char *attribute, 
    char *valueToSave, size_t valuebufferSize, conf_t *conf)
{
    assert(table && attribute && valueToSave && conf);

    /* Reset the file pointer position */
    /* Same as rewind function */
    fseek(conf->file, 0L, SEEK_SET);

    conf->fileLine = 0;

    char linebuffer[100];
    char tableData[100] = {'\0'};

    conf->actualStatus = CONF_E_TABLE_NOT_FOUND;

    while (fgets(linebuffer, sizeof(linebuffer), conf->file) != NULL)
    {
        conf->fileLine++;

        if (*tableData == '\0')
        {
            char *currentTable = strrchr(linebuffer, '[');
            if (currentTable != NULL)
            {
                currentTable++;
                char *tableEnd = strrchr(currentTable, ']');

                conf->actualStatus = CONF_E_TABLE_END_NOT_FOUND;

                if (tableEnd == NULL)
                    break;
                
                uintptr_t size = tableEnd - currentTable;
                strncpy(tableData, currentTable, size);
                *(tableData + size) = '\0';

                if ((strncmp(tableData, table, strlen(table)) != 0))
                    /* After the tableSelected gets a address, the tableData will not beging modified anymore */
                    memset(tableData, '\0', sizeof(tableData));
            }
            continue;
        }

        conf->actualStatus = CONF_E_ATTR_NOT_FOUND;

        char *bak, *tok;
        char *attributeName = strtok_r(linebuffer, "=", &bak);

        if (attributeName == NULL)
            break;

        /* Cut the final space at the end */
        char *attributeEnd = strrchr(attributeName, ' ');
        if (attributeEnd)
            *attributeEnd = '\0';
        
        conf->actualStatus = CONF_E_VALUE_NOT_FOUND;

        if ((strcmp(attributeName, attribute) != 0))
            continue;

        char *value = strtok_r(NULL, "=", &bak);

        if (value == NULL)
            break;
        
        conf->actualStatus = CONF_E_OK;

        if (*value == ' ')
            value++;

        char *valueEnd = strpbrk(value + (strlen(value) - 1), "\n ");

        if (valueEnd != NULL)
            *valueEnd = '\0';

        strncpy(valueToSave, value, valuebufferSize);
        
        break;
    }

    return conf->actualStatus;
}

int conf_print_err(conf_t *conf)
{
    assert(conf);

    return fprintf(stderr, "error at line %ld: %s\n", conf->fileLine, conf_err_to_str(conf->actualStatus));

}

int main()
{
    conf_t conf;

    conf_err_e err = conf_open("user.conf", &conf);

    if (err != CONF_E_OK)
        return conf_print_err(&conf);

    char pi_value[20], user_name[20];

    conf_parser("Values", "PI", pi_value, sizeof(pi_value), &conf);
    err = conf_parser("User", "username", user_name, sizeof(user_name), &conf);

    if (err != CONF_E_OK)
        return conf_print_err(&conf);

    printf("PI value: %s\n", pi_value);
    printf("Username: %s\n", user_name);

    conf_close(&conf);


}

