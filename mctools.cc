// 
//  mctools.cc
//  BDD-LTLK
//  
//  Created by Artur Meski on 2010-10-14.
//  Copyright 2010 VerICS Team. All rights reserved.
// 

#include "mctools.hh"

bool mCheckerIsSetup = false;
Cudd *cuddMgr;
ParamsLTLwSAN *paramsLTLwSAN = NULL;
SymAutomataNet *sys_san;
vector<BDD> *sys_pv;
vector<BDD> *sys_pv_succ;
BDD *sys_pv_E;
BDD *sys_pv_succ_E;
BDD *sys_cur_trans;
ExistBDDcache *existBDDcache = NULL;
ParamsLTLbyForm *paramsLTLbyForm = NULL;

#ifdef MEASURE
double lastEncodingTime = 0;
#endif

void mCheckerSetup(SymAutomataNet *san)
{
#ifdef MEASURE
    lastEncodingTime = cpuTime();
#endif
    if (!mCheckerIsSetup)
    {
        cuddMgr = san->getCuddMgr();
        sys_san = san;
        sys_pv = san->getPVvector();
        sys_pv_succ = san->getPVsuccVector();
        sys_pv_E = san->getPVexist();
        sys_pv_succ_E = san->getPVsuccExist();
        sys_cur_trans = new BDD(*(san->bddTrans()));
        existBDDcache = new ExistBDDcache;
        mCheckerIsSetup = true;
    }
#ifdef MEASURE
    lastEncodingTime = cpuTime() - lastEncodingTime;
#endif
}

void mCheckerUnSetup(void)
{
    delete existBDDcache;
    delete sys_cur_trans;
    mCheckerIsSetup = false;
}

/* sat(f) dla tableau */
BDD
bddSatStates(FormLTLK *form)
{
    switch (form->getOper())
    {
    case LTL_NOT:
    {
        return !bddSatStates(form->getSubFormLeft());
        break;
    }
    case LTL_OR:
    {
        return bddSatStates(form->getSubFormLeft()) + bddSatStates(form->getSubFormRight());
        break;
    }
    case LTL_U:
    {
        /* trzeba odnaleźć formułę Xform w zbiorze elemForm
         * dla każdej podformuły z U musi być oddzielna zmienna,
         * więc wystarczy znaleźć jej indeks
         */
        int idx = paramsLTLwSAN->elemForm->getXFormIdx(form);
        assert(idx >= 0);
        return bddSatStates(form->getSubFormRight()) + (bddSatStates(form->getSubFormLeft()) * (*(paramsLTLwSAN->tabVars))[idx]);
        break;
    }
    case LTL_TF:
    {
        if (form->getTruth())
            return cuddMgr->bddOne();
        else
            return cuddMgr->bddZero();
        break;
    }
    case LTL_X:
    {
        int idx = paramsLTLwSAN->elemForm->getFormIdx(form);
        assert(idx >= 0);
        return (*(paramsLTLwSAN->tabVars))[idx];
        break;
    }
    case LTL_PV:
    {
        return *(form->getBDD());
        break;
    }
    }
    assert(form->getOper() != LTL_KD && form->getOper() != LTL_CD);
    FERROR("Unknown operator");
}

void
getFairConstr(FormLTLK *form)
{
    Oper oper = form->getOper();

    if (oper == LTL_U)
    {
        FormLTLK *constrForm = new FormLTLK(LTL_OR, new FormLTLK(LTL_NOT, form->getCopy()), form->getSubFormRight()->getCopy());
        paramsLTLwSAN->fairConstr->push_back(bddSatStates(constrForm));
        delete constrForm;
    }

    if (LTL_COND_1ARG(oper))
    {
        getFairConstr(form->getSubFormLeft());
    }

    if (LTL_COND_2ARG(oper))
    {
        getFairConstr(form->getSubFormLeft());
        getFairConstr(form->getSubFormRight());
    }
}

BDD
getFairEU(const BDD &reach, const BDD &states)
{
    BDD x = states; // b
    BDD x_p = reach;
    BDD y = reach; // a (true)
    while (x != x_p)
    {
        x_p = x;
        BDD tmp = ( *(paramsLTLwSAN->prod_trans) * x.SwapVariables(
                        *(paramsLTLwSAN->pv), *(paramsLTLwSAN->pv_succ)
                    )).ExistAbstract(*(paramsLTLwSAN->pv_succ_E));
        x = x + (y * tmp);
    }
    return x;
}

