//
//  automata.cc
//  BDD-LTLK
//
//  Created by Artur Meski on 2010-10-14.
//  Copyright 2010 VerICS Team. All rights reserved.
//

#include "automata.hh"

Automaton::Automaton(void)
{
  automatonName = "unnamed";
  initLocation_defined = false;
  agent_defined = false;
}

void
Automaton::setName(string name)
{

  ASSUME_NOT_EMPTY(name, "automaton name");

  automatonName = name;
}

void
Automaton::addLocation(string name)
{
  ASSUME_NOT_EMPTY(name, "location name");

  if (hasLocation(name)) {
    cout << "Duplicated location. Ignored!" <<
         endl; // tutaj powinien być FERROR!
  }
  else {
    unsigned int nextId = locations.size();
    locations[name] = nextId;
  }
}

void
Automaton::addGreenLocation(string name)
{
  ASSUME_NOT_EMPTY(name, "location name");

  addLocation(name);
  setLocationGreen(name);
}

void
Automaton::setLocationGreen(string name)
{
  ASSUME_NOT_EMPTY(name, "location name");

  // !_!_!_!_!_!_!_!_!_!_!_!_!
  //    DOPISAC
  // !_!_!_!_!_!_!_!_!_!_!_!_!
}

bool
Automaton::hasLocation(string name)
{
  ASSUME_NOT_EMPTY(name, "location name");

  if (locations.find(name) == locations.end()) {
    return false;
  }

  return true;
}

void
Automaton::showLocations(void)
{
  cout << "Locations:" << endl;

  for (Locations::iterator ii = locations.begin(); ii != locations.end();
       ++ii) {
    cout << "\t" << ii->first << " (" << ii->second << ")" << endl;
  }

  cout << "Initial location: " << (initLocation_defined ? getLocationName(
                                     initLocationId) : "not defined!") << endl;
}

unsigned int
Automaton::countLocations(void)
{
  return locations.size();
}

Automaton::LocationId
Automaton::getLocationId(string name) const
{
#ifdef AUT_VERBOSE
  cout << "getLocationId(): name=" << name << endl;
#endif
  // assert(locations.find(name) != locations.end());
  return locations.at(name);
}

string
Automaton::getLocationName(LocationId locId)
{
  for (Locations::iterator ii = locations.begin(); ii != locations.end();
       ++ii) {
    if (ii->second == locId) {
      return ii->first;
    }
  }

  FERROR("Location not found!");
}

void
Automaton::setInitLocation(string name)
{
  ASSUME_NOT_EMPTY(name, "initial location name");

  if (!hasLocation(name)) {
    addLocation(name);
  }

  initLocation_defined = true;
  initLocationId = getLocationId(name);
}

void
Automaton::addTransition(string srcLocName, string dstLocName, Action action)
{
  ASSUME_NOT_EMPTY(srcLocName, "source location name");
  ASSUME_NOT_EMPTY(dstLocName, "destination location name");
  ASSUME_NOT_EMPTY(action, "transition name");

  if (!hasLocation(srcLocName)) {
    addLocation(srcLocName);
  }

  if (!hasLocation(dstLocName)) {
    addLocation(dstLocName);
  }

  TransitionDst new_transDst;
  new_transDst.action = action;
  new_transDst.dstLoc = getLocationId(dstLocName);
  transitions[getLocationId(srcLocName)].push_back(new_transDst);

  TransSrcDst new_transSrcDst;
  new_transSrcDst.srcLoc = getLocationId(srcLocName);
  new_transSrcDst.dstLoc = getLocationId(dstLocName);
  transByActions[action].push_back(new_transSrcDst);

  actions.insert(action);
}


void
Automaton::addValuation(string locName, string varName)
{
  valuations.insert(std::make_pair(varName, locName));
}

