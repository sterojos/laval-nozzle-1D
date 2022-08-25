#define EN 0
#define CS 1

#ifndef LANG
#define LANG CS
#endif

#if LANG == CS

#define COMMENT_BUFFER_ERR \
    "Chyba v " MAIN_FILE ": prekroceni max. delky retezce."
#define COMMENT_SUGGEST_BUFFER_FIX(desc, macro) \
    "Zkontroluj " desc " nebo zvetsi hodnotu makra " TAG(macro) "."
#define COMMENT_PARAM_NAMES "nazvy parametru"
#define COMMENT_PARAM_DESC "popisy parametru"
#define COMMENT_PARAM_COUNT "pocet parametru"
#define COMMENT_FUNC_COUNT "pocet funkci"
#define COMMENT_FUNC_NAMES "nazvy funkci"
#define COMMENT_VAR_HANDLES "zkratky promennych"
#define COMMENT_VAR_NAMES "nazvy promennych"
#define COMMENT_DECIMALS "pocet desetinnych mist v souboru"
#define COMMENT_FILE_FOUND \
    "Byl nalezen soubor, ktery vypada jako data z predchoziho vypoctu. \n\
Mohu ho nacist, nebo smazat a nahradit novym souborem. \n\
Mam se ho pokusit nacist? (y/n)"
#define COMMENT_FILE_CREATED \
"Soubor byl vytvoren. Bude nyni nutne definovat parametry."
#define COMMENT_FILE_NOT_FOUND_READ \
    "Funkce read_all_params() nenasla soubor " PARAM_FILE_NAME ". Neco se hodne zvrtlo."
#define COMMENT_FILE_NOT_FOUND_SAVE \
    "Funkce save_all_params() nedokazala otevrit soubor " PARAM_FILE_NAME ". Neco se hodne zvrtlo."
#define COMMENT_FILE_WRONG_ORDER \
    "Chyba v poradi parametru v souboru (read_all_params())."
#define COMMENT_FILE_GENERIC_ERROR \
    "Chyba ve strukture souboru " PARAM_FILE_NAME " (read_all_params())."
#define COMMENT_FILE_EXTRA_DATA \
    "Zda se, ze v souboru " PARAM_FILE_NAME " jsou data navic. Ignoruji je."
#define COMMENT_FILE_EXIT \
    "Bylo zakazano vytvoreni souboru s parametry. Program bude nyni ukoncen."
#define COMMENT_FILE_NONEXISTENT \
    "Nepodarilo se nacist parametry z predchoziho vypoctu. \n\
    Chces vytvorit novy soubor s parametry? (y/n)."
#define COMMENT_FILE_LOAD_SUCCESS \
    "Nalezeny soubor se podarilo nacist."
#define COMMENT_FILE_LOAD_FAILURE \
    "Nalezeny soubor se nepodarilo nacist. \
    Chces vytvorit novy soubor s parametry? (y/n)."
#define COMMENT_MISSING_FILE_NAME \
    "Chybi adresa ciloveho souboru. Priklad: moje_data/data1.txt"
#define COMMENT_FOPEN_ERROR \
    "Nepodarilo se vytvorit soubor:"
#define COMMENT_EXTRA_ARGS \
    "Nezname argumenty na konci. Interpretuji jako:"
#define COMMENT_BOOL_INPUT_ERROR \
    "Nechapu. Zadej \'y\' jako ano, \'n\' jako ne."
#define COMMENT_REMIND_UNITS \
    "Nyni muzes postupne zadat vsechny parametry. Dodrzuj volbu zakladnich jednotek."
#define COMMENT_NDEF_PARAMS \
    "Vypada to, ze jsi bud nezadal vsechny parametry, nebo v dyze nevznikla razova vlna."
#define COMMENT_NDEF_PARAM \
    "-1 (neznama hodnota)"
#define COMMENT_NDEF_SHOCK \
    "-2 (raz mimo dyzu)"
#define COMMENT_REQUEST_INPUT "Zadej prikaz: "
#define COMMENT_EOF_REACHED "Byl dosazen konec souboru."
#define COMMENT_INVALID_COMMAND_STDIN "Neznamy prikaz. Zkus help."
#define COMMENT_INVALID_COMMAND_FILE "... [CHYBA]"
#define COMMENT_SUS_INPUT \
    "Zadej pouze ciselnou hodnotu parametru. (Pouzij desetinnou tecku, nikoliv carku.)"
#define COMMENT_ENFORCE_POSITIVE_INPUT \
    "Zda se, ze jsi nezadal kladnou nenulovou hodnotu parametru. Zkus to znovu."
#define COMMENT_GENERIC_INPUT_ERROR \
    "Nespravny format vstupu. Zadej prosim ciselnou hodnotu."
#define COMMENT_ENFORCE_INTEGER_INPUT \
    "Nespravny format vstupu. Zadej prosim celociselnou hodnotu."
#define COMMENT_TYPE_ERROR_INT \
    "Nespravny datovy typ parametru (get_int_param):"
