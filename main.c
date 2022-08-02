#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* set user interaction language - CS (Czech) or EN (English) */
#define LANG CS

/*return the name of the macro as a string */
#define TAG(x) #x
/*return the value of the macro as a string (for real that's how it works)*/
#define STR(x) TAG(x)

/*All strings that depend on user language can be found in this file. */
#include "user_comm.h"

/*struct + param definitions & macros, basic param functions */
#include "preamble.h"
#include "preamble.c"

/*relations between properties based on NACA Report 1135 */
#include "formulae.h"

/*reference solution generator based on analytic quasi-1D ideal gas equations */
#include "refsol.c"

/*numerical solver*/
#include "iter.c"

#ifndef MAIN_FILE
#define MAIN_FILE "main.c"
#endif

void initialize_params_from_file(int *ctr, params *parameters, local_data data);

int get_user_input(int *ctr,
                   params *parameters, local_data data, func *funcs);

int main(void)
{
    if (FILE_DECIMALS > 93)
    { /* make sure that the decimals actually can be represented */
        THROW_BUFFER_ERR(COMMENT_DECIMALS, FILE_DECIMALS);
    }

    int ctr[CTR_ARRAY_LEN];
    memset(ctr, 0, CTR_ARRAY_LEN * sizeof(int));

    ctr[RSOL_N_CTR] = RSOL_N;

    /*careful around here
    array names are hard defined in the macros from preamble.h*/

    double_param *double_params =
        (double_param *)calloc(REF_PARAM_COUNT, sizeof(double_param));
    int_param *int_params =
        (int_param *)calloc(REF_PARAM_COUNT, sizeof(int_param));
    func *funcs = (func *)calloc(REF_FUNC_COUNT, sizeof(func));
    local_var *local_vars =
        (local_var *)calloc(REF_VARS_COUNT, sizeof(local_var));
    local_var *ref_local_vars =
        (local_var *)calloc(REF_VARS_COUNT, sizeof(local_var));

    /* change initialization later */
    local_data data = {{INITIALIZED, INITIALIZED}, local_vars, ref_local_vars};
    params parameters = (params){double_params, int_params};

    /* =================PARAMETER DEFINITIONS================================ */

    INIT_DOUBLE_PARAM("r_gas", DESC_RGAS, IO_CA);
    INIT_DOUBLE_PARAM("kappa", DESC_KAPPA, IO_CA);
    INIT_DOUBLE_PARAM("p0", DESC_P0, IO_CA);
    INIT_DOUBLE_PARAM("p1", DESC_P1, IO_CA);
    INIT_DOUBLE_PARAM("T0", DESC_T0, IO_CA);
    INIT_DOUBLE_PARAM("A_crit", DESC_ACRIT, IO_CNA);
    INIT_DOUBLE_PARAM("r_crit", DESC_RCRIT, NIO);
    INIT_DOUBLE_PARAM("x_crit", DESC_XCRIT, NIO);
    INIT_DOUBLE_PARAM("A_max", DESC_AMAX, IO_CNA);
    INIT_DOUBLE_PARAM("r_max", DESC_RMAX, NIO);
    INIT_DOUBLE_PARAM("l", DESC_L, NIO);
    INIT_DOUBLE_PARAM("alpha", DESC_VAR_ALPHA, IO_CNA);
    INIT_DOUBLE_PARAM("A_shock", DESC_AMAX, NIO);
    INIT_DOUBLE_PARAM("r_shock", DESC_RMAX, NIO);
    INIT_DOUBLE_PARAM("x_shock", DESC_XCRIT, NIO);
    INIT_DOUBLE_PARAM("delta_s", DESC_DELTA_S, NIO);
    INIT_DOUBLE_PARAM("eps", DESC_EPS, IO_CA);
    INIT_DOUBLE_PARAM("cfl", DESC_CFL, IO_CNA);

    INIT_INT_PARAM("n", DESC_N, IO_CNA);
    INIT_INT_PARAM("nc_r", DESC_N, NIO);
    INIT_INT_PARAM("nc_i", DESC_N, NIO);
    INIT_INT_PARAM("iterno", DESC_ITERNO, NIO);

    /* =================FUNCTION DEFINITIONS================================== */
    /* int func(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr, params *parameters,
    local_data data) */

    INIT_FUNC("params", IO_params);
    INIT_FUNC("refsol", IO_refsol);
    INIT_FUNC("data", IO_data);
    INIT_FUNC("plot", IO_plot);
    INIT_FUNC("iter", IO_iter);
    INIT_FUNC("reset", IO_reset);
    INIT_FUNC("res", IO_res);
    INIT_FUNC("help", IO_help);

    /* =====================VARIABLE DEFINITIONS============================= */

    INIT_VARIABLE("x", DESC_VAR_X, NOZZLE_PROP);
    INIT_VARIABLE("r", DESC_VAR_R, NOZZLE_PROP);
    INIT_VARIABLE("A", DESC_VAR_A, NOZZLE_PROP);
    INIT_VARIABLE("A_x", DESC_VAR_AX, NOZZLE_PROP);
    INIT_VARIABLE("p", DESC_VAR_P, ITER_PROP);
    INIT_VARIABLE("T", DESC_VAR_T, GENERIC_PROP);
    INIT_VARIABLE("rho", DESC_VAR_RHO, ITER_PROP);
    INIT_VARIABLE("c", DESC_VAR_C, ITER_PROP);
    INIT_VARIABLE("a", DESC_VAR_A_SND, ITER_PROP);
    INIT_VARIABLE("Ma", DESC_VAR_MACH, REFSOL_PROP);
    INIT_VARIABLE("h", DESC_VAR_H, GENERIC_PROP);
    INIT_VARIABLE("h_t", DESC_VAR_HT, GENERIC_PROP);
    INIT_VARIABLE("s", DESC_VAR_S, REFSOL_PROP);
    INIT_VARIABLE("m_flux", DESC_VAR_MFLUX, GENERIC_PROP);
    INIT_VARIABLE("P", DESC_VAR_PP, ITER_PROP);
    INIT_VARIABLE("e", DESC_VAR_EN, ITER_PROP);

#if 0
    double_params =
        (double_param *)
            realloc(double_params, ctr[DOUBLE_PARAM_CTR] * sizeof(double_param));
    int_params =
        (int_param *)realloc(int_params, ctr[INT_PARAM_CTR] * sizeof(int_param));
    funcs = (func *)realloc(funcs, ctr[FUNC_CTR] * sizeof(func));
    local_vars = (local_var *)
        realloc(local_vars, ctr[LOC_VAR_CTR] * sizeof(local_var));
    ref_local_vars = (local_var *)
        realloc(ref_local_vars, ctr[LOC_VAR_CTR] * sizeof(local_var));
#endif

    memcpy(ref_local_vars, local_vars, ctr[LOC_VAR_CTR] * sizeof(local_var));

    /* allocate memory for local variables and remove garbage values */
    for (int i = 0; i < ctr[LOC_VAR_CTR]; i++)
    {
        local_vars[i].values = (double *)calloc(RSOL_N, sizeof(double));
        ref_local_vars[i].values = (double *)calloc(RSOL_N, sizeof(double));
    }

    /* the magic begins here */

    initialize_params_from_file(ctr, &parameters, data);

    get_user_input(ctr, &parameters, data, funcs);

    /* end of high risk part */

    free(double_params);
    free(int_params);
    free(funcs);

    for (int i = 0; i < ctr[LOC_VAR_CTR]; i++)
    {
        free(local_vars[i].values);
        free(ref_local_vars[i].values);
    }

    free(local_vars);
    free(ref_local_vars);

    return 0;
}

