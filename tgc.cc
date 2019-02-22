// 
//  tgc.cc
//  BDD-LTLK
//  
//  Created by Artur Męski on 2011-01-17.
//  Copyright 2011 VerICS Team. All rights reserved.
// 

#include "tgc.hh"

/*
int
main(int argc, char *argv[])
{
    if (argc < 1+2)
    {
        cerr << "Usage: " << argv[0] << " <number of trains> <formula>" << endl;
        cerr << "Formulae: 1-4 (as in the paper)" << endl;
        return 1;
    }

    unsigned int nproc = atoi(argv[1]);
    
    if (nproc < 2)
    {
        cerr << "There should be at least 2 trains!" << endl;
        return 1;
    }
    
    unsigned int form_num = atoi(argv[2]);
    
    cerr << "Benchmarking for (Faulty) Train Controller" << endl;
    cerr << "Number of trains = " << nproc << endl;

#ifdef BDD_REORDER
    cout << "BDD reordering enabled" << endl;
#endif

    SymAutomataNet san = genTGC(nproc);
    
    cerr << "Network read" << endl;
    cerr << "Formula: " << flush;
    double genFormStartTime = cpuTime();
    
    FormCTLK *form_ctlk = NULL;
    if (form_num == 1)
    {
        //Cudd *cuddMgr = san.getCuddMgr();
        DEF_AGENT(ag_train0, 0);
        form_ctlk = new FormCTLK(CTLK_EF, new FormCTLK(CTLK_KD, ag_train0, new FormCTLK(CTLK_AND, new FormCTLK("InTunnel0", san.bddLocation(ccat_tgc("train_", 0), "tunnel")), new FormCTLK(CTLK_RT_EG, 1, RT_INF, new FormCTLK(CTLK_NOT, new FormCTLK("InTunnel0", san.bddLocation(ccat_tgc("train_", 0), "tunnel")))))));
    }
    else if (form_num == 2)
    {
        Cudd *cuddMgr = san.getCuddMgr();
        BDD disj = cuddMgr->bddZero();
        for (unsigned int i = 0; i < nproc; ++i)
            disj += san.bddLocation(ccat_tgc("train_", i), "tunnel");
        DEF_AGENT(ag_train0, 0);
        form_ctlk = new FormCTLK(CTLK_EF, new FormCTLK(CTLK_AND, new FormCTLK("InTunnel0", san.bddLocation(ccat_tgc("train_", 0), "tunnel")), new FormCTLK(CTLK_KD, ag_train0, new FormCTLK(CTLK_RT_EG, 1, nproc+1, new FormCTLK(CTLK_NOT, new FormCTLK("(InTunnel0 ... OR ... InTunnelN)", disj))))));

    }
    else return 2;

    assert(form_ctlk != NULL);
    
    cerr << "generated" << endl;
    double genFormTotalTime = cpuTime()-genFormStartTime;
    
    boundedCheckCTLK(san, form_ctlk);

    //getAllReach(san);
    
#ifdef MEASURE
    double howLongTotal = cpuTime();
    int64 vm = memUsed();
    cout << "BENCHMARK: encoding TIME (sec) / verification TIME (sec) / total MEMORY (MB): " << setprecision(4) << lastEncodingTime+genFormTotalTime << " ; " << howLongTotal-lastEncodingTime-genFormTotalTime << " ; " << (vm / 1048576.0) << " -- for " << "nproc=" << nproc << " form=" << form_num << endl;
#endif
#ifdef BDD_REORDER
    cout << "BDD reordering enabled" << endl;
#endif
}
*/

string
ccat_tgc(string a, int b)
{
	stringstream s;
	s << a << b;
	return s.str();
}

SymAutomataNet
genTGC(unsigned int nproc)
{
    SymAutomataNet san;
 
    for (unsigned int i = 0; i < nproc; ++i)
    {
        Automaton train;
        
        train.setName(ccat_tgc("train_", i));
        train.setAgent(i);
        train.addTransition("away", "wait", ccat_tgc("approach_", i));
        train.addTransition("wait", "tunnel", ccat_tgc("go_in_", i));
        train.addTransition("tunnel", "away", ccat_tgc("go_out_", i));
        train.setInitLocation("away");
        san.addAutomaton(train);
    }
    
    Automaton controller;
    
    controller.setName("controller");
    controller.setAgent(nproc+10);
    for (unsigned int i = 0; i < nproc; ++i)
    {
        controller.addTransition("green", "red", ccat_tgc("go_in_", i));
        controller.addTransition("red", "green", ccat_tgc("go_out_", i));        
    }
    controller.setInitLocation("green");
    san.addAutomaton(controller);
    
    san.closeNet();
    
    return san;
}

