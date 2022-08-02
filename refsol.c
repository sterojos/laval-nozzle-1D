#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MACH_DELTA 1e-10
#define SHOCKWAVE_DELTA 1e-10
#define SUBSONIC 0
#define SUPERSONIC 1

double mach_newton_iter(double mach, double A, double A_crit, double *mach_consts)
{
    double exponent = mach_consts[0];
    double const1 = mach_consts[1];
    double const2 = mach_consts[2];
    double formula = QUASI1D_MACH_FORMULA(const1, const2, exponent, mach);
    double formula_derivative = QUASI1D_MACH_FORMULA(const1, const2, exponent - 1, mach);
    double zero_function = formula - A_crit / A;
    double zero_func_derivative = (formula / mach - formula_derivative * 6 * const2 * mach);
    return mach - zero_function / zero_func_derivative;
}

double get_mach_quasi1d(double A, double A_crit, double *mach_consts, int flow_type)
{
    double mach;
    double A_inv_ratio = A / A_crit;
    /* initial guess */
    if (flow_type == SUPERSONIC)
    {
        if (A_inv_ratio > 90)
            mach = 1.3 * pow((A / A_crit), 1. / 3);
        else if (A_inv_ratio < 1.2)
            mach = A_inv_ratio;
        else
            mach = sqrt(A / A_crit);
    }
    else if (flow_type == SUBSONIC)
    {
        mach = sqrt(A_crit / A);
    }
    double mach_next;
    while (fabs(mach - (mach_next = mach_newton_iter(mach, A, A_crit, mach_consts))) > MACH_DELTA)
    {
        mach = mach_next;
    }
    return mach_next;
}

double get_shockwave_area(double const p_stat_in, double const p_dyn_out, double A_min,
                          double A_max, double A_crit, double A_out, double const delta,
                          double *mach_consts, double kappa)
{
    double const A_diff = A_max - A_min;
    double A_shock = A_min + A_diff / 2;
    double Ma1 = get_mach_quasi1d(A_shock, A_crit, mach_consts, SUPERSONIC);
    double Ma2 = sqrt(MACH_RED_SHOCK(Ma1, kappa));
    double p_stat_out = STAT_PRES_RED_SHOCK(Ma1, p_stat_in, kappa);
    double A_crit_out = QUASI1D_MACH_FORMULA(mach_consts[1], mach_consts[2], mach_consts[0], Ma2) * A_shock;
    double Ma_out = get_mach_quasi1d(A_out, A_crit_out, mach_consts, SUBSONIC);
    double p_dyn_out_exp = P_DYN(p_stat_out, Ma_out, kappa);
    if (fabs(p_dyn_out - p_dyn_out_exp) < delta)
        return A_shock;
    if (p_dyn_out < p_dyn_out_exp)
        get_shockwave_area(p_stat_in, p_dyn_out, A_shock, A_max, A_crit, A_out, delta, mach_consts, kappa);
    else
        get_shockwave_area(p_stat_in, p_dyn_out, A_min, A_shock, A_crit, A_out, delta, mach_consts, kappa);
}

int get_n_shock(local_data data, int *ctr, int n_min, int n_max, double A_shock)
{
    int id_A = get_prop_id("A", ctr, data, ITER_VARS);
    if (abs(n_max - n_min) <= 1)
    {
        double upper_val = data.refsol_vars[id_A].values[n_max];
        double lower_val = data.refsol_vars[id_A].values[n_min];
        if (fabs(upper_val - A_shock) < fabs(lower_val - A_shock))
        {
            return n_max;
        }
        else
        {
            return n_min;
        }
    }
    int n_exp = (n_min + n_max) / 2;
    double A_exp = data.refsol_vars[id_A].values[n_exp];
    if (A_exp < A_shock)
    {
        get_n_shock(data, ctr, n_exp, n_max, A_shock);
    }
    else if (A_exp > A_shock)
    {
        get_n_shock(data, ctr, n_min, n_exp, A_shock);
    }
    else
    {
        return n_exp;
    }
}

