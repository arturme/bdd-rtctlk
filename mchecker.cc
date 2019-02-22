//
//  mchecker.cc
//  BDD-LTLK
//
//  Created by Artur Meski on 2010-10-14.
//  Copyright 2010 VerICS Team. All rights reserved.
//

#include "mchecker.hh"

/**
 * @brief Non-bounded LTLK model checking
 * @param san Network of automata to be verified
 * @param form LTLK formula to be verified
 * @return true if the supplied LTLK formula holds, false otherwise
 */
bool
checkLTLK(SymAutomataNet &san, FormLTLK *form)
{
  form->reduce();
  cout << endl << "Verification of LTLK formula " << form->toStr() << endl;

  mCheckerSetup(&san);

  paramsLTLmemSetup();

  /* Trzeba obliczyć wszystkie stany osiągalne */
  vector<BDD> *pv = san.getPVvector();
  vector<BDD> *pv_succ = san.getPVsuccVector();
  BDD *exist_pv = san.getPVexist();
  BDD init = san.bddInitLoc();
  BDD trans = *(san.bddTrans());

  BDD reach = init;
  BDD reach_p = cuddMgr->bddZero();
  BDD next;

  while (reach != reach_p) {
    next = reach * trans;
    next = next.ExistAbstract(*exist_pv);
    next = next.SwapVariables(*pv_succ, *pv);
    reach_p = reach;
    reach += next;
  }

  /* reach zawiera wszystkie stany osiągalne */

  VerTreeLTLK vtree(form);

  FormLTLK *form_pick;

  while ((form_pick = vtree.pickUnproc()) != NULL) {
#ifdef VERBOSE
    cout << "Processing K-subformula: " << form_pick->toStr() << endl;
#endif

    /* zastępowanie K-podfomuł zmiennymi zdaniowymi */
    FormLTLK *simple_form = form_pick->getCopy();
    assert(simple_form->getOper() == LTL_KD);
    simplifyLTLK(simple_form->getSubFormLeft2Ptr(), vtree);

#ifdef VERBOSE
    cout << "Checking simplified K-subformula: " << simple_form->toStr() << endl;
#endif
    vtree.loadLastPickParams();
    BDD verResult = satStates_K(simple_form, reach);

    vtree.saveWithLastPick(verResult);
  }

#ifdef BDD_VERBOSE
  vtree.show();
#endif

  FormLTLK *final_LTL = form->getCopy();
  simplifyLTLK(&final_LTL, vtree);

#ifdef VERBOSE
  cout << "Checking final LTL formula (obtained from LTLK formula): " <<
       final_LTL->toStr() << endl;
#endif

  paramsLTLload(paramsLTLsetup(final_LTL));

#ifdef BDD_VERBOSE
  cout << "Reach BDD:" << endl;
  reach.PrintMinterm();
#endif

  bool result;

  if (states_LTL_Ex() * san.bddInitLoc() != cuddMgr->bddZero()) {
    cout << "RESULT: formula Ef (where f=" << form->toStr() << ") holds" << endl;
    result = true;
  }
  else {
    cout << "RESULT: formula Ef (where f=" << form->toStr() << ") does not hold"
         << endl;
    result = false;
  }

  paramsLTLmemCleanup();
  mCheckerUnSetup();

  return result;
}

/* Weryfikacja egzystencjalnych formuł postaci Ef */
bool
checkLTLK_Ex(SymAutomataNet &san, FormLTLK *form)
{
  bool result = checkLTLK(san, form);

  if (result == true) {
    cout << "FINAL RESULT: Ef is TRUE (where f=" << form->toStr() << ")" << endl;
  }
  else {
    cout << "FINAL RESULT: Ef is FALSE (where f=" << form->toStr() << ")" <<
         endl;
  }

  return result;
}

/* Weryfikacja uniwersalnych formuł postaci Af */
bool
checkLTLK_Un(SymAutomataNet &san, FormLTLK *form)
{
  FormLTLK *nform = form->getCopyNegated();
  bool result = !checkLTLK(san, form);

  if (result) {
    cout << "FINAL RESULT: Af is TRUE (where f=" << form->toStr() << ")" << endl;
  }
  else {
    cout << "FINAL RESULT: Af is FALSE (where f=" << form->toStr() << ")" <<
         endl;
  }

  delete nform;
  return result;
}

