#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COMMENT_REQUEST_NOZZLE_INIT \
    "Chybi definice parametru trysky. Zkus \"params set all\"."

#define N_ACRIT 0
#define N_AMAX 1
#define N_ALPHA 2
#define N_L 3
#define N_RCRIT 4
#define N_RMAX 5
#define N_DX 6
#define N_DX_REFSOL 7
#define N_XCRIT 8
#define NOZZLE_PARAMS_COUNT 9

#define CONSERV_EQS_COUNT 3
#define Q_DENSITY 0
#define Q_MOMENTUM 1
#define Q_ENERGY 2

#define R_RED (noz_desc[N_RMAX] - noz_desc[N_RCRIT])
#define NALPHA noz_desc[N_ALPHA]

#define ARREND (grid_size - 1)

/* The greater the value, the less often will CFL be recalculated. */
#define CFL_MOD 10
/* Below this minimum of iterations, CFL is recalculated every time
due to instability caused by the intentionally stupid initial condition. */
#define CFL_MIN 1000

/* how often is the change in density measured? */
#define RES_MOD 200

/* print a dot after this many iterations*/
#define DOT_MOD 70000

/* intentionally stupid initial condition for numerical methods */
int init_cond(int *ctr, params parameters, local_data data)
{
    if (set_int_param(0, "iterno", ctr, parameters) < 0)
        return -1;

    double p0 = get_double_param("p0", ctr, parameters);
    double T0 = get_double_param("T0", ctr, parameters);
    double p1 = get_double_param("p1", ctr, parameters);
    int n = get_int_param("n", ctr, parameters);

    double l = get_double_param("l", ctr, parameters);
    double dx = l / (n - 1);

    double R = get_double_param("r_gas", ctr, parameters);
    double kappa = get_double_param("kappa", ctr, parameters);
    double c_v = R / (kappa - 1);
    double a_def = sqrt(kappa * R * T0);

    double *T = ACCESS_ITER_DATA("T");
    double *p = ACCESS_ITER_DATA("p");
    double *P = ACCESS_ITER_DATA("P");
    double *rho = ACCESS_ITER_DATA("rho");
    double *c = ACCESS_ITER_DATA("c");
    double *a = ACCESS_ITER_DATA("a");
    double *Ma = ACCESS_ITER_DATA("Ma");
    double *s = ACCESS_ITER_DATA("s");
    double *m_flux = ACCESS_ITER_DATA("m_flux");
    double *e = ACCESS_ITER_DATA("e");
    double *h = ACCESS_ITER_DATA("h");
    double *h_t = ACCESS_ITER_DATA("h_t");
    for (int i = 0; i < n; i++)
    {
        T[i] = T0;
        p[i] = p0 + (p1 - p0) * i / (n - 1);
        rho[i] = p[i] / (R * T[i]);
        a[i] = a_def;
        c[i] = Ma[i] = s[i] = P[i] = m_flux[i] = 0;
        e[i] = c_v * T[i];
        h[i] = h_t[i] = e[i] + p[i] / rho[i];
    }
    data.initialized[ITER_VARS] = INITIALIZED;

    return 0;
}