BDD
getFairEG(const BDD &reach)
{
    BDD x = reach;
    BDD x_p = cuddMgr->bddZero();
    while (x != x_p)
    {
        x_p = x;
        BDD isect = cuddMgr->bddOne();
        for (BDDset::iterator constr = paramsLTLwSAN->fairConstr->begin(); constr != paramsLTLwSAN->fairConstr->end(); constr++)
        {
            isect *= ( *(paramsLTLwSAN->prod_trans) * getFairEU(reach, x * (*constr)).SwapVariables(
                           *(paramsLTLwSAN->pv), *(paramsLTLwSAN->pv_succ)
                       )).ExistAbstract(*(paramsLTLwSAN->pv_succ_E));
        }
        x = x * isect;
    }
    return x;
}

/* Obliczanie BDD do kwantyfikacji pozbywającej się wszystkich komponentów poza k-tym */
BDD
bddOnlyIth(unsigned int k)
{
    if (existBDDcache->find(k) == existBDDcache->end())
    {
        BDD r = cuddMgr->bddOne();

#ifdef VERBOSE
        cout << "Computing bddOnlyIth for agent with ID=" << k << endl;
#endif

        // AutIdx autId = sys_san->getAgentAutomata(k).at(0);
        // 
        // unsigned int initial = sys_san->getAutomatonIndexBegin(autId);
        // unsigned int bound = sys_san->getAutomatonIndexBound(autId);
        // 
        // for (unsigned int i = 0; i < initial; i++)
        // {
        //     r *= (*(sys_pv))[i];
        // }
        // for (unsigned int i = bound; i < sys_san->getPVnvars(); i++)
        // {
        //     r *= (*(sys_pv))[i];
        // }
        // 
        // (*existBDDcache)[k] = r;

        unsigned int a_i = 0;
        VectAutIdx automata = sys_san->getAgentAutomata(k);
        AutIdx autId = automata.at(a_i);
        unsigned int varsBound = sys_san->getPVnvars();
        
        unsigned int first_excl_idx = sys_san->getAutomatonIndexBegin(autId);
        unsigned int last_excl_idx = sys_san->getAutomatonIndexBound(autId) - 1;
        
        for (unsigned int i = 0; i < varsBound; i++)
        {
            if (i < first_excl_idx)
            {
                r *= (*(sys_pv))[i];
            }
            else if (i == last_excl_idx)
            {
                a_i++;
                if (a_i < automata.size())
                {
                    assert(autId < automata.at(a_i)); /* must be sorted! */
                    
                    autId = automata.at(a_i);
                    first_excl_idx = sys_san->getAutomatonIndexBegin(autId);
                    last_excl_idx = sys_san->getAutomatonIndexBound(autId) - 1;
                }
                else
                {
                    first_excl_idx = varsBound;
                }
            }
        }

        (*existBDDcache)[k] = r;

        return r;
    }
    else
    {
#ifdef VERBOSE
        cout << "Using cached bddOnlyIth for agent with ID=" << k << endl;
#endif
        
        return (*existBDDcache)[k];
    }
}

/* Stany z których zaczynają się scieżki na których nie jest prawdziwa formuła LTL form */
BDD
states_LTL_Ex(void)
{
    assert(mCheckerIsSetup);
    assert(paramsLTLwSAN != NULL);

    return states_LTL_Ex_withInit(sys_san->bddInitLoc());
}

BDD
states_LTL_Ex_withInit(BDD const &init)
{
    assert(mCheckerIsSetup);
    assert(paramsLTLwSAN != NULL);

    compTabTrans();
    compFairConstr();

    BDD pv_E = *(paramsLTLwSAN->pv_E);
    vector<BDD> pv = *(paramsLTLwSAN->pv);
    vector<BDD> pv_succ = *(paramsLTLwSAN->pv_succ);
    BDD prod_trans = *(paramsLTLwSAN->prod_trans);

#ifdef BDD_VERBOSE
    cout << "BDD for transition relation of the product:" << endl;
    prod_trans.PrintMinterm();
#endif

    BDD prod_reach = init;
    BDD prod_reach_p = cuddMgr->bddZero();
    BDD prod_next;

    while (prod_reach != prod_reach_p)
    {
        prod_next = prod_reach * prod_trans;
        prod_next = (prod_next.ExistAbstract(pv_E)).SwapVariables(pv_succ, pv);
        prod_reach_p = prod_reach;
        prod_reach += prod_next;
    }

    BDD fairEG = getFairEG(prod_reach);
    BDD satStates_form = bddSatStates(paramsLTLwSAN->form);

#ifdef BDD_VERBOSE
    cout << "BDD for sat-states of tableau:" << endl;
    satStates_form.PrintMinterm();
#endif

    BDD result = satStates_form * fairEG;

    result = result.ExistAbstract(*(paramsLTLwSAN->tabVars_E));
#ifdef BDD_VERBOSE
    cout << "result-BDD:" << endl;
    result.PrintMinterm();
#endif

    return result;
}


