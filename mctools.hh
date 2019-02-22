//
//  mctools.hh
//
//  Created by Artur Meski on 2010-10-14.
//

#ifndef MCTOOLS_HH_AC4K84MQ
#define MCTOOLS_HH_AC4K84MQ

#include <iostream>
#include <assert.h>
#include "bdd_macro.hh"
#include "cudd/include/cuddObj.hh"
#include "automata.hh"
#include "formula.hh"
#include "memtime.h"
#include <iomanip>

class VerTreeLTLK
{
    struct FormBDD {
      FormLTLK *form;
      BDD *states;
      int paramsId;
    };

    typedef std::vector<FormBDD> VerTreeList;

    VerTreeList verTreeList;
    VerTreeList::iterator last_pick;
    bool picked;

    void addForm(FormLTLK *form);
  public:
    VerTreeLTLK(FormLTLK *formLTLK);
    ~VerTreeLTLK(void);
    void prepVerTree(FormLTLK *form);
    FormLTLK *pickUnproc(void);
    int getLastPickParams(void);
    void loadLastPickParams(void);
    void saveWithLastPick(const BDD &states);
    BDD getFormStates(string formStr);
    void show(void);
    void cleanup(void);
};

typedef std::vector<BDD> BDDset;
typedef std::map<unsigned int, BDD> ExistBDDcache;

struct ParamsLTLwSAN { /* Parameters for verification of LTL with SymAutomataNet */
  FormLTLK *form;
  FormLTLKset *elemForm;
  vector<BDD> *tabVars; /* zmienne tableau */
  vector<BDD> *tabVars_succ;
  BDD *tabVars_E;
  vector<BDD> *pv; /* zmienne produktu */
  vector<BDD> *pv_succ;
  BDD *pv_E; /* BDD do kwantyfikacji dla produktu */
  BDD *pv_succ_E;
  BDD *prod_trans; /* relacja przejścia dla produktu */
  BDDset *fairConstr;
  FormLTLKvect *allXforms;
};

typedef std::vector<ParamsLTLwSAN *> ParamsLTLbyForm;

extern bool mCheckerIsSetup;
extern Cudd *cuddMgr;
extern ParamsLTLwSAN *paramsLTLwSAN;
extern SymAutomataNet *sys_san;
extern vector<BDD> *sys_pv;
extern vector<BDD> *sys_pv_succ;
extern BDD *sys_pv_E;
extern BDD *sys_pv_succ_E;
extern BDD *sys_cur_trans;
extern ExistBDDcache *existBDDcache;
extern ParamsLTLbyForm *paramsLTLbyForm;

#ifdef MEASURE
extern double lastEncodingTime;
#endif

void mCheckerSetup(SymAutomataNet *san);
void mCheckerUnSetup(void);
BDD bddSatStates(FormLTLK *form);
void getFairConstr(FormLTLK *form);
BDD getFairEU(const BDD &reach, const BDD &states);
BDD getFairEG(const BDD &reach);
BDD bddOnlyIth(unsigned int i);
void prepVerTree(FormLTLK *form);
BDD states_LTL_Ex(void);
BDD states_LTL_Ex_withInit(BDD const &init);
BDD satStates_K(FormLTLK *form, const BDD &reach);
void paramsLTLmemSetup(void);
void paramsLTLmemCleanup(void);
void paramsLTLmemPopLast(void);
int paramsLTLsetup(FormLTLK *form);
void compFairConstr(void);
void compTabTrans(void);
void paramsLTLformUpdate(FormLTLK *form);
void paramsLTLload(int id);
void paramsLTLcleanup(void);
void paramsLTLcleanup(ParamsLTLwSAN *params);
void simplifyLTLK(FormLTLK **form_p, VerTreeLTLK &vt);
void restrictTrans(BDD &trans, BDD &reach);
BDD preImage(const BDD &states);

using std::cout;

#endif /* end of include guard: MCTOOLS_HH_AC4K84MQ */
