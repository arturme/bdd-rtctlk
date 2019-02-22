// 
//  formula.cc
//  BDD-LTLK
//  
//  Created by Artur Meski on 2010-10-14.
//  Copyright 2010 VerICS Team. All rights reserved.
// 

#include "formula.hh"

FormLTLK::FormLTLK(string varName, BDD varBDD)
{
    oper = LTL_PV;
    name = varName;
    bdd = new BDD(varBDD);
}

FormLTLK::FormLTLK(bool val)
{
    oper = LTL_TF;
    tf = val;
}

FormLTLK::FormLTLK(Oper op, FormLTLK *form1)
{
    if (!LTL_COND_1ARG(op)) FERROR("Wrong operator supplied");

    oper = op;
    subForm[0] = form1;
}

FormLTLK::FormLTLK(Oper op, FormLTLK *form1, FormLTLK *form2)
{
    if (!LTL_COND_2ARG(op)) FERROR("Wrong operator supplied");

    oper = op;
    subForm[0] = form1;
    subForm[1] = form2;
}

/* infiksowo */
FormLTLK::FormLTLK(FormLTLK *form1, Oper op, FormLTLK *form2)
{
    if (!LTL_COND_2ARG(op)) FERROR("Wrong operator supplied");

    oper = op;
    subForm[0] = form1;
    subForm[1] = form2;
}

FormLTLK::FormLTLK(Oper op, Agents &ag, FormLTLK *form1)
{
    if (op == LTL_KD && ag.size() != 1) FERROR("There must be exactly one agent supplied!");
    oper = op;
    agents = ag;
    subForm[0] = form1;
}

FormLTLK::~FormLTLK(void)
{
    if (LTL_COND_1ARG(oper))
    {
        delete subForm[0];
    }

    if (LTL_COND_2ARG(oper))
    {
        delete subForm[1];
    }

    if (LTL_COND_PV(oper))
    {
        delete bdd;
    }
}


string
FormLTLK::toStr(void)
{
    switch(oper)
    {
    case LTL_PV:
    {
        return name;
        break;
    }
    case LTL_TF:
    {
        if (tf)
            return "true";
        else
            return "false";
        break;
    }
    case LTL_OR:
    {
        return "( " + subForm[0]->toStr() + " OR " + subForm[1]->toStr() + " )";
        break;
    }
    case LTL_NOT:
    {
        return "~" + subForm[0]->toStr();
        break;
    }
    case LTL_X:
    {
        return "X( " + subForm[0]->toStr() + " )";
        break;
    }
    case LTL_U:
    {
        return "( " + subForm[0]->toStr() + " U " + subForm[1]->toStr() + " )";
        break;
    }
    case LTL_KD:
    {
        std::stringstream ss;
        ss << agents[0];
        return "KD_" + ss.str() + "( " + subForm[0]->toStr() + " )";
        break;
    }
    case LTL_CD:
    {
        std::stringstream ss;
        for (unsigned int i = 0; i < agents.size(); ++i)
        {
            ss << agents[i];
            if (i+1 != agents.size()) ss << ",";
        }
        return "CD_" + ss.str() + "( " + subForm[0]->toStr() + " )";
        break;
    }
    default:
    {
        assert(0);
        return "??";
    }
    }
    assert(0);
}

void
FormLTLK::getElem(FormLTLKset &fs)
{
    switch (oper)
    {
    case LTL_PV:
    {
        fs.addForm(getCopy()); /* tutaj lepiej byłoby zwracać duplikat "this" */
        break;
    }
    case LTL_NOT:
    {
        subForm[0]->getElem(fs);
        break;
    }
    case LTL_OR:
    {
        subForm[0]->getElem(fs);
        subForm[1]->getElem(fs);
        break;
    }
    case LTL_X:
    {
        subForm[0]->getElem(fs);
        fs.addForm(getCopy());
        break;
    }
    case LTL_U:
    {
        subForm[0]->getElem(fs);
        subForm[1]->getElem(fs);
        fs.addForm(new FormLTLK(LTL_X, getCopy()));
        break;
    }
    }
}

/* Pobieranie kopii formuły */
FormLTLK *
FormLTLK::getCopy(void)
{
    FormLTLK *r = NULL;
    switch(oper)
    {
    case LTL_PV:
    {
        r = new FormLTLK(name, *bdd);
        break;
    }
    case LTL_TF:
    {
        r = new FormLTLK(tf);
        break;
    }
    case LTL_U:
    case LTL_OR:
    {
        r = new FormLTLK(oper, subForm[0]->getCopy(), subForm[1]->getCopy());
        break;
    }
    case LTL_X:
    case LTL_NOT:
    {
        r = new FormLTLK(oper, subForm[0]->getCopy());
        break;
    }
    case LTL_KD:
    case LTL_CD:
    {
        r = new FormLTLK(oper, agents, subForm[0]->getCopy());
        break;
    }
    }
    assert(r != NULL);
    return r;
}

