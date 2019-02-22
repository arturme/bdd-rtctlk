//
//  pipeline.hh
//  BDD-LTLK
//
//  Created by Artur Męski on 2011-01-17.
//  Copyright 2011 VerICS Team. All rights reserved.
//

#ifndef PIPELINE_HH_FNQUVN9L
#define PIPELINE_HH_FNQUVN9L

#include "automata.hh"
#include "mchecker.hh"
#include "macro.hh"
#include "formula.hh"
#include "formula_ctlk.hh"

using namespace std;

string ccat_pipe(string a, int b);
SymAutomataNet genPipeline(unsigned int nodes);

#endif /* end of include guard: PIPELINE_HH_FNQUVN9L */
