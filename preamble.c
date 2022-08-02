DEF_PARAM_STRUCT(double_param, double);
DEF_PARAM_STRUCT(int_param, int);

/*From here on it is assumed only 2 data types are used for params.
This struct exists mostly to make function parameters look nicer.
*/
typedef struct
{
    double_param *doubles;
    int_param *ints;
} params;

/* Struct used to represent the local values of a variable along the nozzle.*/
typedef struct
{
    int form_info;
    char short_name[NAME_MAX_LEN];
    char long_name[VAR_LONG_LEN];
    double *values;
} local_var;

typedef struct
{
    int initialized[2];
    local_var *iter_vars;
    local_var *refsol_vars;
} local_data;

typedef struct
{
    char name[NAME_MAX_LEN];
    int (*function)(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr,
                    params *parameters, local_data data);
} func;

/* find a parameter with a given name, no matter the data type,
   and return the position of the parameter in the array */
int get_param_id(char *param_name, int *ctr,
                 double_param *double_params, int_param *int_params)
{
    for (int i = 0; i < ctr[DOUBLE_PARAM_CTR]; i++)
        if (strcmp(param_name, double_params[i].name) == 0)
            return i;
    for (int i = 0; i < ctr[INT_PARAM_CTR]; i++)
        if (strcmp(param_name, int_params[i].name) == 0)
            return (ctr[DOUBLE_PARAM_CTR] + i);
    return -1;
}

int get_int_param(char *param_name, int *ctr, params parameters)
{
    double_param *double_params = parameters.doubles;
    int_param *int_params = parameters.ints;
    int id = get_param_id(param_name, ctr, double_params, int_params);
    if (id < 0)
    {
        printf("%s %s\n", COMMENT_PARAM_NOT_FOUND, param_name);
        return -2;
    }
    if (id < ctr[DOUBLE_PARAM_CTR])
    {
        printf("%s %s\n", COMMENT_TYPE_ERROR_INT, param_name);
        return -1;
    }
    if (id < ctr[DOUBLE_PARAM_CTR] + ctr[INT_PARAM_CTR])
    {
        id -= ctr[DOUBLE_PARAM_CTR];
        if (int_params[id].value == PARAM_NDEF)
            return -3;
        return int_params[id].value;
    }
}

double get_double_param(char *param_name, int *ctr, params parameters)
{
    double_param *double_params = parameters.doubles;
    int_param *int_params = parameters.ints;
    int id = get_param_id(param_name, ctr, double_params, int_params);
    if (id < 0)
    {
        printf("%s %s\n", COMMENT_PARAM_NOT_FOUND, param_name);
        return -2;
    }
    if (id < ctr[DOUBLE_PARAM_CTR])
    {
        if (double_params[id].value < 0.)
            return -3;
        return double_params[id].value;
    }
    if (id < ctr[DOUBLE_PARAM_CTR] + ctr[INT_PARAM_CTR])
    {
        printf("%s %s\n", COMMENT_TYPE_ERROR_DOUBLE, param_name);
        return -1;
    }
    return -4;
}

