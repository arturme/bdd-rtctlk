//
//  automata.hh
//  BDD-LTLK
//
//  Created by Artur Meski on 2010-10-14.
//  Copyright 2010 VerICS Team. All rights reserved.
//

#ifndef AUTOMATA_HH_ZNJ05LFI
#define AUTOMATA_HH_ZNJ05LFI

#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <cassert>
#include <stdarg.h>
#include <cstdlib>
#include "cudd/include/cuddObj.hh"
#include "macro.hh"
#include "bdd_macro.hh"
#include "options.hh"


extern Options opts;

using std::string;
using std::vector;
using std::endl;
using std::cout;
using std::cerr;

class Automaton
{
    friend class SymAutomataNet;

    typedef unsigned int LocationId;
    typedef std::map<string, LocationId> Locations;
    typedef std::set<LocationId> LocationsSet;
    typedef string Action;
    typedef std::set<Action> Actions;
    struct TransitionDst {
      Action action;
      LocationId dstLoc;
    };
    typedef std::vector<TransitionDst> TransitionDsts;
    typedef std::map<LocationId, TransitionDsts> Transitions;

    struct TransSrcDst {
      LocationId srcLoc;
      LocationId dstLoc;
    };
    typedef std::vector<TransSrcDst> AllTransSrcDst;
    typedef std::map<Action, AllTransSrcDst> TransByActions;

    typedef unsigned int AgentId;

    // var -> BDD representing valuations
    std::map<string, string> valuations;

    string automatonName;
    Locations locations;
    Transitions transitions;
    TransByActions transByActions;
    LocationId initLocationId;
    LocationsSet greenLocations;
    Actions actions;
    bool initLocation_defined;
    bool agent_defined;

    AgentId agentId;

    bool hasLocation(string name);

  public:
    Automaton(void);
    void setName(string name);
    string getName(void) const
    {
      return automatonName;
    }
    void addLocation(string name);
    void addGreenLocation(string name);
    void setLocationGreen(string name);
    void showLocations(void);
    std::map<string, string> getValuations() const
    {
      return valuations;
    }
    unsigned int countLocations(void);
    LocationId getLocationId(string name) const;
    string getLocationName(LocationId locId);
    BDD bddLocation(string automatonName, string locationName);
    void setInitLocation(string name);
    void addTransition(string srcLocName, string dstLocName, Action action);
    void showTransitions(void);
    void showActions(void);

    void addValuation(string locName, string varNr);
    void setAgent(AgentId agId);
    void show(void);
};

/* (Symbolic) Automata Network */
typedef unsigned int AutIdx;
typedef std::vector<AutIdx> VectAutIdx;

class SymAutomataNet
{
    typedef std::vector<Automaton> Automata;
    typedef std::map<string, AutIdx> AutomataByName;
    struct BDDenc {
      vector<BDD> *pv;
      vector<BDD> *pv_succ;
      BDD *pv_E;
      BDD *pv_succ_E;
      unsigned int numVars;
      unsigned int varBegin;
    };
    typedef std::vector<BDDenc> AutomataBDDenc;
    typedef std::map<Automaton::Action, VectAutIdx> ActionsToAut;

    Automata automata;
    Cudd *cuddMgr;
    bool netIsClosed;

    vector<BDD> *pv; /* states (p) */
    vector<BDD> *pv_succ; /* successors (p') of the states p */
    BDD *pv_E;
    BDD *pv_succ_E;
    BDD *trans_bdd;
    std::map<string, BDD> valuationsForNetwork;
    AutomataBDDenc autBDDenc;

    unsigned int pv_nvars; /* the number of used p-variables */

    AutomataByName automataByName;
    ActionsToAut actionsToAut;

    BDD bddActTrans(AutIdx k, Automaton::Action action);
    BDD bddTransNoChange(AutIdx k);
    BDD bddLoc(AutIdx k, Automaton::LocationId loc, bool encode_successor) const;

    typedef std::map<Automaton::AgentId, VectAutIdx> AgentAutomata;

    AgentAutomata agentAutomata;

    void addValuationsForAutomaton(string automatonNameA,
                                   std::map<string, string> valuations);
  public:
    SymAutomataNet(void);
    Cudd *getCuddMgr(void);
    void closeNet(void);
    void addAutomaton(Automaton automaton);
    void showActions(void);
    unsigned int countAutomata(void);
    BDD *bddTrans(void);
    BDD bddInitLoc(void);
    BDD bddLocation(string automatonName, string locationName) const;

    BDD getValuations(string varName) const;
    BDD addValuation(string automatonName, string locName, string varNr);
    void addValuationForAllAutomata();

    unsigned int getPVnvars(void);
    vector<BDD> *getPVvector(void);
    vector<BDD> *getPVsuccVector(void);
    BDD *getPVexist(void);
    BDD *getPVsuccExist(void);
    void show(void);
    unsigned int getAutomatonIndexBegin(unsigned int i);
    unsigned int getAutomatonIndexBound(unsigned int i);
    AutIdx getAutomatonId(string automatonName) const;
    Automata getAutomata() const
    {
      return automata;
    }
    VectAutIdx getAgentAutomata(Automaton::AgentId agent);
    void makeAutAgent(Automaton::AgentId agent_id, string automaton);
    bool isAutInAgent(Automaton::AgentId agent_id, string automaton);
    void showAgents(void);
    void showBddStates(BDD states);
    string globalAutState_toStr(BDD global_state);
    string localAutState_toStr(AutIdx aut_idx, BDD global_state);
    BDD bddGreenStates(Automaton::AgentId agent_id);
};

// Macros
#define ASSUME_NOT_EMPTY(varname, what) \
if (varname == "") { \
  cout << __FILE__ <<  " (function " << __func__ << "), line " << __LINE__ << "; Empty string given: " << what << endl; \
  exit(1); \
}

#define ASSUME_NET_IS_CLOSED assert(netIsClosed);
#define CHECK_NET_IS_CLOSED if (!netIsClosed) { \
    cout << __FILE__ <<  " (function " << __func__ << "), line " << __LINE__ << "; Call closeNet() first!" << endl; \
    exit(1); \
} \

using std::cout;

#endif /* end of include guard: AUTOMATA_HH_ZNJ05LFI */
