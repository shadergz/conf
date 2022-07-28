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

    size_t file_line;

    conf_err_e actual_status;

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
    char *value_to_save, size_t value_buffer_size, conf_t *conf)
{
    assert(table && attribute && value_to_save && conf);

    /* Resetting the file pointer position */
    /* Same as rewind function */
    fseek(conf->file, 0L, SEEK_SET);

    conf->file_line = 0;

    char line_buffer[100];
    char table_data[100] = {'\0'};

    conf->actual_status = CONF_E_TABLE_NOT_FOUND;

    while (fgets(line_buffer, sizeof(line_buffer), conf->file) != NULL)
    {
        conf->file_line++;

        if (*table_data == '\0')
        {
            char *current_table = strrchr(line_buffer, '[');
            if (current_table != NULL)
            {
                current_table++;
                char *tableEnd = strrchr(current_table, ']');

                conf->actual_status = CONF_E_TABLE_END_NOT_FOUND;

                if (tableEnd == NULL)
                    break;
                
                uintptr_t size = tableEnd - current_table;
                strncpy(table_data, current_table, size);
                *(table_data + size) = '\0';

                if ((strncmp(table_data, table, strlen(table)) != 0))
                    /* After the tableSelected gets a address, the table_data will not beging modified anymore */
                    memset(table_data, '\0', sizeof(table_data));
            }
            continue;
        }

        conf->actual_status = CONF_E_ATTR_NOT_FOUND;

        char *bak, *tok;
        char *attr_name = strtok_r(line_buffer, "=", &bak);

        if (attr_name == NULL)
            break;

        /* Cutting the final space at the end */
        char *attributeEnd = strrchr(attr_name, ' ');
        if (attributeEnd)
            *attributeEnd = '\0';
        
        conf->actual_status = CONF_E_VALUE_NOT_FOUND;

        if ((strcmp(attr_name, attribute) != 0))
            continue;

        char *value = strtok_r(NULL, "=", &bak);

        if (value == NULL)
            break;
        
        conf->actual_status = CONF_E_OK;

        if (*value == ' ')
            value++;

        char *value_end = strpbrk(value + (strlen(value) - 1), "\n ");

        if (value_end != NULL)
            *value_end = '\0';

        strncpy(value_to_save, value, value_buffer_size);
        
        break;
    }

    return conf->actual_status;
}

int conf_print_err(conf_t *conf)
{
    assert(conf);

    return fprintf(stderr, "error at line %ld: %s\n", conf->file_line, conf_err_to_str(conf->actual_status));

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