void
Automaton::showTransitions(void)
{
  cout << "Transitions:" << endl;

  for (Transitions::iterator ii = transitions.begin(); ii != transitions.end();
       ++ii) {
    for (TransitionDsts::iterator jj = ii->second.begin(); jj != ii->second.end();
         ++jj) {
      cout << "\t" << getLocationName(ii->first) << " --[ " << jj->action <<
           " ]--> " << getLocationName(jj->dstLoc) << endl;
    }
  }
}

// bool
// Automaton::hasLabel(Action action)
// {
//  /* Dość niefektywny sposób. Jak będzie boleć, to się poprawi... */
//  /* Można dodać set<string> i zapisywać etykiety. Potem szukać za pomocą metody find. */
//  for (Transitions::iterator ii = transitions.begin(); ii != transitions.end(); ii++)
//  {
//    for (TransitionDsts::iterator jj = ii->second.begin() ; jj != ii->second.end(); jj++)
//    {
//      if (action == jj->action) return true;
//    }
//  }
//
//  return false;
// }

void
Automaton::showActions(void)
{
  cout << "Actions:" << endl;

  for (Actions::iterator act = actions.begin(); act != actions.end(); ++act) {
    cout << "\t" << *act << endl;
  }
}

void
Automaton::setAgent(AgentId agent)
{
  agentId = agent;
  agent_defined = true;
}

bool
SymAutomataNet::isAutInAgent(Automaton::AgentId agent_id, string automaton)
{
  AutIdx aut = getAutomatonId(automaton);

  for (VectAutIdx::iterator cur_aut = agentAutomata[agent_id].begin();
       cur_aut != agentAutomata[agent_id].end();
       ++cur_aut) {
    if (*cur_aut == aut) {
      return true;
    }
  }

  return false;
}

void
SymAutomataNet::makeAutAgent(Automaton::AgentId agent_id, string automaton)
{
  if (isAutInAgent(agent_id, automaton)) {
    cout << "WARNING: Not adding duplicated automaton \"" << automaton <<
         "\" to the agent " << agent_id << "!" << endl;
  }
  else {
    agentAutomata[agent_id].push_back(getAutomatonId(automaton));
  }
}

void
SymAutomataNet::showAgents(void)
{
  cout << "Agents:" << endl;

  for (AgentAutomata::iterator ag = agentAutomata.begin();
       ag != agentAutomata.end(); ++ag) {
    cout << "agent ID=" << ag->first << " consists of ";

    for (VectAutIdx::iterator aut = ag->second.begin(); aut != ag->second.end();
         ++aut) {
      cout << *aut << " ";
    }

    cout << endl;
  }
}

void
Automaton::show(void)
{
  cout << "Name: " << automatonName << endl;

  if (agent_defined) {
    cout << "\t(part of agent with ID=" << agentId << ")" << endl;
  }
  else {
    cout << "\t(agent not defined!)" << endl;
  }

  showLocations();
  showTransitions();
  showActions();
}

SymAutomataNet::SymAutomataNet(void)
{
  cuddMgr = new Cudd(0, 0);
  netIsClosed = false;
  trans_bdd = NULL;
}

Cudd *
SymAutomataNet::getCuddMgr(void)
{
  return cuddMgr;
}

void SymAutomataNet::addValuationForAllAutomata()
{
  for (unsigned int i = 0; i < automata.size(); ++i) {
#ifdef INPUT_VERBOSE
    cout << "adding valuation for automaton " << i << endl;
#endif
    const Automaton &a = automata.at(i);
    const std::map<string, string> &valuationsForAutomaton = a.getValuations();
    addValuationsForAutomaton(a.automatonName, valuationsForAutomaton);
  }
}

/*
 * Zamykanie sieci
 *
 * Po dodaniu wszystkich składowych należy wywołać tę funkcję.
 * Zostaną ustawione wszystkie zmienne dla BDD.
 */
