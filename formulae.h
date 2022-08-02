#define P_DYN(P_STAT, MACH, KAPPA) \
    P_STAT *pow(1. + (KAPPA - 1.) / 2. * MACH * MACH, -KAPPA / (KAPPA - 1.))
#define T_DYN(T_STAT, MACH, KAPPA) \
    T_STAT *pow(1. + (KAPPA - 1.) / 2. * MACH * MACH, -1)
#define A_SND_DYN(A_STAT, MACH, KAPPA) \
    A_STAT *pow(1. + (KAPPA - 1.) / 2. * MACH * MACH, -1. / 2.)
#define RHO_DYN(RHO_STAT, MACH, KAPPA) \
    RHO_STAT *pow(1. + (KAPPA - 1.) / 2. * MACH * MACH, -1. / (KAPPA - 1.))

#define QUASI1D_MACH_FORMULA(CONST1, CONST2, EXPONENT, MACH_VAR) \
    CONST1 *MACH_VAR *pow((1 + CONST2 * MACH_VAR * MACH_VAR), EXPONENT)

#define MACH_RED_SHOCK(MACH1, KAPPA)    \
    ((KAPPA - 1) * MACH1 * MACH1 + 2) / \
        (2 * KAPPA * MACH1 * MACH1 - KAPPA + 1)

#define STAT_PRES_RED_SHOCK(MACH1, P_STAT1, KAPPA)       \
    pow(                                                 \
        (KAPPA + 1) * MACH1 * MACH1 /                    \
            ((KAPPA - 1) * MACH1 * MACH1 + 2),           \
        KAPPA / (KAPPA - 1.)) *                          \
        pow(                                             \
            (KAPPA + 1) /                                \
                (2 * KAPPA * MACH1 * MACH1 - KAPPA + 1), \
            1. / (KAPPA - 1)) *                          \
        P_STAT1

#define P2_DYN_SHOCK(MACH1, P_STAT1, KAPPA)                                                                                   \
    (2 * KAPPA * MACH1 * MACH1 - KAPPA + 1) / (KAPPA + 1) * pow(2 / ((KAPPA - 1) * MACH1 * MACH1 + 2), KAPPA / (KAPPA - 1)) * \
        P_STAT1

/*
#define INIT_FORMULA(output, fnc_name)                       \
    if (strlen(output) > NAME_MAX_LEN)                       \
    {                                                        \
        THROW_BUFFER_ERR(COMMENT_VAR_HANDLES, NAME_MAX_LEN); \
    }                                                        \
    if (get_prop_id(output, ctr, data, ITER_VARS) < 0)       \
    {                                                        \
        printf("%s %s\n", COMMENT_VAR_NONEXISTENT, output);  \
    }                                                        \
    formulae[ctr[FORM_CTR]++] = (formula) { output, &fnc_name }

typedef struct
{
    char result[NAME_MAX_LEN];
    double *(*function)(local_var vars);
} formula;
*/