/* initialize nozzle shape, ask for confirmation */
int init_nozzle(int *ctr, params parameters, local_data data)
{
    double *noz_desc = (double *)calloc(NOZZLE_PARAMS_COUNT, sizeof(double));

    /* realloc arrays to proper size */
    int grid_size = get_int_param("n", ctr, parameters);
    if (grid_size < 2)
        return -3;

    for (int i = 0; i < ctr[LOC_VAR_CTR]; i++)
    {
        data.iter_vars[i].values =
            (double *)realloc(data.iter_vars[i].values, grid_size * sizeof(double));
    }

    noz_desc[N_ACRIT] = get_double_param("A_crit", ctr, parameters);
    noz_desc[N_AMAX] = get_double_param("A_max", ctr, parameters);
    NALPHA = get_double_param("alpha", ctr, parameters) * M_PI / 180;
    for (int i = 0; i < NOZZLE_PARAMS_COUNT; i++)
    {
        if (noz_desc[i] < 0)
        {
            puts(COMMENT_REQUEST_NOZZLE_INIT);
            free(noz_desc);
            return -1;
        }
    }
    noz_desc[N_RCRIT] = sqrt(noz_desc[N_ACRIT] / M_PI);
    noz_desc[N_RMAX] = sqrt(noz_desc[N_AMAX] / M_PI);
    noz_desc[N_L] = (R_RED) *
                    (0.5 * tan(NALPHA) + 1. + (1 - cos(NALPHA)) / cos(NALPHA)) / tan(NALPHA);
    noz_desc[N_XCRIT] = 0.5 * (R_RED);

    if (set_double_param(noz_desc[N_RCRIT], "r_crit", ctr, parameters) == -1)
        return -1;
    if (set_double_param(noz_desc[N_RMAX], "r_max", ctr, parameters) == -1)
        return -1;
    if (set_double_param(noz_desc[N_L], "l", ctr, parameters) == -1)
        return -1;
    if (set_double_param(noz_desc[N_XCRIT], "x_crit", ctr, parameters) == -1)
        return -1;

    int A_id, x_id, r_id;
    if ((A_id = get_prop_id("A", ctr, data, REFSOL_VARS)) != get_prop_id("A", ctr, data, ITER_VARS))
    {
        puts(COMMENT_NOZZLE_PROP_ERROR);
        free(noz_desc);
        return -2;
    }
    if ((x_id = get_prop_id("x", ctr, data, REFSOL_VARS)) != get_prop_id("x", ctr, data, ITER_VARS))
    {
        puts(COMMENT_NOZZLE_PROP_ERROR);
        free(noz_desc);
        return -2;
    }
    if ((r_id = get_prop_id("r", ctr, data, REFSOL_VARS)) != get_prop_id("r", ctr, data, ITER_VARS))
    {
        puts(COMMENT_NOZZLE_PROP_ERROR);
        free(noz_desc);
        return -2;
    }

    noz_desc[N_DX_REFSOL] = noz_desc[N_L] / (ctr[RSOL_N_CTR] - 1);
    int n_crit_refsol = floor(noz_desc[N_XCRIT] / noz_desc[N_DX_REFSOL]);
    set_int_param(n_crit_refsol, "nc_r", ctr, parameters);

    int n_st_refsol = floor((noz_desc[N_XCRIT] + R_RED * sin(NALPHA)) / noz_desc[N_DX_REFSOL]);

    noz_desc[N_DX] = noz_desc[N_L] / (ARREND);
    int n_crit_iter = floor(noz_desc[N_XCRIT] / noz_desc[N_DX]);
    set_int_param(n_crit_iter, "nc_i", ctr, parameters);

    int n_st_iter = floor((noz_desc[N_XCRIT] + R_RED * sin(NALPHA)) / noz_desc[N_DX]);

    double xm;
    local_var *x = &data.refsol_vars[x_id];
    local_var *A = &data.refsol_vars[A_id];
    local_var *r = &data.refsol_vars[r_id];

    for (int i = 0; i < n_st_refsol; i++)
    {
        x->values[i] = noz_desc[N_DX_REFSOL] * i;
        xm = noz_desc[N_DX_REFSOL] * i + 0.5 * (R_RED);
        r->values[i] =
            noz_desc[N_RMAX] - sqrt(2 * xm * (R_RED)-xm * xm);
        A->values[i] = M_PI * r->values[i] * r->values[i];
    }
    for (int i = n_st_refsol; i < ctr[RSOL_N_CTR]; i++)
    {
        x->values[i] = noz_desc[N_DX_REFSOL] * i;
        xm = noz_desc[N_DX_REFSOL] * i + 0.5 * (R_RED);
        r->values[i] = noz_desc[N_RCRIT] +
                       tan(NALPHA) * (xm - noz_desc[N_RMAX] + noz_desc[N_RCRIT])- R_RED * (1 - cos(NALPHA)) / cos(NALPHA);
        A->values[i] = M_PI * r->values[i] * r->values[i];
    }

    /* estimate local area derivative with respect to distance
    for right-hand terms */
    double *A_x = ACCESS_REFSOL_DATA("A_x");
    for (int i = 1; i < (ctr[RSOL_N_CTR] - 1); i++)
        A_x[i] = (A->values[i + 1] - A->values[i - 1]) / (2 * noz_desc[N_DX_REFSOL]);
    A_x[0] = LINEAR_EXTRAPOLATION(A_x[1], A_x[2], noz_desc[N_DX_REFSOL]);
    A_x[ctr[RSOL_N_CTR] - 1] =
        LINEAR_EXTRAPOLATION(A_x[ctr[RSOL_N_CTR] - 2], A_x[ctr[RSOL_N_CTR] - 3], noz_desc[N_DX_REFSOL]);

    /* do everything again for the other data set */
    x = &data.iter_vars[x_id];
    A = &data.iter_vars[A_id];
    r = &data.iter_vars[r_id];

    for (int i = 0; i < n_st_iter; i++)
    {
        x->values[i] = noz_desc[N_DX] * i;
        xm = noz_desc[N_DX] * i + 0.5 * (R_RED);
        r->values[i] =
            noz_desc[N_RMAX] - sqrt(2 * xm * (R_RED)-xm * xm);
        A->values[i] = M_PI * r->values[i] * r->values[i];
    }
    for (int i = n_st_iter; i < grid_size; i++)
    {
        x->values[i] = noz_desc[N_DX] * i;
        xm = noz_desc[N_DX] * i + 0.5 * (R_RED);
        r->values[i] = noz_desc[N_RCRIT] +
                       tan(NALPHA) * (xm - noz_desc[N_RMAX] + noz_desc[N_RCRIT]) - R_RED * (1 - cos(NALPHA)) / cos(NALPHA);
        A->values[i] = M_PI * r->values[i] * r->values[i];
    }

    A_x = ACCESS_ITER_DATA("A_x");
    for (int i = 1; i < (ARREND); i++)
        A_x[i] = (A->values[i + 1] - A->values[i - 1]) / (2 * noz_desc[N_DX]);
    A_x[0] = LINEAR_EXTRAPOLATION(A_x[1], A_x[2], noz_desc[N_DX]);
    A_x[ARREND] =
        LINEAR_EXTRAPOLATION(A_x[grid_size - 2], A_x[grid_size - 3], noz_desc[N_DX]);

    free(noz_desc);
    ERASE_RES_FILE;
    return init_cond(ctr, parameters, data);
}

