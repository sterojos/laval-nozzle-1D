#define PARAM_FILE_NAME "data_parameters.txt"
#define PLOT_DATAFILE_NAME "data_plot.txt"
#define PLOT_DATAFILE_NAME2 "data_plot0.txt"
#define RESIDUUM_FILE_NAME "data_res.txt"

/*used to allocate memory for function/parameter description */
#define NAME_MAX_LEN 8
#define DESC_MAX_LEN 50

/*length of variable description */
#define VAR_LONG_LEN 64

/*precision of floating-point parameters saved into the files */
#define FILE_DECIMALS 15

/*precision of local values (they are not re-used) */
#define FILE_DEC_SHORT 4

/*characters from user input beyond this length shall be ignored*/
#define STDIN_BUFFER 128

/*max length of user input history*/
#define HIST_MAX_LEN 0x1000000

/*max number of arguments passed to functions (function name included)*/
#define ARGS_COUNT 5

/*max number of loaded parameters/functions, respectively*/
#define REF_PARAM_COUNT 100
#define REF_FUNC_COUNT 50
/* max number of variables saved during the simulation */
#define REF_VARS_COUNT 50

/*value used as an "undefined/error" parameter value */
#define PARAM_NDEF -1

/*
The array ctr[] defined in main() stores various integers that are significant,
but don't quite make it to the parameters array.

Most of them are sizes of various arrays for use in loops, etc.

One notable exception is RSOL_N_CTR.
It is the grid size used by the reference solver - the higher this number,
the smoother the graph resulting from the data.

It will be initialized to the value of RSOL_N. This value is also used as the
default size of grid malloc'd for numerical methods.
*/
#define DOUBLE_PARAM_CTR 0
#define INT_PARAM_CTR 1
#define FUNC_CTR 2
#define LOC_VAR_CTR 3
#define RSOL_N_CTR 4
#define FORM_CTR 5

#define CTR_ARRAY_LEN 6

#define RSOL_N 400
#define NMAX 20000

#define ROW_LEN (NAME_MAX_LEN + FILE_DECIMALS + DESC_MAX_LEN + 11)

#define DEF_PARAM_STRUCT(struct_name, data_type) \
    typedef struct                               \
    {                                            \
        int type;                                \
        char name[NAME_MAX_LEN];                 \
        char desc[DESC_MAX_LEN];                 \
        data_type value;                         \
    } struct_name

/* the param type informs functions how the param should be handled.
    IO - the parameter should be requested from the user
    NIO - the parameter should not be requested from the user (is internal in some way),
            and also cannot be edited by the user
    CA - when this parameter is changed, ask if the calculation should be reset to
        initial condition (it's not necessary in all cases, but it's possible that
        things will break if the reset does not occur)
    CNA - when this parameter is changed, force reset (previous results don't make sense)
*/
#define NIO 0
#define IO_CA 1
#define IO_CNA 2

#define THROW_BUFFER_ERR(desc, macro)              \
    puts(COMMENT_BUFFER_ERR);                      \
    puts(COMMENT_SUGGEST_BUFFER_FIX(desc, macro)); \
    exit(-1)

#define INIT_PARAM(param_name, desc, param_type, struct_name, id_ref, dest_array) \
    if (strlen(param_name) > NAME_MAX_LEN)                                        \
    {                                                                             \
        THROW_BUFFER_ERR(COMMENT_PARAM_NAMES, NAME_MAX_LEN);                      \
    }                                                                             \
    if (strlen(desc) > DESC_MAX_LEN)                                              \
    {                                                                             \
        THROW_BUFFER_ERR(COMMENT_PARAM_DESC, DESC_MAX_LEN);                       \
    }                                                                             \
    if (id_ref >= REF_PARAM_COUNT)                                                \
    {                                                                             \
        THROW_BUFFER_ERR(COMMENT_PARAM_COUNT, REF_PARAM_COUNT);                   \
    }                                                                             \
    dest_array[id_ref++] = (struct_name) { param_type, param_name, desc, PARAM_NDEF }
/*set params to PARAM_NDEF by default to check proper initialization*/

#define INIT_DOUBLE_PARAM(param_name, desc, param_type) \
    INIT_PARAM(param_name, desc, param_type, double_param, ctr[DOUBLE_PARAM_CTR], double_params)

#define INIT_INT_PARAM(param_name, desc, param_type) \
    INIT_PARAM(param_name, desc, param_type, int_param, ctr[INT_PARAM_CTR], int_params)

#define INIT_VARIABLE(handle, short_desc, form_info)         \
    if (strlen(handle) > NAME_MAX_LEN)                       \
    {                                                        \
        THROW_BUFFER_ERR(COMMENT_VAR_HANDLES, NAME_MAX_LEN); \
    }                                                        \
    if (strlen(short_desc) > VAR_LONG_LEN)                   \
    {                                                        \
        THROW_BUFFER_ERR(COMMENT_VAR_NAMES, VAR_LONG_LEN);   \
    }                                                        \
    local_vars[ctr[LOC_VAR_CTR]++] =                         \
        (local_var) { form_info, handle, short_desc, NULL }

/* form_info carries information about whether the variable must be updated
    after a calculation via a formula.
    NOZZLE_PROP - property of the nozzle, update only when nozzle params are edited
    ITER_PROP - property used during numerical calculations (eg. in conservative eqs.)
    REFSOL_PROP - calculated directly by the reference solver
    GENERIC PROP - requires a formula to update after any calculation
    (assuming there's no overlap between CONSERVATIVE_PROP and REFSOL_PROP)
    */
#define NOZZLE_PROP 0
#define ITER_PROP 1
#define REFSOL_PROP 2
#define GENERIC_PROP 3

#define ITER_VARS 0
#define REFSOL_VARS 1

#define INITIALIZED 1
#define NOT_INITIALIZED 0

#define INIT_FUNC(cmd_name, fnc_handle)                           \
    int fnc_handle(char args[ARGS_COUNT][STDIN_BUFFER], int *ctr, \
                   params *parameters, local_data data);          \
    if (strlen(cmd_name) > NAME_MAX_LEN)                          \
    {                                                             \
        THROW_BUFFER_ERR(COMMENT_FUNC_NAMES, TAG(NAME_MAX_LEN));  \
    }                                                             \
    if (ctr[FUNC_CTR] >= REF_FUNC_COUNT)                          \
    {                                                             \
        THROW_BUFFER_ERR(COMMENT_FUNC_COUNT, REF_FUNC_COUNT);     \
    }                                                             \
    funcs[ctr[FUNC_CTR]++] = (func){cmd_name, &fnc_handle};

#define ACCESS_REFSOL_DATA(name) \
    data.refsol_vars[get_prop_id(name, ctr, data, ITER_VARS)].values
#define ACCESS_ITER_DATA(name) \
    data.iter_vars[get_prop_id(name, ctr, data, ITER_VARS)].values

#define LINEAR_EXTRAPOLATION(closer_value, farther_value, step) \
    closer_value + (closer_value - farther_value) * step

#define OPEN_RES_FILE(ptr_name) \
    FILE *ptr_name = fopen(RESIDUUM_FILE_NAME, "a")

#define ERASE_RES_FILE \
    FILE *res_file = fopen(RESIDUUM_FILE_NAME, "w"); \
    fclose(res_file);

#define INT_PARAM_ID (param_id - ctr[DOUBLE_PARAM_CTR])