void
SymAutomataNet::closeNet(void)
{
  if (!netIsClosed) {
    pv_nvars = 0;

    /* zliczamy liczbę potrzebych zmiennych */
    for (unsigned int i = 0; i < automata.size(); ++i) {
      unsigned int bitCount =
        0; /* ile bitów potrzeba do zapisania wszystkich stanów i-tego automatu */
      unsigned int bitCountMaxVal = 1;

      while (bitCountMaxVal <
             automata[i].locations.size()) { // czy nie powinno być <= ??? - co się dzieje jak jest tylko jedna lokacja?
        bitCount++;
        bitCountMaxVal *= 2;
      }

      BDDenc new_bddenc;
      new_bddenc.numVars = bitCount;
      new_bddenc.pv = new vector<BDD>(bitCount);
      new_bddenc.pv_succ = new vector<BDD>(bitCount);
      new_bddenc.pv_E = new BDD(cuddMgr->bddOne());
      new_bddenc.pv_succ_E = new BDD(cuddMgr->bddOne());
      autBDDenc.push_back(new_bddenc);

      pv_nvars += bitCount;
    }

    /* przygotowujemy wektory pv i pv_succ dla produktu */
    pv = new vector<BDD>(pv_nvars);
    pv_succ = new vector<BDD>(pv_nvars);

    /* BDD do kwantyfikacji egzystencjalnej */
    pv_E = new BDD(cuddMgr->bddOne());
    pv_succ_E = new BDD(cuddMgr->bddOne());

    unsigned int used = 0;

    for (unsigned int i = 0; i < automata.size(); ++i) {
      unsigned int bitCount = autBDDenc[i].numVars;
      autBDDenc[i].varBegin =
        used; /* zapamiętujemy indeks początkowej zmiennej */

      for (unsigned int j = 0; j < bitCount; ++j) {
        unsigned int k = used + j;
        (*pv)[k] = cuddMgr->bddVar(k * 2);
        (*pv_succ)[k] = cuddMgr->bddVar((k * 2) + 1);

        *pv_E *= (*pv)[k];
        *pv_succ_E *= (*pv_succ)[k];

        (*(autBDDenc[i].pv_E)) *= (*pv)[k];
        (*(autBDDenc[i].pv_succ_E)) *= (*pv_succ)[k];

        (*(autBDDenc[i].pv))[j] = cuddMgr->bddVar(k * 2);
        (*(autBDDenc[i].pv_succ))[j] = cuddMgr->bddVar((k * 2) + 1);
      }

      used += bitCount;
    }
  }

  netIsClosed = true;
}

void
SymAutomataNet::addAutomaton(Automaton automaton)
{
  if (automataByName.find(automaton.automatonName) != automataByName.end()) {
    FERROR("Duplicated automaton (name)!");
  }

  if (!automaton.initLocation_defined) {
    FERROR("Initial location must be defined! Not adding this automaton!");
  }

  // We do not force the agent to be defined - it can be done later
  //if (!automaton.agent_defined)
  //{
  //   FERROR("Agent not defined, use setAgent()");
  //}

  automata.push_back(automaton);
  AutIdx newAutomatonIdx = automata.size() - 1;
  automataByName[automaton.automatonName] = newAutomatonIdx;

  if (automaton.agent_defined) {
    agentAutomata[automaton.agentId].push_back(newAutomatonIdx);
  }

  /* rejestrujemy akcje... */
  for (Automaton::Actions::iterator act = automaton.actions.begin();
       act != automaton.actions.end(); ++act) {
    actionsToAut[*act].push_back(
      newAutomatonIdx); /* ... tzn. zapamiętujemy, które automaty posiadają daną akcję */
  }
}

void
SymAutomataNet::showActions(void)
{
  cout << "Net-actions:" << endl;

  for (ActionsToAut::iterator act = actionsToAut.begin();
       act != actionsToAut.end(); ++act) {
    cout << "\t" << act->first;

    if (act->second.size() == 1) {
      cout << " (local)" << endl;
    }
    else {
      cout << " (sync)" << endl;
    }
  }
}

unsigned int
SymAutomataNet::countAutomata(void)
{
  return automata.size();
}