int IO_refsol(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr,
              params *parameters, local_data data)
{
    refsol_func(ctr, data, parameters);
}

int IO_data(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr,
            params *parameters, local_data data)
{
    if (strcmp(args[1], "backup") == 0)
    {
        if (args[2][0] != '\0')
        {
            FILE *backup_file = fopen(args[2], "w");
            if (backup_file == NULL)
            {
                printf("%s %s\n", COMMENT_FOPEN_ERROR, args[2]);
                fclose(backup_file);
                return -1;
            }
            if (strcmp(args[3], "iter") == 0 || args[3][0] == '\0')
            {
                print_all_data(backup_file, ctr, data, *parameters, ITER_VARS);
                fclose(backup_file);
                return 0;
            }
            if (strcmp(args[3], "ref") == 0)
            {
                print_all_data(backup_file, ctr, data, *parameters, REFSOL_VARS);
                fclose(backup_file);
                return 0;
            }
            fclose(backup_file);
            return -1;
        }
        else
        {
            puts(COMMENT_MISSING_FILE_NAME);
            return -1;
        }
    }
    if (strcmp(args[1], "info") == 0)
    {
        if (args[2][0] != '\0')
            printf("%s %s %s\n", COMMENT_EXTRA_ARGS, args[0], args[1]);
        for (int i = 0; i < ctr[LOC_VAR_CTR]; i++)
        {
            printf("%-" STR(NAME_MAX_LEN) "s %s\n",
                   data.iter_vars[i].short_name,
                   data.iter_vars[i].long_name);
        }
        return 0;
    }
}

