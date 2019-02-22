//
//  mchecker.hh
//  BDD-LTLK
//
//  Created by Artur Meski on 2010-10-14.
//  Copyright 2010 VerICS Team. All rights reserved.
//

#ifndef MCHECKER_HH_JWJDONJ1
#define MCHECKER_HH_JWJDONJ1

#include <iostream>
#include <assert.h>
#include "bdd_macro.hh"
#include "cudd/include/cuddObj.hh"
#include "automata.hh"
#include "formula.hh"
#include "formula_ctlk.hh"
#include "mctools.hh"

using std::cout;
using std::cerr;
using std::endl;

bool checkLTLK(SymAutomataNet &san, FormLTLK *form);
bool checkLTLK_Ex(SymAutomataNet &san, FormLTLK *form);
bool checkLTLK_Un(SymAutomataNet &san, FormLTLK *form);
bool boundedCheckLTLK(SymAutomataNet &san, FormLTLK *form);
bool boundedCheckLTLK_Ex(SymAutomataNet &san, FormLTLK *form);
bool boundedCheckLTLK_Un(SymAutomataNet &san, FormLTLK *form);
bool boundedCheckCTLK_Loop(SymAutomataNet &san, FormCTLK *form);
bool boundedCheckCTLK(SymAutomataNet &san, FormCTLK *form);
bool checkCTLK(SymAutomataNet &san, FormCTLK *form);
void getAllReach(SymAutomataNet &san);

#endif /* end of include guard: MCHECKER_HH_JWJDONJ1 */