FormLTLK *
FormLTLK::getCopyNegated(void)
{
    if (getOper() == LTL_NOT)
        return subForm[0]->getCopy();
    else
        return new FormLTLK(LTL_NOT, getCopy());
}

Oper
FormLTLK::getOper(void)
{
    return oper;
}

FormLTLK *
FormLTLK::getSubFormLeft(void)
{
    return subForm[0];
}

FormLTLK *
FormLTLK::getSubFormRight(void)
{
    return subForm[1];
}

FormLTLK **
FormLTLK::getSubFormLeft2Ptr(void)
{
    return &subForm[0];
}

FormLTLK **
FormLTLK::getSubFormRight2Ptr(void)
{
    return &subForm[1];
}

bool
FormLTLK::getTruth(void)
{
    assert(oper == LTL_TF);
    return tf;
}

BDD *
FormLTLK::getBDD(void)
{
    assert(oper == LTL_PV);
    return bdd;
}

unsigned int
FormLTLK::getAgent(void)
{
    assert(oper == LTL_KD); /* tymczasowo, żeby się nie zapomnieć */
    return agents[0];
}

void
FormLTLK::dropSubForm(void)
{
    subForm[0] = NULL;
    subForm[1] = NULL;
}

/* Reduction of double negations (which often are a by-product of using conjunction in the input, etc) */
void
FormLTLK::reduce(void)
{
    if (oper == LTL_NOT && subForm[0]->getOper() == LTL_NOT)
    {
        FormLTLK *jump = subForm[0]->getSubFormLeft(); /* the formula f after two negations, i.e., ~~f */

        subForm[0]->dropSubForm();
        delete subForm[0];
        
        subForm[0] = jump->subForm[0];
        subForm[1] = jump->subForm[1];
        oper = jump->oper;
        name = jump->name;
        if (jump->oper == LTL_PV) bdd = new BDD(*(jump->bdd));
        agents = jump->agents;
        tf = jump->tf;
        
        jump->dropSubForm();
        delete jump;

        reduce();
    }
    else
    {
        /* two cases */
        if (LTL_COND_1ARG(oper))
        {
            subForm[0]->reduce();
        }
        else if (LTL_COND_2ARG(oper))
        {
            subForm[0]->reduce();
            subForm[1]->reduce();
        }
    }
}

/*****************************************************************************************/

/* Konstruktor zbioru formuł */
FormLTLKset::FormLTLKset(void)
{
    nonPVcount = 0; /* inicjalizacje licznika podformuł które nie są zmiennymi zdaniowymi */
}

/* Dodawanie formuły do zbioru */
void
FormLTLKset::addForm(FormLTLK *form)
{
    string formStr = form->toStr();
    if (formSet.find(formStr) == formSet.end())
    {
        FormLTLKidx new_formIdx;

        new_formIdx.formula = form;
        if (form->getOper() != LTL_PV)
            new_formIdx.idx = nonPVcount++;
        else
            new_formIdx.idx = -1;

        formSet[formStr] = new_formIdx;
    }
    else
    {
        delete form;
    }
}

/* Wyświetlanie zawartości zbioru formuł */
void
FormLTLKset::show(void)
{
    for (FormLTLKstrMap::iterator ii = formSet.begin(); ii != formSet.end(); ii++)
    {
        cout << ii->first << " (" << ii->second.idx << ")";
        if (ii->second.idx == -1) cout << " (PV, no id assigned)";
        cout << endl;
    }
}

unsigned int
FormLTLKset::getNewVarsCount(void)
{
    return nonPVcount;
}

/* Szukanie indeksu formuły form (odpowiadającego indeksowi zmiennej w BDD), która dana jest argumentem */
int
FormLTLKset::getFormIdx(FormLTLK *form)
{
    FormLTLKstrMap::iterator w = formSet.find(form->toStr());
    if (w == formSet.end())
        return -1;
    else
        return w->second.idx;
}

/* Szukanie indeksu formuły która zaczyna się od operatora X i za nim stoi formuła form (argument)*/
int
FormLTLKset::getXFormIdx(FormLTLK *form)
{
    string searchedFormStr = form->toStr();
    for (FormLTLKstrMap::iterator ii = formSet.begin(); ii != formSet.end(); ii++)
    {
        if (ii->second.formula->getOper() == LTL_X)
        {
            if (ii->second.formula->getSubFormLeft()->toStr() == searchedFormStr)
            {
                return ii->second.idx;
            }
        }
    }
    return -1;
}

/* Konstruuowanie wektora wskaźników na formuły LTL które zaczynają się od operatora X */
FormLTLKvect
FormLTLKset::getAllXForm(void)
{
    FormLTLKvect r;
    for (FormLTLKstrMap::iterator ii = formSet.begin(); ii != formSet.end(); ii++)
    {
        if (ii->second.formula->getOper() == LTL_X)
        {
            r.push_back(ii->second.formula);
        }
    }
    return r;
}

/* Destruktor */
FormLTLKset::~FormLTLKset(void)
{
    for (FormLTLKstrMap::iterator ii = formSet.begin(); ii != formSet.end(); ii++)
    {
        delete ii->second.formula;
    }
}