int read_all_params(int *ctr, double_param *double_params,
                    int_param *int_params)
{
    int scanned_vars;
    char read_row[ROW_LEN], read_name[ROW_LEN];

    double read_double[ctr[DOUBLE_PARAM_CTR]];
    FILE *params_file = fopen(PARAM_FILE_NAME, "r");
    if (params_file == NULL)
    {
        puts(COMMENT_FILE_NOT_FOUND_READ);
        exit(-2);
    }
    for (int j = 0; j < ctr[DOUBLE_PARAM_CTR]; j++)
    {
        fgets(read_row, ROW_LEN, params_file);
        scanned_vars = sscanf(read_row, "%s %lf [^\n]\n", read_name, &read_double[j]);
        if (scanned_vars == 2)
        {
            if (strcmp(read_name, double_params[j].name) == 0)
                continue;
            else
            {
                puts(COMMENT_FILE_WRONG_ORDER);
                fclose(params_file);
                return -1;
            }
        }
        else
        {
            puts(COMMENT_FILE_GENERIC_ERROR);
            fclose(params_file);
            return -1;
        }
    }

    int read_int[ctr[INT_PARAM_CTR]];
    for (int k = 0; k < ctr[INT_PARAM_CTR]; k++)
    {
        fgets(read_row, ROW_LEN, params_file);
        scanned_vars = sscanf(read_row, "%s %d [^\n]", read_name, &read_int[k]);
        if (scanned_vars == 2)
        {
            if (strcmp(read_name, int_params[k].name) == 0)
                continue;
            else
            {
                puts(COMMENT_FILE_WRONG_ORDER);
                fclose(params_file);
                return -1;
            }
        }
        else
        {
            puts(COMMENT_FILE_GENERIC_ERROR);
            fclose(params_file);
            return -1;
        }
    }

    if (fgets(read_row, ROW_LEN, params_file) != NULL)
        puts(COMMENT_FILE_EXTRA_DATA);

    fclose(params_file);

    for (int j = 0; j < ctr[DOUBLE_PARAM_CTR]; j++)
    {
        double_params[j].value = read_double[j];
    }
    for (int k = 0; k < ctr[INT_PARAM_CTR]; k++)
    {
        int_params[k].value = read_int[k];
    }

    return 0;
}

void print_all_params(FILE *dest, int *ctr,
                      double_param *double_params, int_param *int_params)
{
    int print_int;
    int error_present = 0;
    for (int j = 0; j < ctr[DOUBLE_PARAM_CTR]; j++)
    {
        if (double_params[j].value >= 0)
        {
            if (double_params[j].value == 0)
            {
                double_params[j].value = 0;
            } /* -0. */
            fprintf(dest, "%-" STR(NAME_MAX_LEN) "s ", double_params[j].name);
            if(dest == stdout)
            {
                char double_dec_str[8];
                snprintf(double_dec_str, 8, "%c-%dlf ", '%', (FILE_DECIMALS + 6));
                fprintf(dest, double_dec_str, double_params[j].value);
            }
            else
            {
                fprintf(dest, "%-." STR(FILE_DECIMALS) "E ", double_params[j].value);
            }
            fprintf(dest, "%-" STR(DESC_MAX_LEN) "s ", double_params[j].desc);
            fprintf(dest, "\n");
        }
        else
        {
            /* extra sauce to preserve row length
            if any value is negative, notify the user */
            fprintf(dest, "%-" STR(NAME_MAX_LEN) "s ", double_params[j].name);
            if(dest == stdout)
            {
                char decimals_char_string[7];
                snprintf(decimals_char_string, 7, "%c-%ds ", '%', (FILE_DECIMALS + 6));
                if(double_params[j].value == -1)
                    fprintf(dest, decimals_char_string, COMMENT_NDEF_PARAM);
                else if(double_params[j].value == -2)
                    fprintf(dest, decimals_char_string, COMMENT_NDEF_SHOCK);
            }
            else
            {
                char decimals_value_string[8];
                snprintf(decimals_value_string, 8, "%c-.%dE ", '%', (FILE_DECIMALS - 1));
                fprintf(dest, decimals_value_string, double_params[j].value);
            }
            fprintf(dest, "%-" STR(DESC_MAX_LEN) "s ", double_params[j].desc);
            fprintf(dest, "\n");
            error_present = 1;
        }
    }
    for (int k = 0; k < ctr[INT_PARAM_CTR]; k++)
    {
        char decimals_string[7];
        fprintf(dest, "%-" STR(NAME_MAX_LEN) "s ", int_params[k].name);
        if (int_params[k].value < 0)
        {
            error_present = 1;
            if(dest == stdout)
            {
                snprintf(decimals_string, 7, "%c-%ds ", '%', (FILE_DECIMALS + 6));
                if(int_params[k].value == -1)
                    fprintf(dest, decimals_string, COMMENT_NDEF_PARAM);
                else if(int_params[k].value == -2)
                    fprintf(dest, decimals_string, COMMENT_NDEF_SHOCK);
                else
                {
                    snprintf(decimals_string, 7, "%c-%dd ", '%', (FILE_DECIMALS + 6));
                    fprintf(dest, decimals_string, int_params[k].value);
                }
            }
            else
            {
                snprintf(decimals_string, 7, "%c-%dd ", '%', (FILE_DECIMALS + 6));
                fprintf(dest, decimals_string, int_params[k].value);
            }
        }
        else
        {
            snprintf(decimals_string, 7, "%c-%dd ", '%', (FILE_DECIMALS + 6));
            fprintf(dest, decimals_string, int_params[k].value);
        }
        fprintf(dest, "%-" STR(DESC_MAX_LEN) "s ", int_params[k].desc);
        fprintf(dest, "\n");
    }
    if (dest == stdout && error_present)
    {
        puts("");
        puts(COMMENT_NDEF_PARAMS);
    }
}