#define COMMENT_TYPE_ERROR_DOUBLE \
    "Nespravny datovy typ parametru (get_double_param):"
#define COMMENT_PARAM_NOT_FOUND \
    "Hledani neexistujiciho parametru:"
#define COMMENT_ILLEGAL_PARAM_ACCESS \
    "Nelze upravit parametr:"
#define COMMENT_PARAM_TYPE_ERROR \
    "Chyba pri zapisovani parametru:"
#define COMMENT_NOZZLE_PROP_ERROR \
    "Neocekavana odlisnost v poradi promennych (init_nozzle)."
#define COMMENT_INVALID_DATA_PRINT \
    "Nekterou z techto velicin neznam:"
#define COMMENT_PLOT_DATAFILE_ERROR \
    "Nebylo mozne zapsat data pro vykresleni grafu."
#define COMMENT_GNUPLOT_ERROR \
    "Nepodarilo se otevrit gnuplot. Graf proto nebude vykreslen."
#define COMMENT_VAR_NONEXISTENT \
    "Vztah pro neznamou promennou: "
#define COMMENT_CONFIRM_RESET \
    "Zmena tohoto parametru restartuje vypocet. \n\
Opravdu chces pokracovat? (y/n)"
#define COMMENT_REQUEST_RESET \
    "Parametr bude zmenen. \n \
Zmena tohoto parametru muze znicit dosavadni vysledky. \n\
Nechces nahodou restart? (y/n)"
#define COMMENT_ERROR_RETURN "Pri behu funkce vznikla chyba..."
#define COMMENT_FNC_NOT_FOUND "Funkce nebyla nalezena. \
Zadej \"help\" pro zobrazeni seznamu funkci."
#define COMMENT_NO_RESIDUUM "Soubor s rezidui neexistuje, protoze nebyl \
proveden dostatek iteraci..."
#define COMMENT_MISSING_ITER_COUNT "Chybi pocet iteraci. (pr. \"iter 300000\")"
#define COMMENT_PRESSURE_ERROR \
    "Prilis vysoky tlak na vystupu nebo nizky tlak na vstupu."

#define DESC_P0 "tlak na vstupu trysky"
#define DESC_P1 "tlak na vystupu trysky"
#define DESC_T0 "teplota na vstupu trysky"
#define DESC_T1 "teplota na vystupu trysky"
#define DESC_ACRIT "nejmensi (kriticky) prurez trysky"
#define DESC_RCRIT "nejmensi (kriticky) prumer trysky"
#define DESC_AMAX "nejvetsi prurez trysky"
#define DESC_RMAX "nejvetsi prumer trysky"
#define DESC_L "delka trysky"
#define DESC_CFL "maximalni hodnota CFL kriteria"
#define DESC_EPS "umela difuze schematu (mezi 0 a 1)"
#define DESC_RGAS "merna plynova konstanta"
#define DESC_KAPPA "Poissonova konstanta (adiabaticky exponent)"
#define DESC_N "pocet bodu v siti"
#define DESC_ITERNO "pocet iteraci"
#define DESC_XCRIT "vzdalenost od vstupu ke krit. prurezu"
#define DESC_DELTA_S "ztrata merne entropie na razove vlne"

#define DESC_MAX_RES "maximalni reziduum"
#define DESC_AVG_RES "prumerne reziduum"

#define DESC_VAR_X "vzdalenost podel trysky"
#define DESC_VAR_P "staticky tlak"
#define DESC_VAR_R "prumer dyzy"
#define DESC_VAR_A "prurez dyzy"
#define DESC_VAR_ALPHA "uhel rozsirovani dyzy ve stupnich"
#define DESC_VAR_T "teplota"
#define DESC_VAR_RHO "hustota"
#define DESC_VAR_C "rychlost proudeni"
#define DESC_VAR_A_SND "rychlost zvuku"
#define DESC_VAR_MACH "Machovo cislo"
#define DESC_VAR_H "merna entalpie"
#define DESC_VAR_S "merna entropie"
#define DESC_VAR_AX "derivace prurezu"
#define DESC_VAR_HT "klidova entalpie"
#define DESC_VAR_MFLUX "hmotnostni tok"
#define DESC_VAR_PP "hybnost"
#define DESC_VAR_EN "energie"

#define HELP_STRING \
"params \n\
params backup soubor.txt \n\
params save \n\
params edit all \n\
params edit parametr 20 \n\
refsol \t (vykonava se vetsinou automaticky) \n\
data backup soubor.txt \n\
data backup soubor.txt iter \n\
data backup soubor.txt ref \n\
data info \t (nazvy promennych)\n\
plot promenna1 promenna2 \n\
plot promenna1 promenna2 iter \n\
plot promenna1 promenna2 ref \n\
res \t (graf rezidui) \n\
reset \t (restartuje numericky vypocet na poc. podminku)"

#elif LANG == EN
#define DESC_P0 "pressure at nozzle inlet"
#endif