bool
boundedCheckLTLK(SymAutomataNet &san, FormLTLK *form)
{
  form->reduce();
  cerr << "Bounded verification of LTLK formula " << form->toStr() << endl;
  mCheckerSetup(&san);

  int finalLTLparams = -1;
  paramsLTLmemSetup();

  bool result = false;
  bool done = false;
  bool checkLoop = false; // !!!
  VerTreeLTLK vtree(
    form); /* tutaj robi się ParamsLTLwSAN dla każdej podfomuły z vtree */

  vector<BDD> *pv = san.getPVvector();
  vector<BDD> *pv_succ = san.getPVsuccVector();
  BDD *exist_pv = san.getPVexist();
  BDD init = san.bddInitLoc();
  BDD trans = *(san.bddTrans());

  BDD reach = init;
  BDD reach_p = cuddMgr->bddZero();
  BDD next = cuddMgr->bddZero();
  BDD new_states = init;

#ifdef VERBOSE
  cout << "Initial state: ";
  san.showBddStates(init);
#endif

  int k = 0;

  while (!done) { /* petla dziala tak dlugo jak wynik nie bedzie sfalsyfikowany */
#ifdef VERBOSE
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ k = " << k << endl;
    cout << "BDD for reachable states";
    reach.print(san.getPVnvars(), 1); /* printing the number of BDD nodes */
#endif

#ifndef VERBOSE
    cerr << "\rk=" << k << std::flush;
#endif

#ifdef BDD_REORDER
    Cudd_ReduceHeap(cuddMgr->getManager(), CUDD_REORDER_SIFT, 100000);
#endif

    // cout << "reordered BDD for reachable states";
    // reach.print(san.getPVnvars(),1); /* printing the number of BDD nodes */

    /* rozbudowujemy przestrzeń stanów */
    next = new_states * trans;
    next = next.ExistAbstract(*exist_pv);
    next = next.SwapVariables(*pv_succ, *pv);
#ifdef VERBOSE
    cout << "Computed next-states; BDD";
    next.print(san.getPVnvars(), 1);
#endif

    if (!checkLoop || (next * reach != cuddMgr->bddZero())) {
#ifdef VERBOSE
      cout << "Running LTLK check!" << endl;
#endif
      restrictTrans(trans, reach);

#ifdef VERBOSE
      cout << "BDD for reachable states";
      reach.print(san.getPVnvars(), 1); /* printing the number of BDD nodes */
#endif
      vtree.cleanup();
      FormLTLK *form_pick;

      while ((form_pick = vtree.pickUnproc()) != NULL) {
#ifdef VERBOSE
        cout << "Processing K-subformula: " << form_pick->toStr() << endl;
#endif
        // Wszystkie K-podformuły muszą być zastąpione zmiennymi zdaniowymi
        // takimi, że są one prawdziwe wszędzie tam, gdzie te K-podformuły
        // są prawdziwe.
        FormLTLK *simple_form = form_pick->getCopy();
        assert(simple_form->getOper() == LTL_KD || simple_form->getOper() == LTL_CD);
        simplifyLTLK(simple_form->getSubFormLeft2Ptr(), vtree);

#ifdef VERBOSE
        cout << "Checking simplified K-subformula: " << simple_form->toStr() << endl;
#endif

        /* tutaj trzeba przywołać odpowiednie paramsLTLwSAN, przed wywołaniem satStates_K(), bo tam jest ukryta weryfikacja LTL */
        vtree.loadLastPickParams();

        BDD verResult = satStates_K(simple_form, reach);
        vtree.saveWithLastPick(verResult);

        paramsLTLwSAN = NULL; /* tylko do asercji, na razie... */
      }

#ifdef BDD_VERBOSE
      vtree.show();
#endif
      FormLTLK *final_LTL = form->getCopy();
      simplifyLTLK(&final_LTL, vtree);
#ifdef VERBOSE
      cout << "Checking final LTL formula (obtained from LTLK formula): " <<
           final_LTL->toStr() << endl;
#endif

      if (finalLTLparams == -1) {
        finalLTLparams = paramsLTLsetup(final_LTL);
      }

      paramsLTLload(finalLTLparams);
      paramsLTLformUpdate(final_LTL);
#ifdef BDD_VERBOSE
      cout << "Reach BDD:" << endl;
      reach.PrintMinterm();
#endif

      if (states_LTL_Ex() * san.bddInitLoc() != cuddMgr->bddZero()) {
#ifdef VERBOSE
        cout << "-> Ef holds, where f = " << form->toStr() << endl;
#endif
        result = true;
        done = true;
        k++;
        break;
      }
      else {
        assert(result != true);
#ifdef VERBOSE
        cout << "-> Ef does not hold (yet?), where f = " << form->toStr() << endl;
#endif
      }
    } // checkLoop

    new_states = next - reach;
    reach_p = reach;

    if (new_states ==
        cuddMgr->bddZero()) { /* are there any undiscovered states? */
      done = true;  /* fixed-point reached */
    }
    else {
      reach += new_states;
    }

#ifdef VERBOSE
    cout << "BDD for new_states";
    new_states.print(san.getPVnvars(), 1);
#endif

    k++;
  }

  // san.showBddStates(reach);

#ifdef VERBOSE
  cout << "Formula Ef (where f=" << form->toStr() << ") is " <<
       (result ? "true" : "false") << endl;
#endif

  paramsLTLmemCleanup();
  mCheckerUnSetup();

  // san.showBddStates(reach);

#ifndef VERBOSE
  cerr << endl;
#endif

  cout << "Terminated at k=" << k - 1;

  if (next - reach == cuddMgr->bddZero()) {
    cout << " (with full state space)" << endl;
  }
  else {
    cout << " (with partial state space)" << endl;
  }

  cout << "BDD for reachable states";
  reach.print(san.getPVnvars(), 1); /* printing the number of BDD nodes */

  return result;
}