int IO_plot(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr,
            params *parameters, local_data data)
{
    FILE *datafile = fopen(PLOT_DATAFILE_NAME, "w");
    FILE *plotpipe = popen("gnuplot -persist", "w");
    if (datafile == NULL)
    {
        printf("%s %s\n", COMMENT_FOPEN_ERROR, PLOT_DATAFILE_NAME);
        puts(COMMENT_PLOT_DATAFILE_ERROR);
        fclose(datafile);
        pclose(plotpipe);
        return -2;
    }
    if (args[3][0] == '\0')
    {
        FILE *datafile1 = fopen(PLOT_DATAFILE_NAME2, "w");
        if (print_data_relation(datafile, args[1], args[2], ctr, data, *parameters, ITER_VARS) < 0)
        {
            fclose(datafile);
            pclose(plotpipe);
            return -3;
        }
        if (print_data_relation(datafile1, args[1], args[2], ctr, data, *parameters, REFSOL_VARS) < 0)
        {
            fclose(datafile);
            pclose(plotpipe);
            return -3;
        }
        fclose(datafile);
        fclose(datafile1);
        fprintf(plotpipe, "set terminal qt size 1600,1000 font \"Helvetica, 14\"\n\
                        set key outside\n\
                        set grid\n\
                        show grid\n\
                        set xlabel \"%s - %s\"\n\
                        show xlabel\n\
                        set ylabel \"%s - %s\"\n\
                        show ylabel\n\
                        plot \"%s\" using 1:2,\
                        \"%s\" using 1:2\n",
                data.iter_vars[get_prop_id(args[1], ctr, data, ITER_VARS)].short_name,
                data.iter_vars[get_prop_id(args[1], ctr, data, ITER_VARS)].long_name,
                data.iter_vars[get_prop_id(args[2], ctr, data, ITER_VARS)].short_name,
                data.iter_vars[get_prop_id(args[2], ctr, data, ITER_VARS)].long_name,
                PLOT_DATAFILE_NAME, PLOT_DATAFILE_NAME2);
        pclose(plotpipe);
        return 0;
    }
    int src;
    if (strcmp(args[3], "iter") == 0)
        src = ITER_VARS;
    else if (strcmp(args[3], "ref") == 0)
        src = REFSOL_VARS;
    else
    {
        fclose(datafile);
        pclose(plotpipe);
        return -1;
    }
    if (print_data_relation(datafile, args[1], args[2], ctr, data, *parameters, src) < 0)
    {
        fclose(datafile);
        pclose(plotpipe);
        return -3;
    }
    fclose(datafile);
    if (plotpipe == NULL)
    {
        puts(COMMENT_GNUPLOT_ERROR);
        pclose(plotpipe);
        return -4;
    }
    fprintf(plotpipe, "set terminal qt size 1600,1000 font \"Helvetica, 14\"\n\
                        set key outside\n\
                        set grid\n\
                        show grid\n\
                        set xlabel \"%s - %s\"\n\
                        show xlabel\n\
                        set ylabel \"%s - %s\"\n\
                        show ylabel\n\
                        plot \"%s\" using 1:2 \n",
            data.iter_vars[get_prop_id(args[1], ctr, data, src)].short_name,
            data.iter_vars[get_prop_id(args[1], ctr, data, src)].long_name,
            data.iter_vars[get_prop_id(args[2], ctr, data, src)].short_name,
            data.iter_vars[get_prop_id(args[2], ctr, data, src)].long_name,
            PLOT_DATAFILE_NAME);
    pclose(plotpipe);
    return 0;
}

