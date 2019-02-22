// 
//  formula.hh
//  BDD-LTLK
//  
//  Created by Artur Meski on 2010-10-14.
//  Copyright 2010 VerICS Team. All rights reserved.
// 

#ifndef FORMULA_HH_1A3ODT83
#define FORMULA_HH_1A3ODT83

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <cstdlib>
#include "macro.hh"
#include "cudd/include/cuddObj.hh"
#include "bdd_macro.hh"

#define LTL_PV 0
#define LTL_NOT 1
#define LTL_OR 2
#define LTL_U 3
#define LTL_X 4
#define LTL_TF 50
#define LTL_KD 70
#define LTL_CD 71

#define LTLF_F(a) new FormLTLK(LTL_U, new FormLTLK(true), (a))
#define LTLF_G(a) new FormLTLK(LTL_NOT, new FormLTLK(LTL_U, new FormLTLK(true), new FormLTLK(LTL_NOT, (a))))
#define LTLF_X(a) new FormLTLK(LTL_X, (a))
#define LTLF_NOT(a) new FormLTLK(LTL_NOT, (a))
#define LTLF_AND(a,b) new FormLTLK(LTL_NOT, new FormLTLK(LTL_OR, new FormLTLK(LTL_NOT, (a)), new FormLTLK(LTL_NOT, (b))))
#define LTLF_OR(a, b) new FormLTLK(LTL_OR, (a), (b))
#define LTLF_IMPL(a,b) new FormLTLK(LTL_OR, new FormLTLK(LTL_NOT, (a)), (b))

using std::string;
using std::cout;
using std::endl;

class FormLTLK;
class FormLTLKset;

typedef unsigned char Oper;
typedef unsigned char OperSub;
typedef std::vector<unsigned int> Agents;
class FormLTLK
{
    Oper oper;
    FormLTLK *subForm[2];
    string name;
    BDD *bdd;
    Agents agents;
    bool tf;
    bool isExistentialFormula;
public:
    FormLTLK(string varName, BDD varBDD);
    FormLTLK(Oper op, FormLTLK *form1);
    FormLTLK(Oper op, FormLTLK *form1, FormLTLK *form2);
    FormLTLK(FormLTLK *form1, Oper op, FormLTLK *form2);
    FormLTLK(Oper op, Agents &agents, FormLTLK *form1);
    FormLTLK(bool tf);
    ~FormLTLK(void);
    void getElem(FormLTLKset &fs);
    FormLTLK *getCopy(void);
    FormLTLK *getCopyNegated(void);
    string toStr(void);
    Oper getOper(void);
    FormLTLK *getSubFormLeft(void);
    FormLTLK *getSubFormRight(void);
    FormLTLK **getSubFormLeft2Ptr(void);
    FormLTLK **getSubFormRight2Ptr(void);
    bool getTruth(void);
    BDD *getBDD(void);
    unsigned int getAgent(void);
	Agents *getAgentsPtr(void)
	{
		return &agents;
	}
    void dropSubForm(void);
	void reduce(void);
    void setExistential(bool existential)
    {
        isExistentialFormula = existential;
    }    
    bool isExistential()
    {
        return isExistentialFormula;
    }
};

typedef std::vector<FormLTLK *> FormLTLKvect;

class FormLTLKset
{
    struct FormLTLKidx
    {
        FormLTLK *formula;
        int idx;
    };
    typedef std::map<string, FormLTLKidx> FormLTLKstrMap;
    unsigned int nonPVcount;
    FormLTLKstrMap formSet;
public:
    FormLTLKset(void);
    void addForm(FormLTLK *form);
    void show(void);
    unsigned int getNewVarsCount(void);
    int getFormIdx(FormLTLK *form);
    int getXFormIdx(FormLTLK *form);
    FormLTLKvect getAllXForm(void);
    ~FormLTLKset(void);
};

#define LTL_COND_PV(a) ((a) == LTL_PV)
#define LTL_COND_1ARG(a) ((a) == LTL_NOT || (a) == LTL_X || (a) == LTL_KD || (a) == LTL_CD)
#define LTL_COND_2ARG(a) ((a) == LTL_U || (a) == LTL_OR)

using std::cout;

#endif /* end of include guard: FORMULA_HH_1A3ODT83 */