/* Obliczanie zbioru stanów w których prawdziwe jest (KD_i form) */
BDD
satStates_K(FormLTLK *form, const BDD &reach)
{
    FormLTLK *subform = form->getSubFormLeft();
    BDD subform_bdd;

#ifdef VERBOSE
    cout << "Computing BDD for "<< form->toStr() << "." << endl;
#endif

	/* Jeśli subformuła jest zanegowaną zmienną zdaniową (drugi if) 
	 * lub po prostu zmienną zdaniową (pierwszy if), 
	 * to możemy szybciej obliczyć dla niej BDD - bez uruchamiania 
	 * weryfikacji LTL, konstruowania tableau, itd.
	 */
    Oper subformOp = subform->getOper();
    if (subformOp == LTL_PV)
    {
        subform_bdd = *(subform->getBDD()) * reach; /* jak mamy zmienną zdaniową, to nie ma co liczyć */
    }
    else if (subformOp == LTL_NOT && subform->getSubFormLeft()->getOper() == LTL_PV) /* wystarczy taki warunek jak form jest zredukowane */
    {
        subform_bdd = !*(subform->getSubFormLeft()->getBDD()) * reach;
    }
    else
    {
        paramsLTLformUpdate(subform);
        subform_bdd = states_LTL_Ex_withInit(reach); /* BDD ze stanami w których prawdziwe jest form */
    }

#ifdef BDD_VERBOSE
    cout << "Form BDD:" << endl;
    subform_bdd.PrintMinterm();
#endif

	BDD r;

	if (form->getOper() == LTL_KD)
	{
		// r = form_bdd.ExistAbstract(bddOnlyIth(form->getAgent())) * reach; /* czy faktycznie trzeba przecinać z reach? */
		r = subform_bdd.ExistAbstract(bddOnlyIth(form->getAgent()));
	}
	else if (form->getOper() == LTL_CD)
	{
		// subform_bdd - stany w których prawdziwa jest podformuła
		r = reach; // x
		BDD y = subform_bdd;
		while (r != y)
		{
			r = y;
			y = cuddMgr->bddZero();
			Agents *ags = form->getAgentsPtr();
			for (Agents::iterator a = ags->begin(); a != ags->end(); ++a)
				y += r.ExistAbstract(bddOnlyIth(*a)) * r; // ew. dodać ... * reach ;)
		}
	}
	
	// r = cuddMgr->bddZero(); // psuja :)
#ifdef BDD_VERBOSE
	    cout << "Final BDD for "<< form->toStr() << ":" << endl;
	    r.PrintMinterm();
#endif		

    return r;
}

/* Przygotowanie stuktury do zapamiętywania parametrów weryfikacji LTL per formuła LTL */
void
paramsLTLmemSetup(void)
{
    if (paramsLTLbyForm == NULL)
    {
        paramsLTLbyForm = new ParamsLTLbyForm;
    }
}

/* tylko tymczasowo, bo nie wolno usuwać tak sobie ... */
void
paramsLTLmemPopLast(void)
{
    assert(0);
    paramsLTLcleanup((*paramsLTLbyForm)[paramsLTLbyForm->size()-1]);
    paramsLTLbyForm->erase(paramsLTLbyForm->begin() + paramsLTLbyForm->size()-1);
}

/* Zwalnianie pamięci struktury do zapamiętywania parametrów weryfikacji LTL per formuła LTL */
void
paramsLTLmemCleanup(void)
{
    for (ParamsLTLbyForm::iterator ii = paramsLTLbyForm->begin(); ii != paramsLTLbyForm->end(); ii++)
    {
        paramsLTLcleanup(*ii);
    }
    delete paramsLTLbyForm;
    paramsLTLbyForm = NULL;
}