int IO_res(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr,
            params *parameters, local_data data)
{
    int iterno = get_int_param("iterno", ctr, *parameters);
    if (iterno <= RES_MOD)
    {
        puts(COMMENT_NO_RESIDUUM);
        return -3;
    }
    FILE *plotpipe = popen("gnuplot -persist", "w");
    if (plotpipe == NULL)
    {
        puts(COMMENT_GNUPLOT_ERROR);
        pclose(plotpipe);
        return -1;
    }
    fprintf(plotpipe, "set terminal qt size 1600,1000 font \"Helvetica, 14\"\n\
                        set key outside\n\
                        set logscale y\n\
                        set grid\n\
                        show grid\n\
                        set xlabel \"%s\"\n\
                        show xlabel\n\
                        set ylabel \"%s\"\n\
                        show ylabel\n\
                        plot \"%s\" using 1:3 \n",
            DESC_ITERNO,
            DESC_MAX_RES,
            RESIDUUM_FILE_NAME);
    pclose(plotpipe);
    plotpipe = popen("gnuplot -persist", "w");
    fprintf(plotpipe, "set terminal qt size 1600,1000 font \"Helvetica, 14\"\n\
                        set key outside\n\
                        set logscale y\n\
                        set grid\n\
                        show grid\n\
                        set xlabel \"%s\"\n\
                        show xlabel\n\
                        set ylabel \"%s\"\n\
                        show ylabel\n\
                        plot \"%s\" using 1:4 \n",
            DESC_ITERNO,
            DESC_AVG_RES,
            RESIDUUM_FILE_NAME);
    pclose(plotpipe);
    return 0;
}

int user_input_all_params(int *ctr, double_param *double_params, int_param *int_params)
{
    /*remind the user that they have to choose the units responsibly*/
    puts(COMMENT_REMIND_UNITS);
    char user_input[STDIN_BUFFER];
    double double_input;
    int int_input, scanned_vals;
    for (int j = 0; j < ctr[DOUBLE_PARAM_CTR]; j++)
    {
        if (double_params[j].type != NIO)
        {
            while (1)
            {
                printf("%s: ", double_params[j].desc);
                fgets(user_input, STDIN_BUFFER, stdin);
                scanned_vals = sscanf(user_input, "%lf %s", &double_input, user_input);
                if (scanned_vals == 2)
                {
                    puts(COMMENT_SUS_INPUT);
                }
                else if (scanned_vals == 1)
                {
                    if (double_input > 0)
                    {
                        double_params[j].value = double_input;
                        break;
                    }
                    else
                        puts(COMMENT_ENFORCE_POSITIVE_INPUT);
                }
                else
                    puts(COMMENT_GENERIC_INPUT_ERROR);
            }
        }
    }
    for (int k = 0; k < ctr[INT_PARAM_CTR]; k++)
    {
        if (int_params[k].type != NIO)
        {
            while (1)
            {
                printf("%s: ", int_params[k].desc);
                fgets(user_input, STDIN_BUFFER, stdin);
                scanned_vals = sscanf(user_input, "%lf %s", &double_input, user_input);
                if (scanned_vals == 2)
                    puts(COMMENT_SUS_INPUT);
                else if (scanned_vals == 1)
                {
                    int_input = (int)double_input;
                    if (int_input == double_input)
                    {
                        if (int_input > 0)
                        {
                            int_params[k].value = int_input;
                            break;
                        }
                        else
                            puts(COMMENT_ENFORCE_POSITIVE_INPUT);
                    }
                    else
                        puts(COMMENT_ENFORCE_INTEGER_INPUT);
                }
                else
                    puts(COMMENT_ENFORCE_POSITIVE_INPUT);
            }
        }
    }

    return 0;
}