/* Kodowanie BDD opisującego lokację loc k-tego automatu
 *
 * Zmienna encode_successor określa czy kodujemy następnik (zmienne "primowane")
 */
BDD
SymAutomataNet::bddLoc(AutIdx k, Automaton::LocationId loc,
                       bool encode_successor) const
{
  ASSUME_NET_IS_CLOSED;

  BDD r = cuddMgr->bddOne();

  Automaton::LocationId val = loc;

  for (unsigned int i = 0; i < autBDDenc[k].numVars; ++i) {
    BDD new_var;

    if (!encode_successor) {
      new_var = (*(autBDDenc[k].pv))[i];
    }
    else {
      new_var = (*(autBDDenc[k].pv_succ))[i];
    }

    if (val != 0) {
      if ((val % 2) == 1) {
        r *= new_var;
      }
      else {
        r *= !new_var;
      }

      val /= 2;
    }
    else {
      r *= !new_var;
    }
  }

  return r;
}

/* Budowanie BDD opisującego przejścia pod akcją action w k-tym automacie */
BDD
SymAutomataNet::bddActTrans(AutIdx k, Automaton::Action action)
{
  BDD r = cuddMgr->bddZero();

  for (Automaton::AllTransSrcDst::iterator srcdst =
         automata[k].transByActions[action].begin();
       srcdst != automata[k].transByActions[action].end();
       ++srcdst) {
    r += bddLoc(k, srcdst->srcLoc, false) * bddLoc(k, srcdst->dstLoc, true);
  }

  return r;
}

/* Kodowanie relacji przejścia k-tego automatu określającej brak zmiany stanu */
BDD
SymAutomataNet::bddTransNoChange(AutIdx k)
{
  BDD r = cuddMgr->bddOne();

  for (unsigned int i = 0; i < autBDDenc[k].numVars; i++) {
    r *= BDD_IFF((*(autBDDenc[k].pv))[i], (*(autBDDenc[k].pv_succ))[i]);
  }

  /* W praktyce można byłoby zapamiętać to BDD i nie generować go za każdym razem
   * bo nie będzie się ono zmieniać.
   */
  return r;
}

/* Kodowanie relacji przejścia automatu produktowego */
BDD *
SymAutomataNet::bddTrans(void)
{
  BDD r = cuddMgr->bddZero();

  ASSUME_NET_IS_CLOSED;

  // std::vector<BDD> transNoChange;
  // for (unsigned int i = 0; i < automata.size(); ++i)
  //     transNoChange.push_back(bddTransNoChange(i));

  BDD defaultNoChange = cuddMgr->bddOne();

  for (unsigned int i = 0; i < automata.size(); ++i) {
    defaultNoChange *= bddTransNoChange(i);
  }

  if (trans_bdd == NULL) {
    unsigned int actNum = 0;

    for (ActionsToAut::iterator act = actionsToAut.begin();
         act != actionsToAut.end(); ++act) {
      // BDD new_act_bdd = cuddMgr->bddOne();
      BDD new_act_bdd = defaultNoChange;

#ifdef BDD_TRANS_REORDER
      Cudd_ReduceHeap(cuddMgr->getManager(), CUDD_REORDER_SIFT, 100000);
#endif

      VectAutIdx autIdx = act->second;
#ifdef VERBOSE
      cout << "action: " << act->first;
      cout << " (size=" << autIdx.size() << ")" << endl;
#endif

#ifndef VERBOSE
      cerr << "\rEncoding: " << actNum << std::flush;
#endif
      // for (unsigned int i = 0; i < automata.size(); ++i)
      //             {
      //                 bool hasAct = false;
      //
      //                 if (autIdx.size() > 1)
      //                 {
      //                     for (VectAutIdx::iterator idx = autIdx.begin(); idx != autIdx.end(); ++idx)
      //                     {
      //                         if (*idx == i)
      //                         {
      //                             hasAct = true;
      //                             /* z usuwaniem jednak bywa wolniej... przy tak małych wektorach jednak lepiej szukać */
      //                             autIdx.erase(idx); /* usuwamy, będzie mniej szukania */
      //                             break;
      //                         }
      //                     }
      //                 }
      //                 else
      //                 {
      //                     /* jak mamy akcję niesynchronizowaną (tylko związaną z jednym automatem)
      //                      * to możemy to zrobić nieco prościej, tzn. sprawdzamy tylko [0]. Może być
      //                      * trochę szybsze dzięki temu jak jest więcej akcji lokalnych.
      //                      */
      //                     if (autIdx[0] == i) hasAct = true;
      //                 }
      //
      //                 if (hasAct)
      //                     new_act_bdd *= bddActTrans(i, act->first);
      //                 else
      //                 {
      //                     new_act_bdd *= bddTransNoChange(i);
      //                     // new_act_bdd *= transNoChange[i];
      //                 }
      //             }

      for (VectAutIdx::iterator idx = autIdx.begin(); idx != autIdx.end(); ++idx) {
        new_act_bdd = new_act_bdd.ExistAbstract(*(autBDDenc[*idx].pv_E));
        new_act_bdd = new_act_bdd.ExistAbstract(*(autBDDenc[*idx].pv_succ_E));
        new_act_bdd *= bddActTrans(*idx, act->first);
      }

      r += new_act_bdd;
      ++actNum;
    }

    trans_bdd = new BDD(r);
  }

#ifndef VERBOSE
  cerr << endl;
#endif

  return trans_bdd;
}