/* Existential (!!!) LTL verification setup */
int
paramsLTLsetup(FormLTLK *form)
{
    /* parametry weryfikacji - zapisywane globalnie */
    paramsLTLwSAN = new ParamsLTLwSAN;
    paramsLTLwSAN->form = form->getCopy();
    paramsLTLwSAN->elemForm = new FormLTLKset;
    form->getElem(*(paramsLTLwSAN->elemForm));

    /* ile dodatkowych zmiennych potrzeba na tableau */
    unsigned int tabNumVars = paramsLTLwSAN->elemForm->getNewVarsCount();

    /* wektory dla dodatkowych zmiennych dla tableau oraz BDD do kwantyfikacji */
    vector<BDD> *tabVars = new vector<BDD>(tabNumVars);
    vector<BDD> *tabVars_succ = new vector<BDD>(tabNumVars);
    BDD *tabVars_E = new BDD(cuddMgr->bddOne()); /* BDD do kwantyfikacji */

    unsigned int sanNumVars = sys_san->getPVnvars();
    for (unsigned int i = 0; i < tabNumVars; i++)
    {
        unsigned int k = (sanNumVars+i)*2;

        (*tabVars)[i] = cuddMgr->bddVar(k);
        (*tabVars_succ)[i] = cuddMgr->bddVar(k+1);

        *tabVars_E *= (*tabVars)[i]; /* BDD do kwantyfikacji */
    }

    /* zapisujemy wektory zmiennych tableau globalnie */
    paramsLTLwSAN->tabVars = tabVars;
    paramsLTLwSAN->tabVars_succ = tabVars_succ;
    paramsLTLwSAN->tabVars_E = tabVars_E;

    /* wektory dla PRODUKTU (automaty + tableau) oraz BDD do kwantyfikacji */
    unsigned int prodNumVars = sanNumVars + tabNumVars;
    vector<BDD> *pv = new vector<BDD>(prodNumVars);
    vector<BDD> *pv_succ = new vector<BDD>(prodNumVars);
    BDD *pv_succ_E = new BDD(cuddMgr->bddOne()); /* BDD do kwantyfikacji */
    BDD *pv_E = new BDD(cuddMgr->bddOne());

    for (unsigned int i = 0; i < prodNumVars; i++)
    {
        (*pv)[i] = cuddMgr->bddVar(i*2);
        (*pv_succ)[i] = cuddMgr->bddVar((i*2)+1);

        *pv_E *= (*pv)[i];
        *pv_succ_E *= (*pv_succ)[i];
    }
    paramsLTLwSAN->pv = pv;
    paramsLTLwSAN->pv_succ = pv_succ;
    paramsLTLwSAN->pv_E = pv_E;
    paramsLTLwSAN->pv_succ_E = pv_succ_E;

    /* BDD z relacją przejścia tableau */
    // BDD tableau_trans = cuddMgr->bddOne();
    // FormLTLKvect allXforms = paramsLTLwSAN->elemForm->getAllXForm();
    // for (FormLTLKvect::iterator formXg = allXforms.begin(); formXg != allXforms.end(); formXg++)
    // {
    // 	BDD sat_Xg = bddSatStates(*formXg);
    // 	BDD sat_g = bddSatStates((*formXg)->getSubFormLeft()).SwapVariables(*pv, *pv_succ);
    // 	tableau_trans *= BDD_IFF(sat_Xg, sat_g);
    // }

    paramsLTLwSAN->allXforms = new FormLTLKvect;
    *(paramsLTLwSAN->allXforms) = paramsLTLwSAN->elemForm->getAllXForm();

    /* relacja przejścia produktu */
    // paramsLTLwSAN->prod_trans = new BDD(tableau_trans * *(sys_san->bddTrans()));
    paramsLTLwSAN->prod_trans = new BDD;

    paramsLTLwSAN->fairConstr = NULL;
    // paramsLTLwSAN->fairConstr = new BDDset;
    // getFairConstr(negForm); /* dopisać asercję sprawdzającą czy jest zainicjalizowany zbiór!!! */

    paramsLTLbyForm->push_back(paramsLTLwSAN);
    int newId = paramsLTLbyForm->size()-1;

    paramsLTLwSAN = NULL;

    return newId;
}