void initialize_params_from_file(int *ctr, params *parameters, local_data data)
{
    double_param *double_params = parameters->doubles;
    int_param *int_params = parameters->ints;
    FILE *params_file = fopen(PARAM_FILE_NAME, "r");
    char input[STDIN_BUFFER];
    if (params_file == NULL)
    {
        puts(COMMENT_FILE_NONEXISTENT);
        while (1)
        {
            fgets(input, STDIN_BUFFER, stdin);
            sscanf(input, "%s", input);
            if (strcmp(input, "y") == 0)
            {
                puts(COMMENT_FILE_CREATED);
                user_input_all_params(ctr, parameters->doubles, parameters->ints);
                save_all_params(ctr, double_params, int_params);
                init_nozzle(ctr, *parameters, data);
                save_all_params(ctr, double_params, int_params);
                refsol_func(ctr, data, parameters);
                break;
            }
            else if (strcmp(input, "n") == 0)
            {
                puts(COMMENT_FILE_EXIT);
                exit(-1);
            }
            else
                puts(COMMENT_BOOL_INPUT_ERROR);
        }
    }
    else
    {
        fclose(params_file);
        puts(COMMENT_FILE_FOUND);
        int init_success = 0;
        while (!init_success)
        {
            fgets(input, STDIN_BUFFER, stdin);
            sscanf(input, "%s", input);
            if (strcmp(input, "y") == 0)
            {
                int read_success = read_all_params(ctr, double_params, int_params);
                if (read_success == 0)
                {
                    puts(COMMENT_FILE_LOAD_SUCCESS);
                    init_nozzle(ctr, *parameters, data);
                    refsol_func(ctr, data, parameters);
                    set_int_param(0, "iterno", ctr, *parameters);
                    --init_success;
                } /* ukonci cyklus pri vytvoreni/nacteni souboru */
                else
                {
                    puts(COMMENT_FILE_LOAD_FAILURE);
                    while (!init_success)
                    {
                        fgets(input, STDIN_BUFFER, stdin);
                        sscanf(input, "%s", input);
                        if (strcmp(input, "y") == 0)
                        {
                            puts(COMMENT_FILE_CREATED);
                            user_input_all_params(ctr, parameters->doubles, parameters->ints);
                            save_all_params(ctr, double_params, int_params);
                            init_nozzle(ctr, *parameters, data);
                            save_all_params(ctr, double_params, int_params);
                            refsol_func(ctr, data, parameters);
                            --init_success;
                        }
                        else if (strcmp(input, "n") == 0)
                        {
                            puts(COMMENT_FILE_EXIT);
                            exit(-1);
                        }
                        else
                            puts(COMMENT_BOOL_INPUT_ERROR);
                    }
                }
            }
            else if (strcmp(input, "n") == 0)
            {
                puts(COMMENT_FILE_CREATED);
                user_input_all_params(ctr, parameters->doubles, parameters->ints);
                save_all_params(ctr, double_params, int_params);
                init_nozzle(ctr, *parameters, data);
                save_all_params(ctr, double_params, int_params);
                refsol_func(ctr, data, parameters);
                --init_success;
            }
            else
            {
                puts(COMMENT_BOOL_INPUT_ERROR);
            }
        }
    }
}