void save_all_params(int *ctr, double_param *double_params, int_param *int_params)
{
    FILE *params_file = fopen(PARAM_FILE_NAME, "w");
    if (params_file == NULL)
    {
        puts(COMMENT_FILE_NOT_FOUND_SAVE);
        exit(-2);
    }
    print_all_params(params_file, ctr, double_params, int_params);
    fclose(params_file);
}

int get_prop_id(char *prop_name, int *ctr, local_data data, int src)
{
    if (src == REFSOL_VARS)
    {
        for (int i = 0; i < ctr[LOC_VAR_CTR]; i++)
            if (strcmp(prop_name, data.refsol_vars[i].short_name) == 0)
                return i;
    }
    if (src == ITER_VARS)
    {
        for (int i = 0; i < ctr[LOC_VAR_CTR]; i++)
            if (strcmp(prop_name, data.iter_vars[i].short_name) == 0)
                return i;
    }
    return -1;
}
/* for internal use - no safety checks*/
int set_int_param(int param_value, char *param_name, int *ctr, params parameters)
{
    int param_id = get_param_id(param_name, ctr, parameters.doubles, parameters.ints);
    if (ctr[DOUBLE_PARAM_CTR] <= param_id && param_id < (ctr[DOUBLE_PARAM_CTR] + ctr[INT_PARAM_CTR]))
    {
        parameters.ints[param_id - ctr[DOUBLE_PARAM_CTR]].value = param_value;
        return 0;
    }
    else
    {
        printf("%s %s\n", COMMENT_PARAM_TYPE_ERROR, param_name);
        return -1;
    }
    save_all_params(ctr, parameters.doubles, parameters.ints);
}

int set_double_param(double param_value, char *param_name, int *ctr, params parameters)
{
    int param_id = get_param_id(param_name, ctr, parameters.doubles, parameters.ints);
    if(0 <= param_id && param_id < ctr[DOUBLE_PARAM_CTR])
    {
        parameters.doubles[param_id].value = param_value;
        return 0;
    }
    else
    {
        printf("%s %s\n", COMMENT_PARAM_TYPE_ERROR, param_name);
        return -1;
    }
    save_all_params(ctr, parameters.doubles, parameters.ints);
}

int print_all_data(FILE *dest, int *ctr, local_data data, params parameters, int src)
{
    char name_string[8];
    snprintf(name_string, 8, "%c-%ds ", '%', (FILE_DEC_SHORT + 6));
    if(src == REFSOL_VARS && data.initialized[0] == INITIALIZED)
    {
        for (int j = 0; j < ctr[LOC_VAR_CTR]; j++)
        {
            fprintf(dest, name_string, data.refsol_vars[j].short_name);
        }
        fprintf(dest, "\n");
        for (int i = 0; i < ctr[RSOL_N_CTR]; i++)
        {
            for (int j = 0; j < ctr[LOC_VAR_CTR]; j++)
            {
                fprintf(dest, "%-." STR(FILE_DEC_SHORT) "E ", data.refsol_vars[j].values[i]);
            }
            fprintf(dest, "\n");
        }
        return 0;
    }
    if(src == ITER_VARS && data.initialized[1] == INITIALIZED)
    {
        int n_id = get_param_id("n", ctr, parameters.doubles, parameters.ints);
        for (int j = 0; j < ctr[LOC_VAR_CTR]; j++)
        {
            fprintf(dest, name_string, data.iter_vars[j].short_name);
        }
        fprintf(dest, "\n");
        for (int i = 0; i < parameters.ints[n_id - ctr[DOUBLE_PARAM_CTR]].value; i++)
        {
            for (int j = 0; j < ctr[LOC_VAR_CTR]; j++)
            {
                fprintf(dest, "%-." STR(FILE_DEC_SHORT) "E ", data.iter_vars[j].values[i]);
            }
            fprintf(dest, "\n");
        }
        return 0;
    }
    return -1;
}