/* Computing fairness constraints */
void
compFairConstr(void)
{
#ifdef VERBOSE
    cout << "Computing fairness constraints for " << paramsLTLwSAN->form->toStr() << endl;
#endif
    if (paramsLTLwSAN->fairConstr != NULL) delete paramsLTLwSAN->fairConstr;
    paramsLTLwSAN->fairConstr = new BDDset;
    getFairConstr(paramsLTLwSAN->form); /* dopisać asercję sprawdzającą czy jest zainicjalizowany zbiór!!! */
}

/* Computing transition relation for tableau and product */
void
compTabTrans(void)
{
#ifdef VERBOSE
    cout << "Computing new transition relation for tableau and product" << endl;
#endif

    delete paramsLTLwSAN->elemForm;
    paramsLTLwSAN->elemForm = new FormLTLKset;
    paramsLTLwSAN->form->getElem(*(paramsLTLwSAN->elemForm));
    *(paramsLTLwSAN->allXforms) = paramsLTLwSAN->elemForm->getAllXForm();
    
    BDD tab_trans = cuddMgr->bddOne();
    for (FormLTLKvect::iterator formXg = paramsLTLwSAN->allXforms->begin(); formXg != paramsLTLwSAN->allXforms->end(); formXg++)
    {
        BDD sat_Xg = bddSatStates(*formXg);
        BDD sat_g = bddSatStates((*formXg)->getSubFormLeft()).SwapVariables(*(paramsLTLwSAN->pv), *(paramsLTLwSAN->pv_succ));
        tab_trans *= BDD_IFF(sat_Xg, sat_g);
    }
    *(paramsLTLwSAN->prod_trans) = tab_trans * *(sys_cur_trans);

#ifdef HARD_VERBOSE
    cout << "+ Tableau's transition relation";
    tab_trans.print(sys_san->getPVnvars(), 1);
    cout << "+ System's transition relation";
    sys_cur_trans->print(sys_san->getPVnvars(), 1);
    cout << "+ Product transition relation";
    paramsLTLwSAN->prod_trans->print(sys_san->getPVnvars(), 1);
#endif
}

void
paramsLTLformUpdate(FormLTLK *form)
{
    delete paramsLTLwSAN->form;
    paramsLTLwSAN->form = form;
}

/* Ładowanie odpowiednich parametrów do weryfikacji LTL */
void
paramsLTLload(int id)
{
    if (id >= 0)
    {
        paramsLTLwSAN = (*paramsLTLbyForm)[id];
#ifdef VERBOSE
        cout << "Loaded LTL verification parameters ID=" << id << " ";
        cout << "for formula " << paramsLTLwSAN->form->toStr() << endl;
#endif
    }
    else
    {
#ifdef VERBOSE
        cout << "OK, not loading LTL verification parameters for TOO SIMPLE FORMULA" << endl;
#endif
        paramsLTLwSAN = NULL;
    }
}

void
paramsLTLcleanup(ParamsLTLwSAN *params)
{
    delete params->form;
    delete params->elemForm;
    delete params->tabVars;
    delete params->tabVars_succ;
    delete params->tabVars_E;
    delete params->pv;
    delete params->pv_succ;
    delete params->pv_E;
    delete params->pv_succ_E;
    delete params->prod_trans;
    delete params->fairConstr;
    delete params->allXforms;
    delete params;
}

void
paramsLTLcleanup(void)
{
    paramsLTLcleanup(paramsLTLwSAN);
    paramsLTLwSAN = NULL;
}

void simplifyLTLK(FormLTLK **form_p, VerTreeLTLK &vt)
{
    FormLTLK *form = *form_p;

    Oper op = form->getOper();
    if (LTL_COND_1ARG(op))
    {
        if (op == LTL_KD || op == LTL_CD)
        {
            string formStr = form->toStr();
            /* Podmiana podformuły na zmienną */
            delete form;
            *form_p = new FormLTLK("[PV<=>" + formStr + "]", vt.getFormStates(formStr));
        }
        else
        {
            simplifyLTLK(form->getSubFormLeft2Ptr(), vt);
        }

    }
    else if (LTL_COND_2ARG(op))
    {
        simplifyLTLK(form->getSubFormLeft2Ptr(), vt);
        simplifyLTLK(form->getSubFormRight2Ptr(), vt);
    }
}