/* Kodowanie BDD opisującego lokację początkową automatu produktowego */
BDD
SymAutomataNet::bddInitLoc(void)
{
  BDD r = cuddMgr->bddOne();

  ASSUME_NET_IS_CLOSED;

  for (unsigned int i = 0; i < automata.size(); i++) {
    r *= bddLoc(i, automata[i].initLocationId, false);
  }

  // r.PrintMinterm();

  return r;
}

/* Pobieranie liczby zmiennych BDD potrzebych do zakodowania stanu */
unsigned int
SymAutomataNet::getPVnvars(void)
{
  ASSUME_NET_IS_CLOSED;

  return pv_nvars;
}

/* Pobieranie wektora zmiennych BDD opisujących stany */
vector<BDD> *
SymAutomataNet::getPVvector(void)
{
  ASSUME_NET_IS_CLOSED;

  return pv;
}

/* Pobieranie wektora zmiennych BDD "primowanych", tzn. opisujących następniki */
vector<BDD> *
SymAutomataNet::getPVsuccVector(void)
{
  ASSUME_NET_IS_CLOSED;

  return pv_succ;
}

BDD *
SymAutomataNet::getPVexist(void)
{
  ASSUME_NET_IS_CLOSED;

  return pv_E;
}

BDD *
SymAutomataNet::getPVsuccExist(void)
{
  ASSUME_NET_IS_CLOSED;

  return pv_succ_E;
}

/* Wyświetlanie wszystkich automatów dodanych do sieci */
void
SymAutomataNet::show(void)
{
  cout << "Total number of (unprimed) variables: " << pv_nvars << endl;
  cout << "Network consists of " << countAutomata() << " automata" << endl <<
       endl;

  int i = 0;

  for (Automata::iterator ii = automata.begin(); ii != automata.end(); ++ii) {
    cout << "----- Automaton ID = " << i++ << endl;
    ii->show();
    cout << endl;
  }

  showActions();
}

/* Pobieranie identyfikatora automatu danego nazwą */
AutIdx
SymAutomataNet::getAutomatonId(string automaton) const
{
  // cout << "XXX: " << automaton << endl;
  return automataByName.at(automaton);
}

VectAutIdx
SymAutomataNet::getAgentAutomata(Automaton::AgentId agent)
{
  return agentAutomata.at(agent);
}

