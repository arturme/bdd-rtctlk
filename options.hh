#ifndef IISMC_OPTIONS_HH
#define IISMC_OPTIONS_HH

class Options
{

  public:
    unsigned int verbose;
    bool it_reorder;

    Options(void)
    {
      verbose = 0;
      it_reorder = false;
    }
};

#endif