int print_data_relation(FILE *dest, char *name1, char *name2, 
    int *ctr, local_data data, params parameters, int src)
{
    int id1 = get_prop_id(name1, ctr, data, src);
    int id2 = get_prop_id(name2, ctr, data, src);
    if(id1 < 0 || id2 < 0)
    {
        printf("%s %s %s\n", COMMENT_INVALID_DATA_PRINT, name1, name2);
        return -1;
    }
    char name_string[8];
    snprintf(name_string, 8, "%c-%ds ", '%', (FILE_DEC_SHORT + 6));
    if(src == REFSOL_VARS && data.initialized[0] == INITIALIZED)
    {
        fprintf(dest, name_string, data.refsol_vars[id1].short_name);
        fprintf(dest, name_string, data.refsol_vars[id2].short_name);
        fprintf(dest, "\n");
        for (int i = 0; i < ctr[RSOL_N_CTR]; i++)
        {
            fprintf(dest, "%-." STR(FILE_DEC_SHORT) "E ", data.refsol_vars[id1].values[i]);
            fprintf(dest, "%-." STR(FILE_DEC_SHORT) "E ", data.refsol_vars[id2].values[i]);
            fprintf(dest, "\n");
        }
        return 0;
    }
    if(src == ITER_VARS && data.initialized[1] == INITIALIZED)
    {
        int n_id = get_param_id("n", ctr, parameters.doubles, parameters.ints);
        fprintf(dest, name_string, data.iter_vars[id1].short_name);
        fprintf(dest, name_string, data.iter_vars[id2].short_name);
        fprintf(dest, "\n");
        for (int i = 0; i < parameters.ints[n_id - ctr[DOUBLE_PARAM_CTR]].value; i++)
        {
            fprintf(dest, "%-." STR(FILE_DEC_SHORT) "E ", data.iter_vars[id1].values[i]);
            fprintf(dest, "%-." STR(FILE_DEC_SHORT) "E ", data.iter_vars[id2].values[i]);
            fprintf(dest, "\n");
        }
        return 0;
    }
    return -2;
}

/*
input parser rekurzivni verze (to asi neni dobry napad)

int parse_input(FILE *src, int layer, int *ctr,
                double_param *double_params, int_param *int_params)
{
    if (src == stdin)
        puts(COMMENT_REQUEST_INPUT);

    char scanned_input[STDIN_BUFFER], args[ARGS_COUNT][STDIN_BUFFER];
    nainicializovani prazdnych stringu v argumentech
    for (int i = 0; i < ARGS_COUNT; i++)
        args[i][0] = '\0';

    fgets(scanned_input, STDIN_BUFFER, src);
    if (scanned_input == NULL)
    {
        if (src != stdin)
        {
            puts(COMMENT_EOF_REACHED);
            parse_input(stdin, ++layer, ctr, double_params, int_params);
        }
        else return;
    }
    else
    {
        sscanf(scanned_input, "%s %s %s %s %s", args[0], args[1], args[2], args[3], args[4]);
        if (strcmp(args[0], "exit") == 0) exit(-1);
        if (strcmp(args[0], "params") == 0)
        {
            IO_params(args, ctr, double_params, int_params);
            parse_input(src, ++layer, ctr, double_params, int_params);
        }
        if (strcmp(args[0], "execute") == 0)
        {
            FILE *script_file = fopen(args[1], "r");
            if (script_file != NULL)
            {
                parse_input(script_file, ctr, double_params, int_params);
                okomentovat
            }
        }
    }

    puts(COMMENT_INVALID_COMMAND_STDIN);
    args[5] = COMMENT_INVALID_COMMAND_FILE;
    parse_input(src, ++layer, ctr, double_params, int_params);
}
*/