BDD
SymAutomataNet::bddLocation(string automaton, string location) const
{
  CHECK_NET_IS_CLOSED;
  AutIdx idx = getAutomatonId(automaton);
  return bddLoc(idx, automata[idx].getLocationId(location), false);
}

/* Pobieranie początkowego indeksu zmiennych BDD dla i-tego automatu */
unsigned int
SymAutomataNet::getAutomatonIndexBegin(unsigned int i)
{
  return autBDDenc[i].varBegin;
}

/* Pobieranie górnego ograniczenia indeksów zmienych BDD dla i-tego automatu */
unsigned int
SymAutomataNet::getAutomatonIndexBound(unsigned int i)
{
  return autBDDenc[i].varBegin + autBDDenc[i].numVars;
}

void
SymAutomataNet::addValuationsForAutomaton(string automatonName,
    std::map<string, string> valuations)
{
  BDD result;

  std::map<string, string>::const_iterator valuations_it;

  for (valuations_it = valuations.begin(); valuations_it != valuations.end();
       ++valuations_it) {
    string locName = valuations_it->second;
    string varName = valuations_it->first;
    BDD locBDD = bddLocation(automatonName, locName);
#ifdef INPUT_VERBOSE
    cout << "  locName=" << locName << " / varName=" << varName << endl;
#endif

    if (valuationsForNetwork.find(varName) == valuationsForNetwork.end()) {
      result = locBDD;
    }
    else {
      result = locBDD + valuationsForNetwork[varName];
    }

    valuationsForNetwork[varName] = result;
  }
}

BDD
SymAutomataNet::getValuations(string varName) const
{
  if (valuationsForNetwork.find(varName) == valuationsForNetwork.end()) {
    FERROR("no valuations defined for var: " + varName);
  }

  return valuationsForNetwork.at(varName);
}

void
SymAutomataNet::showBddStates(BDD states)
{
  BDD unproc = states;

  while (unproc != cuddMgr->bddZero()) {
    BDD tmp = unproc.PickOneMinterm(*pv);
    cout << globalAutState_toStr(tmp) << endl;
    unproc = unproc - tmp;
  }
}

string
SymAutomataNet::globalAutState_toStr(BDD global_state)
{
  unsigned int nAut = automata.size();
  string str = "( ";

  for (unsigned int i = 0; i < nAut; i++) {
    str += "<" + automata[i].getName() + "/" + localAutState_toStr(i,
           global_state) + "> ";
  }

  return str + ")";
}

string
SymAutomataNet::localAutState_toStr(AutIdx aut_idx, BDD global_state)
{
  unsigned int nLocations = automata[aut_idx].countLocations();
  string locationName = "(unknown location)";

  for (unsigned int locId = 0; locId < nLocations; locId++) {
    if (bddLoc(aut_idx, locId, false) * global_state != cuddMgr->bddZero()) {
      locationName = automata[aut_idx].getLocationName(locId);
    }
  }

  return locationName;
}

BDD
SymAutomataNet::bddGreenStates(Automaton::AgentId agent_id)
{
  BDD st = cuddMgr->bddZero();
  VectAutIdx automata_for_agent = agentAutomata[agent_id];

  /*
   * Przyjmujemy uproszczenie:
   *
   * Jak agent składa się z kilku automatów, to stan "zielony" wyznaczony
   * jest przez tylko jeden a automatów. Przykładowo, mamy dwa automaty:
   * a i b tworzące agenta A. Wtedy stan "zielony" można określić tylko
   * w postaci: "automat a jest w stanie x" lub "automat b jest w stanie y".
   * Nie określamy stanów "zielony" będących produktem obu automatów.
   */

  for (VectAutIdx::iterator aut = automata_for_agent.begin();
       aut != automata_for_agent.end(); ++aut) {
    Automaton::LocationsSet locs = automata[*aut].greenLocations;

    for (Automaton::LocationsSet::iterator loc = locs.begin(); loc != locs.end();
         ++loc) {
      st += bddLoc(*aut, *loc, false);
    }
  }

  return st;
}

