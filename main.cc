/*
   Copyright (c) 2012, 2013
   Artur Meski <meski@ipipan.waw.pl>

   Reuse of the code or its part for any purpose
   without the author's permission is strictly prohibited.
*/

#include "main.hh"

int main(int argc, char **argv)
{
    SymAutomataNet net;

    iisin_driver driver(&net);

    bool usage_error = false;
    bool reach_states = false;
    bool ctlk_model_checking = false;
    bool bmc = false;
    bool showsys = false;

    string loadsys = "";
    int loadsys_param = 2;

    static struct option long_options[] = 
    {
        {"trace-parsing",   no_argument,    0, 0   },
        {"trace-scanning",  no_argument,    0, 0   },
        {0,                 0,              0, 0   }
    };

    int c;
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "bcdl:n:vrs", long_options, &option_index)) != -1)
    {
        switch (c) {
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");

                if (strcmp(long_options[option_index].name, "trace-parsing"))
                    driver.trace_parsing = true;
                else if (strcmp(long_options[option_index].name, "trace-scanning"))
                    driver.trace_scanning = true;

                break;

            case 'b':
                bmc = true;
                break;

            case 'c':
                ctlk_model_checking = true;
                break;

            case 'd':
                showsys = true;
                break;

            case 'l':
                driver.nosys = true;
                loadsys = optarg;
                break;

            case 'n':
                loadsys_param = atoi(optarg);
                break;

            case 's':
                reach_states = true;
                break;

            case 'r':
                opts.it_reorder = true;
                break;

            case 'v':
                opts.verbose++;
                break;

            default:
                usage_error = true;
        }
    }

    std::string inputfile;
    if (optind < argc)
    {
        inputfile = argv[optind];
    }
    else
    {
        cout << "Missing input file" << endl;
        usage_error = true;
    }

    if (usage_error)
    {
        cout << endl
            << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl
            << " @@                                                   @@" << endl
            << " @@   Interleaved Interpreted Systems Model Checker   @@" << endl
            << " @@                                                   @@" << endl
            << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl
            << endl
            << "   Version:   " << VERSION << endl
            << "   Contact:   " << AUTHOR << endl
            << endl
            << " Usage: " << argv[0] << " [options] <inputfile>" << endl << endl
            << " Options:" << endl << endl
            << " -b       -- enable bounded model checking heuristic" << endl
            << " -c       -- (RT)CTLK model checking mode" << endl
            << " -d       -- show all the processes of the system" << endl
            << " -r       -- enable reordering of the BDD variables" << endl
            << " -s       -- generate the set of reachable states" << endl
            << " -v       -- verbose (can be used more than once to increase verbosity)" << endl
            << endl
            << " -l name  -- load compiled-in/parsed system" << endl
            << "             available systems:" << endl
            << "              * tgc  (train-gate-controller)" << endl
            << "              * pipe (pipeline protocol)" << endl
            << " -n N     -- scaling parameter for compiled-in systems (default: 2)" << endl
            << endl;
        return -100;
    }

    if (opts.verbose > 0) 
    {
        cout << "Verbose level: " << opts.verbose << endl;
    }

    VERB("Parsing " << inputfile);

    if (loadsys == "tgc")
        net = genTGC(loadsys_param);
    else if (loadsys == "pipe")
        net = genPipeline(loadsys_param);
    else
        FERROR("Unknown system: " << loadsys);
    
    if (driver.parse(inputfile))
        FERROR("Parse error");

    if (showsys) net.show();

    if (reach_states) getAllReach(net);

    FormCTLK *fctlk;
    if (ctlk_model_checking)
    {
        fctlk = driver.getFormCTLK();
        if (fctlk == NULL)
        {
            FERROR("Formula not defined");
        }
        fctlk->encodeAtoms(&net);
    }

    if (ctlk_model_checking && !bmc)
        checkCTLK(net, fctlk);

    if (ctlk_model_checking && bmc)
        boundedCheckCTLK(net, fctlk);

    return 0;
}

