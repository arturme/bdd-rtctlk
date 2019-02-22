//
//  tgc.hh
//  BDD-LTLK
//
//  Created by Artur Męski on 2011-01-17.
//  Copyright 2011 VerICS Team. All rights reserved.
//

#ifndef TGC_HH_2CYF81JD
#define TGC_HH_2CYF81JD

#include "automata.hh"
#include "mchecker.hh"
#include "macro.hh"
#include "formula.hh"
#include "formula_ctlk.hh"

using namespace std;

string ccat_tgc(string a, int b);
SymAutomataNet genTGC(unsigned int nproc);

#endif /* end of include guard: TGC_HH_2CYF81JD */