void
restrictTrans(BDD &trans, BDD &reach)
{
    *sys_cur_trans = trans * reach.SwapVariables(*sys_pv, *sys_pv_succ);
#ifdef VERBOSE
    cout << "Restricted transition relation";
    sys_cur_trans->print(sys_san->getPVnvars(), 1);
#endif
#ifdef BDD_VERBOSE
    cout << "BDD for restricted transition relation:" << endl;
    (*sys_cur_trans).PrintMinterm();
#endif
}

/********************************************/

VerTreeLTLK::VerTreeLTLK(FormLTLK *form)
{
    prepVerTree(form);
    picked = false;
}

/* Przygotowywanie struktury drzewa do obliczania dla LTLK */
void
VerTreeLTLK::prepVerTree(FormLTLK *form)
{
    Oper op = form->getOper();
    if (LTL_COND_1ARG(op))
    {
        prepVerTree(form->getSubFormLeft());
    }
    else if (LTL_COND_2ARG(op))
    {
        prepVerTree(form->getSubFormLeft());
        prepVerTree(form->getSubFormRight());
    }

    if (op == LTL_KD || op == LTL_CD)
    {
        addForm(form);
    }
}

/* Dodawanie formuły */
void
VerTreeLTLK::addForm(FormLTLK *form)
{
    /* Brakuje sprawdzania, czy już nie mamy zapisanej takiej formuły */
#ifdef VERBOSE
    cout << "Saving " << form->toStr() << "." << endl;
#endif

    FormBDD new_fb;
    new_fb.states = NULL;
    new_fb.form = form->getCopy();

    FormLTLK *subform = form->getSubFormLeft();
    Oper op = subform->getOper();

    if (op == LTL_PV)
        new_fb.paramsId = -2;
    else if (op == LTL_NOT && subform->getSubFormLeft()->getOper() == LTL_PV)
        new_fb.paramsId = -3;
    else
    {
        new_fb.paramsId = paramsLTLsetup(subform); /* bierzemy formułę za K */
    }
    verTreeList.push_back(new_fb);
}

FormLTLK *
VerTreeLTLK::pickUnproc(void)
{
    for (VerTreeList::iterator ii = verTreeList.begin(); ii != verTreeList.end(); ii++)
    {
        if ((*ii).states == NULL)
        {
            last_pick = ii;
            picked = true;
            return (*ii).form;
        }
    }
    return NULL;
}

int
VerTreeLTLK::getLastPickParams(void)
{
    assert(picked);
    return (*last_pick).paramsId;
}

void
VerTreeLTLK::loadLastPickParams(void)
{
    assert(picked);
    paramsLTLload((*last_pick).paramsId);
}

void
VerTreeLTLK::saveWithLastPick(const BDD &states)
{
    assert(picked);
    (*last_pick).states = new BDD(states);
}

BDD
VerTreeLTLK::getFormStates(string formStr)
{
    for (VerTreeList::iterator ii = verTreeList.begin(); ii != verTreeList.end(); ii++)
    {
        if ((*ii).states != NULL && (*ii).form->toStr() == formStr)
        {
            return *((*ii).states);
        }
    }
    FERROR("Not found");
}

void
VerTreeLTLK::cleanup(void)
{
    picked = false;
    for (VerTreeList::iterator ii = verTreeList.begin(); ii != verTreeList.end(); ii++)
    {
        delete (*ii).states;
        (*ii).states = NULL;
    }
}

void
VerTreeLTLK::show(void)
{
    for (VerTreeList::iterator ii = verTreeList.begin(); ii != verTreeList.end(); ii++)
    {
        cout << "VerTree BDD for " << (*ii).form->toStr() << endl;
        (*ii).states->PrintMinterm();
    }
}

VerTreeLTLK::~VerTreeLTLK(void)
{
    for (VerTreeList::iterator ii = verTreeList.begin(); ii != verTreeList.end(); ii++)
    {
        delete (*ii).states;
        delete (*ii).form;
    }
}

/********************************************/

BDD preImage(const BDD &states)
{
    BDD result = states.SwapVariables(*sys_pv, *sys_pv_succ) * *sys_cur_trans;
    result = result.ExistAbstract(*sys_pv_succ_E);
    return result;
}