int get_user_input(int *ctr,
                   params *parameters, local_data data, func *funcs)
{
    int exit_request = 0;
    int return_value;
    // char *cline_history;
    while (!exit_request)
    {
        char scanned_input[STDIN_BUFFER];
        char args[ARGS_COUNT][STDIN_BUFFER];

        int fnc_found = 0;
        /* replace strings from previous reading with empty ones*/
        for (int i = 0; i < ARGS_COUNT; i++)
            args[i][0] = '\0';

        puts(COMMENT_REQUEST_INPUT);
        fgets(scanned_input, STDIN_BUFFER, stdin);
        putchar('\n');

        /* remove the newline character from the end of input */
        scanned_input[strcspn(scanned_input, "\n")] = '\0';

        char *arg = strtok(scanned_input, " ");
        int j = 0;
        while (j < ARGS_COUNT && arg != NULL)
        {
            strcpy(args[j++], arg);
            arg = strtok(NULL, " ");
        }
        if (strcmp(args[0], "exit") == 0)
        {
            exit_request++;
            continue;
        }
        for (int i = 0; i < ctr[FUNC_CTR]; i++)
        {
            if (strcmp(args[0], funcs[i].name) == 0)
            {
                return_value = funcs[i].function(args, ctr, parameters, data);
                fnc_found++;
            }
        }
        if(fnc_found)
        {
            if(return_value == 0) 
                puts("\nOK\n");
            else
                printf("%s (%d)\n", COMMENT_ERROR_RETURN, return_value); 
        }
        else
        {
            puts(COMMENT_FNC_NOT_FOUND);
        }
#if 0
        else if (strcmp(args[0], "params") == 0)
            IO_params(args, ctr, parameters, data);
        else if (strcmp(args[0], "execute") == 0)
            /* IO_execute */ puts("TBA");
        else if (strcmp(args[0], "hist") == 0)
            /* IO_hist(cline_history); */ puts("TBA");
#endif
    }
    return 0;
}

int IO_iter(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr,
              params *parameters, local_data data)
{
    int iter_count;
    double double_input;
    if(args[1][0] == '\0')
    {
        puts(COMMENT_MISSING_ITER_COUNT);
        return -3;
    }
    if(sscanf(args[1], "%lf", &double_input) == 0)
    {
        return -1;
    }
    if((iter_count = (int) double_input) != double_input)
    {
        return -2;
    }
    return iter_func(iter_count, ctr, *parameters, data);
}

int IO_reset(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr,
              params *parameters, local_data data)
{
    if(args[1][0] != '\0') return -1;
    return init_nozzle(ctr, *parameters, data);
}

/* from command line, change the value of param_name to param_value */
int set_param_value(char *param_name, char *param_value, int *ctr, params parameters,
local_data data)
{
    double_param *double_params = parameters.doubles;
    int_param *int_params = parameters.ints;
    const int param_id = get_param_id(param_name, ctr, double_params, int_params);
    const int param_count = ctr[DOUBLE_PARAM_CTR] + ctr[INT_PARAM_CTR];

    const int iterno = get_int_param("iterno", ctr, parameters);

    if (param_id < 0 || param_id >= param_count)
        return -1;
    if (param_id < ctr[DOUBLE_PARAM_CTR])
    {
        double double_input;
        int scan_success = sscanf(param_value, "%lf", &double_input);
        if (scan_success)
        {
            if (double_params[param_id].type == NIO)
            {
                printf("%s %s\n", COMMENT_ILLEGAL_PARAM_ACCESS,
                    double_params[param_id].name);
                return -2;
            }
            int read_success = 0;
            char input[STDIN_BUFFER];
            if (double_params[param_id].type == IO_CNA)
            {
                if(iterno > 0)
                {
                    puts(COMMENT_CONFIRM_RESET);
                    while (!read_success)
                    {
                        fgets(input, STDIN_BUFFER, stdin);
                        sscanf(input, "%s", input);
                        if (strcmp(input, "y") == 0)
                        {
                            double_params[param_id].value = double_input;
                            --read_success;
                            continue;
                        }
                        if (strcmp(input, "n") == 0) return 0;
                        puts(COMMENT_BOOL_INPUT_ERROR);
                    }
                }
                else
                {
                    double_params[param_id].value = double_input;
                }
            }
            else if (double_params[param_id].type = IO_CA)
            {
                if(iterno > 0)
                {
                    puts(COMMENT_REQUEST_RESET);
                    double_params[param_id].value = double_input;
                    while (!read_success)
                    {
                        fgets(input, STDIN_BUFFER, stdin);
                        sscanf(input, "%s", input);
                        if (strcmp(input, "y") == 0)
                        {
                            --read_success;
                            continue;
                        }
                        if (strcmp(input, "n") == 0) return 0;
                        puts(COMMENT_BOOL_INPUT_ERROR);
                    }
                }
                else
                {
                    double_params[param_id].value = double_input;
                }
            }
        }
        else
            return -1;
    }
    else
    {
        int int_input;
        int scan_success = sscanf(param_value, "%d", &int_input);
        if (scan_success)
        {
            if(int_params[INT_PARAM_ID].type == NIO)
            {
                printf("%s %s\n", COMMENT_ILLEGAL_PARAM_ACCESS,
                    int_params[INT_PARAM_ID].name);
                return -2;
            }
            int read_success = 0;
            char input[STDIN_BUFFER];
            if (int_params[INT_PARAM_ID].type == IO_CNA)
            {
                if(iterno > 0)
                {
                    puts(COMMENT_CONFIRM_RESET);
                    while (!read_success)
                    {
                        fgets(input, STDIN_BUFFER, stdin);
                        sscanf(input, "%s", input);
                        if (strcmp(input, "y") == 0)
                        {
                            int_params[INT_PARAM_ID].value = int_input;
                            --read_success;
                            continue;
                        }
                        if (strcmp(input, "n") == 0) return 0;
                        puts(COMMENT_BOOL_INPUT_ERROR);
                    }
                }
                else
                {
                    int_params[INT_PARAM_ID].value = int_input;
                }
            }
            else if (int_params[INT_PARAM_ID].type = IO_CA)
            {
                if(iterno > 0)
                {
                    puts(COMMENT_REQUEST_RESET);
                    int_params[INT_PARAM_ID].value = int_input;
                    while (!read_success)
                    {
                        fgets(input, STDIN_BUFFER, stdin);
                        sscanf(input, "%s", input);
                        if (strcmp(input, "y") == 0)
                        {
                            --read_success;
                            continue;
                        }
                        if (strcmp(input, "n") == 0) return 0;
                        puts(COMMENT_BOOL_INPUT_ERROR);
                    }
                }
                else
                {
                    int_params[INT_PARAM_ID].value = int_input;
                }
            }
        }
        else
            return -1;
    }

    save_all_params(ctr, double_params, int_params);
    init_nozzle(ctr, parameters, data);
    iter_func(iterno, ctr, parameters, data);
    return 0;
}