/* Weryfikacja egzystencjalnych formuł postaci Ef */
bool
boundedCheckLTLK_Ex(SymAutomataNet &san, FormLTLK *form)
{
  bool result;

  if (boundedCheckLTLK(san, form) == true) {
    cout << "FINAL RESULT: Ef is TRUE (where f=" << form->toStr() << ")" << endl;
    result = true;
  }
  else {
    cout << "FINAL RESULT: Ef is FALSE (where f=" << form->toStr() << ")" << endl;
    result = false;
  }

  return result;
}

/* Weryfikacja uniwersalnych formuł postaci Af */
bool
boundedCheckLTLK_Un(SymAutomataNet &san, FormLTLK *form)
{
  bool result;

  FormLTLK *nform = form->getCopyNegated();

  if (boundedCheckLTLK(san, nform) == false) {
    cout << "FINAL RESULT: Af is TRUE (where f=" << form->toStr() << ")" << endl;
    result = true;
  }
  else {
    cout << "FINAL RESULT: Af is FALSE (where f=" << form->toStr() << ")" << endl;
    result = false;
  }

  delete nform;
  return result;
}

bool
boundedCheckCTLK_Loop(SymAutomataNet &san, FormCTLK *form)
{
  cerr << "Bounded verification of CTLK formula (with loops checking) " <<
       form->toStr() << endl;

  mCheckerSetup(&san);

  vector<BDD> *pv = san.getPVvector();
  vector<BDD> *pv_succ = san.getPVsuccVector();
  BDD *exist_pv = san.getPVexist();
  BDD init = san.bddInitLoc();
  BDD trans = *(san.bddTrans());

  BDD reach = init;
  BDD reach_p = cuddMgr->bddZero();
  BDD next = cuddMgr->bddZero();
  BDD new_states = init;
  BDD form_states;

  bool checkLoop = form->hasOper(CTLK_EG);
  bool result = false;
  bool done = false;

  int k = 0;

  while (!done) {
#ifdef VERBOSE
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ k = " << k << endl;
#endif

#ifndef VERBOSE
    cerr << "\rk=" << k << std::flush;
#endif

#ifdef BDD_REORDER
    Cudd_ReduceHeap(cuddMgr->getManager(), CUDD_REORDER_SIFT, 100000);
#endif

    next = new_states * trans;
    next = next.ExistAbstract(*exist_pv);
    next = next.SwapVariables(*pv_succ, *pv);

    if (!checkLoop || (next * reach != cuddMgr->bddZero())) {
      /* CTLK check */
      form_states = form->satStates(reach);

      if (init * form_states != cuddMgr->bddZero()) {
        result = true;
        done = true;
        form->cleanup();
        k++;
        break;
      }
    }

    new_states = next - reach;

    // reach_p = reach;
    if (new_states ==
        cuddMgr->bddZero()) { /* are there any undiscovered states? */
      done = true;  /* fixed-point reached */
    }
    else {
      reach += new_states;
    }

    form->cleanup();

    k++;
  }

  mCheckerUnSetup();

#ifndef VERBOSE
  cerr << endl;
#endif


  cout << "Terminated at k=" << k - 1;

  if (next - reach == cuddMgr->bddZero()) {
    cout << " (with full state space)" << endl;
  }
  else {
    cout << " (with partial state space)" << endl;
  }

  cout << "BDD for reachable states";
  reach.print(san.getPVnvars(), 1); /* printing the number of BDD nodes */

  cout << "FINAL RESULT: " << form->toStr() << " is " <<
       (result ? "true" : "false") << endl;

  return result;
}

