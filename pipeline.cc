//
//  Created by Artur Męski.
//  Copyright 2012, 2013 VerICS Team. All rights reserved.
//

#include "pipeline.hh"

/*
int
main(int argc, char *argv[])
{
    if (argc < 1+2)
    {
        cerr << "Usage: " << argv[0] << " <number of nodes> <formula> [parameter]" << endl;
        cerr << "Formulae: 1-3 (as in the paper)" << endl;
        return 1;
    }

    unsigned int nproc = atoi(argv[1]);

    if (nproc < 1)
    {
        cerr << "There should be at least 1 node!" << endl;
        return 1;
    }

    unsigned int form_num = atoi(argv[2]);

    int par = 0;

    if (form_num > 9 && argc < 1+3)
    {
        cerr << "You need to specify the parameter" << endl;
        exit(-4);
    }

    if (form_num > 9) par = atoi(argv[3]);

    cerr << "Benchmarking for (Faulty) Generic Pipeline Paradigm" << endl;
    cerr << "Number of nodes = " << nproc << endl;

#ifdef BDD_REORDER
    cout << "BDD reordering enabled" << endl;
#endif

    SymAutomataNet san = genPipeline(nproc);

    cerr << "Network read" << endl;
    cerr << "Formula: " << flush;
    double genFormStartTime = cpuTime();

    FormCTLK *form_ctlk = NULL;
    if (form_num == 1)
    {
        DEF_AGENT(ag_P, 0);
        DEF_AGENT(ag_C, 1);
        int a = 2*nproc+1; assert(nproc >= 1);
        form_ctlk = new FormCTLK(CTLK_EF, new FormCTLK(CTLK_AND, new FormCTLK("ProdSend", san.bddLocation("Producer", "ProdSend")), new FormCTLK(CTLK_RT_EG, a, RT_INF, new FormCTLK(CTLK_KD, ag_C, new FormCTLK(CTLK_KD, ag_P, new FormCTLK("Received", san.bddLocation("Consumer", "Received")))))));
    }
    else if (form_num == 2)
    {
        DEF_AGENT(ag_P, 0);
        form_ctlk = new FormCTLK(CTLK_EF, new FormCTLK(CTLK_KD, ag_P, new FormCTLK(CTLK_AND, new FormCTLK("ProdSend", san.bddLocation("Producer", "ProdSend")), new FormCTLK(CTLK_RT_EF, 0, 3, new FormCTLK("Received", san.bddLocation("Consumer", "Received"))))));
    }
    else if (form_num == 3)
    {
        DEF_AGENT(ag_P, 0);
        form_ctlk = new FormCTLK(CTLK_EF, new FormCTLK(CTLK_KD, ag_P, new FormCTLK(CTLK_AND, new FormCTLK("ProdSend", san.bddLocation("Producer", "ProdSend")), new FormCTLK(CTLK_RT_EF, nproc, nproc+3, new FormCTLK("Received", san.bddLocation("Consumer", "Received"))))));
    }
    else if (form_num == 10)
    {
        form_ctlk = new FormCTLK(CTLK_EG, new FormCTLK(CTLK_OR, new FormCTLK(CTLK_NOT, new FormCTLK("ProdSend", san.bddLocation("Producer", "ProdSend"))), new FormCTLK(CTLK_RT_EG, 0, par-1, new FormCTLK(CTLK_NOT, new FormCTLK("ConsReceived", san.bddLocation("Consumer", "Received"))))));
    }


    cerr << "generated" << endl;
    double genFormTotalTime = cpuTime()-genFormStartTime;

    if (form_num == 0)
    {
        getAllReach(san);
    }
    else
    {
        assert(form_ctlk != NULL);
        boundedCheckCTLK(san, form_ctlk);
    }

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
ccat_pipe(string a, int b)
{
  stringstream s;
  s << a << b;
  return s.str();
}

SymAutomataNet
genPipeline(unsigned int nodes)
{
  SymAutomataNet san;

  Automaton producer;
  producer.setName("Producer");
  producer.setAgent(0);
  producer.addTransition("ProdReady", "ProdSend", "producing");
  producer.addTransition("ProdSend", "ProdReady", "send_1");
  producer.setInitLocation("ProdReady");
  san.addAutomaton(producer);

  Automaton consumer;
  consumer.setName("Consumer");
  consumer.setAgent(1);
  consumer.addTransition("ConsReady", "Received", ccat_pipe("send_",
                         nodes + 1));
  consumer.addTransition("Received", "ConsReady", "consuming");
  consumer.setInitLocation("ConsReady");
  san.addAutomaton(consumer);

  for (unsigned int i = 1; i <= nodes; ++i) {
    Automaton node;
    node.setName(ccat_pipe("Node_", i + 1));
    node.setAgent(i + 1);
    node.addTransition("Ready", "Proc", ccat_pipe("send_", i));
    node.addTransition("Proc", "Send", ccat_pipe("processing_", i));
    node.addTransition("Send", "Ready", ccat_pipe("send_", i + 1));
    node.setInitLocation("Ready");
    san.addAutomaton(node);
  }

  san.closeNet();

  return san;
}