int IO_params(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr, params *parameters,
    local_data data)
{
    double_param *double_params = parameters->doubles;
    int_param *int_params = parameters->ints;
    if (args[1][0] == '\0')
    {
        save_all_params(ctr, double_params, int_params);
        print_all_params(stdout, ctr, double_params, int_params);
        return 0;
    }
    if (strcmp(args[1], "backup") == 0)
    {
        if (args[2][0] == '\0')
        {
            puts(COMMENT_MISSING_FILE_NAME);
            return -1;
        }
        else
        {
            FILE *backup_file = fopen(args[2], "w");
            if (backup_file == NULL)
            {
                printf("%s %s\n", COMMENT_FOPEN_ERROR, args[2]);
                fclose(backup_file);
                return -1;
            }
            else
            {
                save_all_params(ctr, double_params, int_params);
                print_all_params(backup_file, ctr, double_params, int_params);
            }
            fclose(backup_file);
            return 0;
        }
    }
    if (strcmp(args[1], "save") == 0)
    {
        if (args[2][0] != '\0')
        {
            printf("%s %s %s\n", COMMENT_EXTRA_ARGS, args[0], args[1]);
        }
        save_all_params(ctr, double_params, int_params);
        return 0;
    }
    if (strcmp(args[1], "view") == 0)
    {
        if (args[2][0] != '\0')
        {
            printf("%s %s %s\n", COMMENT_EXTRA_ARGS, args[0], args[1]);
        }
        print_all_params(stdout, ctr, double_params, int_params);
        return 0;
    }
    if (strcmp(args[1], "edit") == 0)
    {
        if (strcmp(args[2], "all") == 0)
        {
            user_input_all_params(ctr, double_params, int_params);
            return 0;
        }
        else if (args[3][0] != '\0')
            return set_param_value(args[2], args[3], ctr,
                                   *parameters, data);
        else
            puts(COMMENT_MISSING_FILE_NAME);
        return -1;
    }
}

int IO_help(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr, params *parameters,
    local_data data)
{
    puts(HELP_STRING);
    return 0;
}