bool
boundedCheckCTLK(SymAutomataNet &san, FormCTLK *form)
{
  cerr << "Bounded verification of ECTLK formula " << form->toStr() << endl;

  if (!form->isECTLK()) {
    FERROR("Formula " << form->toStr() <<
           " is not syntactically an ECTLK formula");
  }

  mCheckerSetup(&san);

  vector<BDD> *pv = san.getPVvector();
  vector<BDD> *pv_succ = san.getPVsuccVector();
  BDD *exist_pv = san.getPVexist();
  BDD init = san.bddInitLoc();
  BDD trans = *(san.bddTrans());

  BDD reach = init;
  BDD reach_p = cuddMgr->bddZero();
  BDD next = cuddMgr->bddZero();

  bool result = false;

  int k = 0;

  while (reach != reach_p) {
    VERB("k = " << k);

    if (opts.it_reorder) {
      Cudd_ReduceHeap(cuddMgr->getManager(), CUDD_REORDER_SIFT, 100000);
    }

    /* CTLK check */
    if (init * form->satStates(reach) != cuddMgr->bddZero()) {
      result = true;
      form->cleanup();
      k++;
      break;
    }

    next = reach * trans;
    next = (next.ExistAbstract(*exist_pv)).SwapVariables(*pv_succ, *pv);
    reach_p = reach;
    reach += next;

    form->cleanup();

    k++;
  }

  mCheckerUnSetup();

  cout << "Terminated at k=" << k - 1;

  if (reach == reach_p) {
    cout << " (with full state space)" << endl;
  }
  else {
    cout << " (with partial state space)" << endl;
  }

  cout << "BDD for reachable states";
  reach.print(san.getPVnvars(), 1); /* printing the number of BDD nodes */

  cout << "FINAL RESULT: " << form->toStr() << " is " <<
       (result ? "true" : "false") << endl;

  return result;
}

bool
checkCTLK(SymAutomataNet &san, FormCTLK *form)
{
  cerr << "Non-bounded verification of CTLK formula " << form->toStr() << endl;

  mCheckerSetup(&san);

  vector<BDD> *pv = san.getPVvector();
  vector<BDD> *pv_succ = san.getPVsuccVector();
  BDD *exist_pv = san.getPVexist();
  BDD init = san.bddInitLoc();
  BDD trans = *(san.bddTrans());

  BDD reach = init;
  BDD reach_p = cuddMgr->bddZero();
  BDD next = cuddMgr->bddZero();

  bool result = false;

  int k = 0;

  while (reach != reach_p) {
    VERB("k = " << k);

    if (opts.it_reorder) {
      Cudd_ReduceHeap(cuddMgr->getManager(), CUDD_REORDER_SIFT, 100000);
    }

    next = reach * trans;
    next = (next.ExistAbstract(*exist_pv)).SwapVariables(*pv_succ, *pv);
    reach_p = reach;
    reach += next;

    k++;
  }

  /* CTLK check */
  if (init * form->satStates(reach) != cuddMgr->bddZero()) {
    result = true;
    form->cleanup();
  }

  mCheckerUnSetup();

  cout << "Terminated at k=" << k - 1;

  if (next - reach == cuddMgr->bddZero()) {
    cout << " (with full state space)" << endl;
  }
  else {
    cout << " (with partial state space)" << endl;
  }

  cout << "BDD for reachable states";
  reach.print(san.getPVnvars(), 1); /* printing the number of BDD nodes */

  cout << "FINAL RESULT: " << form->toStr() << " is " <<
       (result ? "true" : "false") << endl;

  return result;
}


void
getAllReach(SymAutomataNet &san)
{
  VERB("Generating reachable states");

  mCheckerSetup(&san);

  vector<BDD> *pv = san.getPVvector();
  vector<BDD> *pv_succ = san.getPVsuccVector();
  BDD *exist_pv = san.getPVexist();
  BDD init = san.bddInitLoc();
  BDD trans = *(san.bddTrans());

  BDD reach = init;
  BDD reach_p = cuddMgr->bddZero();
  BDD next = cuddMgr->bddZero();

  while (reach != reach_p) {
    next = reach * trans;
    next = (next.ExistAbstract(*exist_pv)).SwapVariables(*pv_succ, *pv);
    reach_p = reach;
    reach += next;
  }

  san.showBddStates(reach);

  mCheckerUnSetup();

}