int iter_func(int iter_count, int *ctr, params parameters, local_data data)
{
    double *rho = ACCESS_ITER_DATA("rho");
    double *A = ACCESS_ITER_DATA("A");
    double *A_x = ACCESS_ITER_DATA("A_x");
    double *e = ACCESS_ITER_DATA("e");
    double *c = ACCESS_ITER_DATA("c");
    double *p = ACCESS_ITER_DATA("p");
    double *a = ACCESS_ITER_DATA("a");

    int grid_size = get_int_param("n", ctr, parameters);

    /* quasi-1d conservative variables */
    double *cv_q1d[CONSERV_EQS_COUNT];
    /* fluxes of quasi-1d conservative variables */
    double *flx_q1d[CONSERV_EQS_COUNT];
    /* make copies of conservative variables in order to be able to
        preserve the previous grid layer */
    double *cv_q1d_next[CONSERV_EQS_COUNT];
    double *flx_q1d_next[CONSERV_EQS_COUNT];

    /* initialize values of quasi-1d variables */
    for (int i = 0; i < CONSERV_EQS_COUNT; i++)
    {
        cv_q1d[i] = calloc(grid_size, sizeof(double));
        flx_q1d[i] = calloc(grid_size, sizeof(double));
        cv_q1d_next[i] = calloc(grid_size, sizeof(double));
        flx_q1d_next[i] = calloc(grid_size, sizeof(double));
    }
    for (int i = 0; i < grid_size; i++)
    {
        cv_q1d[Q_DENSITY][i] = A[i] * rho[i];
        cv_q1d[Q_MOMENTUM][i] = A[i] * rho[i] * c[i];
        cv_q1d[Q_ENERGY][i] = A[i] * rho[i] * e[i];

        flx_q1d[Q_DENSITY][i] = cv_q1d[Q_MOMENTUM][i];
        flx_q1d[Q_MOMENTUM][i] = A[i] * (rho[i] * c[i] * c[i] + p[i]);
        flx_q1d[Q_ENERGY][i] = A[i] * c[i] * (rho[i] * e[i] + p[i]);
    }
    for (int i = 0; i < CONSERV_EQS_COUNT; i++)
    {
        memcpy(cv_q1d_next[i], cv_q1d[i], grid_size * sizeof(double));
        memcpy(flx_q1d_next[i], flx_q1d[i], grid_size * sizeof(double));
    }

    /* aliases for pointers */
    double *rho_u = cv_q1d_next[Q_DENSITY];
    double *P_u = cv_q1d_next[Q_MOMENTUM];
    double *e_u = cv_q1d_next[Q_ENERGY];

    double *rho_f = flx_q1d_next[Q_DENSITY];
    double *P_f = flx_q1d_next[Q_MOMENTUM];
    double *e_f = flx_q1d_next[Q_ENERGY];

    /* make internal copies of all variables edited during iteration,
    so that the originals are not destroyed through divergent behavior */
    double *p_int = (double *)calloc(grid_size, sizeof(double));
    double *a_int = (double *)calloc(grid_size, sizeof(double));
    double *c_int = (double *)calloc(grid_size, sizeof(double));

    memcpy(p_int, p, grid_size * sizeof(double));
    memcpy(a_int, a, grid_size * sizeof(double));
    memcpy(c_int, c, grid_size * sizeof(double));

    /* acquire constant params */
    double len = get_double_param("l", ctr, parameters);
    double dx = len / ARREND;
    double kappa = get_double_param("kappa", ctr, parameters);
    double r = get_double_param("r_gas", ctr, parameters);
    /* max value of cfl as selected by the user */
    double cfl = get_double_param("cfl", ctr, parameters);
    /* eps as selected by the user */
    double eps = get_double_param("eps", ctr, parameters);

    double p0 = get_double_param("p0", ctr, parameters);
    double T0 = get_double_param("T0", ctr, parameters);

    double p1 = get_double_param("p1", ctr, parameters);

    /* dt is recalculated based on the CFL condition */
    double dt;

    double x_shock = get_double_param("x_shock", ctr, parameters);
    /* initial iteration count, final iteration count if successful */
    int iterno = get_int_param("iterno", ctr, parameters);
    int iterno_final = iterno + iter_count;

    OPEN_RES_FILE(res_file);
    if(iterno == 0)
    {
        fprintf(res_file, "%-12s %-12s %-12s %-12s\n",
        "i", "max_pos", "res_max", "res_avg");
    }

    /*start iterating */
    for (int i = iterno; i < iterno_final; i++)
    {
        if (i < CFL_MIN || i % CFL_MOD == 0)
        {
            double lambda_max = 0;
            for (int k = 0; k < grid_size; k++)
            {
                double lambda_1 = fabs(c_int[k]);
                double lambda_2 = fabs(c_int[k] + a_int[k]);
                double lambda_3 = fabs(c_int[k] - a_int[k]);
                double lambda_max_local = fmax(lambda_1, lambda_2);
                lambda_max_local = fmax(lambda_max_local, lambda_3);
                if (lambda_max_local > lambda_max)
                    lambda_max = lambda_max_local;
            }
            /* update dt based on maximum wave velocity */
            dt = cfl * dx / lambda_max;
        }

        /* c heck if output hasn't diverged */

        /* extrapolate to the boundaries */
        for (int j = 0; j < CONSERV_EQS_COUNT; j++)
        {
            cv_q1d_next[j][0] = LINEAR_EXTRAPOLATION(cv_q1d_next[j][1], cv_q1d_next[j][2], dx); 
            cv_q1d_next[j][ARREND] =
                LINEAR_EXTRAPOLATION(cv_q1d_next[j][ARREND - 1], cv_q1d_next[j][ARREND - 2], dx);
        }

        /* update fluxes based on new values */
        for (int k = 0; k < grid_size; k++)
        {
            rho_f[k] = P_u[k];
            P_f[k] = P_u[k] * c_int[k] + A[k] * p_int[k];
            e_f[k] = c_int[k] * (e_u[k] + A[k] * p_int[k]);
        }

        /* use the numerical method */
        for (int j = 0; j < CONSERV_EQS_COUNT; j++)
        {
            for (int k = 1; k < ARREND; k++)
            {
                cv_q1d_next[j][k] =
                    cv_q1d[j][k] +
                    dt / (2. * dx) * (flx_q1d[j][k - 1] - flx_q1d[j][k + 1]) +
                    eps / 2. * (cv_q1d[j][k - 1] - 2 * cv_q1d[j][k] + cv_q1d[j][k + 1]);
            }
        }

        /* add right-hand term to momentum equation (the only non-zero right hand)*/
        for (int k = 1; k < ARREND; k++)
        {
            cv_q1d_next[Q_MOMENTUM][k] += dt * A_x[k] * p_int[k];
        }

                /* update variables required for flux + wave speed calculations */
        for (int k = 0; k < grid_size; k++)
        {
            c_int[k] = P_u[k] / rho_u[k];
            p_int[k] = (kappa - 1.) * (e_u[k] - rho_u[k] * c_int[k] * c_int[k] / 2.) / A[k];
            a_int[k] = sqrt(kappa * p_int[k] * A[k] / rho_u[k]);
        }

        /* adjust inlet boundary condition using known parameters at inlet */
        
        c_int[0] = LINEAR_EXTRAPOLATION(c_int[1], c_int[2], dx);

        double Ma_l = c_int[0] / sqrt(kappa * r * T0 - (kappa - 1.) / 2. * c_int[0] * c_int[0]);

        p_int[0] = P_DYN(p0, Ma_l, kappa);

        rho_u[0] = RHO_DYN((p0 / (r * T0)), Ma_l, kappa) * A[0];
        P_u[0] = rho_u[0] * c_int[0];
        e_u[0] = A[0] * p_int[0] / (kappa - 1.) + rho_u[0] * c_int[0] * c_int[0] / 2.;

         /* adjust outlet boundary condition based on known dynamic pressure,
            if flow is subsonic */
        c_int[ARREND] = LINEAR_EXTRAPOLATION(c_int[ARREND - 1], c_int[ARREND - 2], dx);
        double T_A2 = p_int[ARREND - 2] * A[ARREND - 2] / (rho_u[ARREND - 2] * r);
        double T_A1 = p_int[ARREND - 1] * A[ARREND - 1] / (rho_u[ARREND - 1] * r);
        double T = LINEAR_EXTRAPOLATION(T_A1, T_A2, dx);
        double Ma_r = c_int[ARREND] / sqrt(kappa * r * T);
        if(Ma_r <= 1. || i < 50000)
        {
            p_int[ARREND] = p1;
            e_u[ARREND] = A[ARREND] * p1 / (kappa - 1.) + rho_u[ARREND] * c_int[ARREND] * c_int[ARREND] / 2.;
        }

        /* measure residua */
        if(i % RES_MOD == 0 && i != 0)
        {
            double res_max = 0;
            double res_sum = 0;
            int max_position = 0;
            for (int k = 0; k < grid_size; k++)
            {
                double res = fabs(cv_q1d[Q_DENSITY][k] / A[k] - cv_q1d_next[Q_DENSITY][k] / A[k]);
                res_sum += res;
                if(res > res_max)
                {
                    res_max = res;
                    max_position = k;
                }
            }
            double res_avg = res_sum / grid_size;
            fprintf(res_file, "%-12d %-12d %.6E %.6E\n", i, max_position, res_max, res_avg);  

            if(i % DOT_MOD == 0) 
            {
                putchar('.'); 
                putchar('\n');
            }          
        }
        /* swap all pointers to allow for the next layer */
        for (int w = 0; w < CONSERV_EQS_COUNT; w++)
        {
            /* var 1 - for some reason this is not precise*/
            #if 0
            double *swap;

            swap = cv_q1d_next[w];
            cv_q1d_next[w] = cv_q1d[w];
            cv_q1d[w] = swap;

            swap = flx_q1d_next[w];
            flx_q1d_next[w] = flx_q1d[w];
            flx_q1d[w] = swap;
            #endif

            /* var 2 */
            memcpy(cv_q1d[w], cv_q1d_next[w], grid_size * sizeof(double));
            memcpy(flx_q1d[w], flx_q1d_next[w], grid_size * sizeof(double));
        }
    }

    fclose(res_file);

    /* once iterations are finished, update all relevant properties */

    memcpy(p, p_int, grid_size * sizeof(double));
    memcpy(c, c_int, grid_size * sizeof(double));
    memcpy(a, a_int, grid_size * sizeof(double));

    double *T = ACCESS_ITER_DATA("T");
    double *Ma = ACCESS_ITER_DATA("Ma");
    double *h = ACCESS_ITER_DATA("h");
    double *h_t = ACCESS_ITER_DATA("h_t");
    double *m_flux = ACCESS_ITER_DATA("m_flux");
    double *P = ACCESS_ITER_DATA("P");

    double c_v = r / (kappa - 1);
    for (int k = 0; k < grid_size; k++)
    {
        rho[k] = cv_q1d[Q_DENSITY][k] / A[k];
        T[k] = p[k] / (r * rho[k]);
        Ma[k] = c[k] / a[k];
        h[k] = c_v * T[k] + p[k] / rho[k];
        h_t[k] = h[k] + c[k] * c[k] / 2.;
        m_flux[k] = rho[k] * A[k] * c[k];
        P[k] = cv_q1d[Q_MOMENTUM][k] / A[k];
        e[k] = cv_q1d[Q_ENERGY][k] / (rho[k] * A[k]);
    }

    /*calculate change in entropy with respect to inlet */
    double c_p = kappa * r / (kappa - 1);
    double *s = ACCESS_ITER_DATA("s");
    s[0] = 0.;
    for (int i = 1; i < grid_size; i++)
    {
        s[i] = s[i-1] 
            + 2 * c_p * (T[i] - T[i-1]) / (T[i] + T[i-1])
            - 2 * r * (p[i] - p[i-1]) / (p[i] + p[i-1]);
    }

    free(p_int);
    free(a_int);
    free(c_int);

    for (int i = 0; i < CONSERV_EQS_COUNT; i++)
    {
        free(cv_q1d[i]);
        free(cv_q1d_next[i]);
        free(flx_q1d[i]);
        free(flx_q1d_next[i]);
    }

    /* if everything else succeeded, increment iteration count */
    return set_int_param(iterno_final, "iterno", ctr, parameters);
}