int refsol_func(int *ctr, local_data data, params *parameters)
{
    /* add safety checks? */
    double kappa = get_double_param("kappa", ctr, *parameters);
    double A_crit = get_double_param("A_crit", ctr, *parameters);
    double A_out = get_double_param("A_max", ctr, *parameters);
    double p_stat_in = get_double_param("p0", ctr, *parameters);
    double p_dyn_out = get_double_param("p1", ctr, *parameters);
    double R = get_double_param("r_gas", ctr, *parameters);

    if(p_dyn_out >= p_stat_in)
    {
        puts(COMMENT_PRESSURE_ERROR);
        return -2;
    }

    double mach_consts[3] =
        {
            -(kappa + 1.) / (2 * (kappa - 1.)),
            pow((kappa + 1.) / 2, -mach_consts[0]),
            (kappa - 1.) / 2};

    const double mach1_out_exp = get_mach_quasi1d(A_out, A_crit, mach_consts, SUPERSONIC);
    const double p_dyn_out_exp = P2_DYN_SHOCK(mach1_out_exp, p_stat_in, kappa);

    double A_shock, x_shock, r_shock, A_crit_out, delta_s, p_stat_out;
    A_shock = x_shock = r_shock = -2;
    if (p_dyn_out > p_dyn_out_exp)
    {
        A_shock = get_shockwave_area(p_stat_in, p_dyn_out, A_crit, A_out, A_crit, A_out, SHOCKWAVE_DELTA, mach_consts, kappa);
        r_shock = sqrt(A_shock / M_PI);
        double Ma1 = get_mach_quasi1d(A_shock, A_crit, mach_consts, SUPERSONIC);
        double Ma2 = sqrt(MACH_RED_SHOCK(Ma1, kappa));
        p_stat_out = STAT_PRES_RED_SHOCK(Ma1, p_stat_in, kappa);
        A_crit_out = QUASI1D_MACH_FORMULA(mach_consts[1], mach_consts[2], mach_consts[0], Ma2) * A_shock;
        delta_s = R * log(p_stat_in / p_stat_out);
    }

    int n_shock = -2;
    int n = ctr[RSOL_N_CTR];
    int n_crit = get_int_param("nc_r", ctr, *parameters);
    double l = get_double_param("l", ctr, *parameters);
    if (A_shock >= A_crit)
    {
        n_shock = get_n_shock(data, ctr, n_crit, n, A_shock);
        x_shock = l * ((double) n_shock / n);
    }
    /* write all data */
    int error_return;
    if ((error_return = set_double_param(A_shock, "A_shock", ctr, *parameters)) < 0)
        return error_return;
    if ((error_return = set_double_param(r_shock, "r_shock", ctr, *parameters)) < 0)
        return error_return;
    if ((error_return = set_double_param(x_shock, "x_shock", ctr, *parameters)) < 0)
        return error_return;
    if ((error_return = set_double_param(delta_s, "delta_s", ctr, *parameters)) < 0)
        return error_return;

    double *A = ACCESS_REFSOL_DATA("A");
    double *Ma = ACCESS_REFSOL_DATA("Ma");
    double *p = ACCESS_REFSOL_DATA("p");
    double *T = ACCESS_REFSOL_DATA("T");
    double *a = ACCESS_REFSOL_DATA("a");
    double *c = ACCESS_REFSOL_DATA("c");
    double *rho = ACCESS_REFSOL_DATA("rho");
    double *s = ACCESS_REFSOL_DATA("s");
    double *h = ACCESS_REFSOL_DATA("h");
    double *h_t = ACCESS_REFSOL_DATA("h_t");
    double *m_flux = ACCESS_REFSOL_DATA("m_flux");
    double *P = ACCESS_REFSOL_DATA("P");
    double *e = ACCESS_REFSOL_DATA("e");

    double c_v = R / (kappa - 1);

    double T_stat = get_double_param("T0", ctr, *parameters);
    double a_stat = sqrt(kappa * R * T_stat);
    if (n_shock == -2)
        n_shock = ctr[RSOL_N_CTR];
    for (int i = 0; i < n_crit; i++)
    {
        Ma[i] = get_mach_quasi1d(A[i], A_crit, mach_consts, SUBSONIC);
        p[i] = P_DYN(p_stat_in, Ma[i], kappa);
        s[i] = 0;
    }
    Ma[n_crit] = 1;
    p[n_crit] = P_DYN(p_stat_in, Ma[n_crit], kappa);
    for (int i = n_crit + 1; i < n_shock; i++)
    {
        Ma[i] = get_mach_quasi1d(A[i], A_crit, mach_consts, SUPERSONIC);
        p[i] = P_DYN(p_stat_in, Ma[i], kappa);
        s[i] = 0;
    }
    for (int i = n_shock; i < ctr[RSOL_N_CTR]; i++)
    {
        Ma[i] = get_mach_quasi1d(A[i], A_crit_out, mach_consts, SUBSONIC);
        p[i] = P_DYN(p_stat_out, Ma[i], kappa);
        s[i] = delta_s;
    }

    for (int i = 0; i < ctr[RSOL_N_CTR]; i++)
    {
        T[i] = T_DYN(T_stat, Ma[i], kappa);
        a[i] = A_SND_DYN(a_stat, Ma[i], kappa);
        c[i] = Ma[i] * a[i];
        rho[i] = p[i] / (R * T[i]);
        e[i] = p[i] / (kappa - 1.) + c[i] * c[i] / 2.;
        h[i] = c_v * T[i] + p[i] / rho[i];
        h_t[i] = h[i] + c[i] * c[i] / 2.;
        P[i] = rho[i] * c[i];
        m_flux[i] = P[i] * A[i];
    }

    /* signal that data in the array is now not garbage */
    data.initialized[REFSOL_VARS] = INITIALIZED;
    return 0;
}