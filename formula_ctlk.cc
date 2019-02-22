#include "formula_ctlk.hh"

FormCTLK::FormCTLK(string a, string l)
{
    oper = CTLK_PV;
    aut = a;
    loc = l;
    arg[0] = NULL;
    arg[1] = NULL;
    bdd = NULL;
}

FormCTLK::FormCTLK(bool val)
{
    oper = CTLK_TF;
    tf = val;
    arg[0] = NULL;
    arg[1] = NULL;
    bdd = NULL;
}

FormCTLK::FormCTLK(Oper op, FormCTLK *form1)
{
    oper = op;
    arg[0] = form1;
    arg[1] = NULL;
    bdd = NULL;
}

FormCTLK::FormCTLK(Oper op, FormCTLK *form1, FormCTLK *form2)
{
    oper = op;
    arg[0] = form1;
    arg[1] = form2;
    bdd = NULL;
}

FormCTLK::FormCTLK(Oper op, Agents &ag, FormCTLK *form1)
{
    if (ag.size() != 1) FERROR("There must be exactly one agent supplied!");
    oper = op;
    agents = ag;
    arg[0] = form1;
    arg[1] = NULL;
    bdd = NULL;
}

FormCTLK::FormCTLK(Oper op, int a, int b, FormCTLK *form1)
{
    assert(op == CTLK_RT_EG || op == CTLK_RT_EF);
    oper = op;
    arg[0] = form1;
    arg[1] = NULL;
    left = a;
    right = b;
    bdd = NULL;
}

FormCTLK::FormCTLK(Oper op, int a, int b, FormCTLK *form1, FormCTLK *form2)
{
    assert(op == CTLK_RT_EU);
    oper = op;
    arg[0] = form1;
    arg[1] = form2;
    left = a;
    right = b;
    bdd = NULL;
}

FormCTLK::~FormCTLK()
{
    delete arg[0];
    delete arg[1];
    delete bdd;
}

string
FormCTLK::toStr(void)
{
    switch(oper)
    {
    case CTLK_PV:
        return "<" + aut + ":" + loc + ">";
        break; // estetyka ;)
    case CTLK_TF:
        if (tf) return "true";
        else return "false";
        break;
    case CTLK_AND:
        return "( " + arg[0]->toStr() + " AND " + arg[1]->toStr() + " )";
        break;
    case CTLK_OR:
        return "( " + arg[0]->toStr() + " OR " + arg[1]->toStr() + " )";
        break;
    case CTLK_IMPL:
        return "( " + arg[0]->toStr() + " -> " + arg[1]->toStr() + " )";
        break;
    case CTLK_NOT:
        return "~" + arg[0]->toStr();
        break;
    case CTLK_EX:
        return "EX(" + arg[0]->toStr() + ")";
        break;
    case CTLK_EG:
        return "EG(" + arg[0]->toStr() + ")";
        break;
    case CTLK_EU:
        return "E(" + arg[0]->toStr() + " U " + arg[1]->toStr() + ")";
        break;
    case CTLK_EF:
        return "EF(" + arg[0]->toStr() + ")";
        break;
    case CTLK_KD:
    {
        std::stringstream ss;
        ss << agents[0];
        return "KD_" + ss.str() + "(" + arg[0]->toStr() + ")";
        break;
    }
    case CTLK_OD:
    {
        std::stringstream ss;
        ss << agents[0];
        return "OD_" + ss.str() + "(" + arg[0]->toStr() + ")";
        break;
    }
    case CTLK_RT_EG:
    {
        std::stringstream ss;
        if (right == RT_INF)
            ss << left << ",inf";
        else
            ss << left << "," << right;
        return "EG(" + ss.str() + "," + arg[0]->toStr() + ")";
        break;
    }
    case CTLK_RT_EU:
    {
        std::stringstream ss;
        if (right == RT_INF)
            ss << left << ",inf";
        else
            ss << left << "," << right;

        return "EU(" + ss.str() + "," + arg[0]->toStr() + "," + arg[1]->toStr() + ")";
        break;
    }
    case CTLK_RT_EF:
    {
        std::stringstream ss;
        if (right == RT_INF)
            ss << left << ",inf";
        else
            ss << left << "," << right;
        return "EF(" + ss.str() + "," + arg[0]->toStr() + ")";
        break;
    }
    default:
        return "??";
    }
    assert(0);
}

void
FormCTLK::cleanup(void)
{
    if (arg[0] != NULL) arg[0]->cleanup();
    if (arg[1] != NULL) arg[1]->cleanup();

    if (oper != CTLK_PV)
    {
        delete bdd;
        bdd = NULL;
    }
}

BDD
FormCTLK::satStates(const BDD &reach)
{
    BDD r = cuddMgr->bddZero();
    if (oper == CTLK_TF)
    {
        if (tf)
            r = reach;
        else
            r = cuddMgr->bddZero();
    }
    else if (oper == CTLK_PV)
    {
        assert(bdd != NULL);
        r = reach * *bdd;
    }
    else if (oper == CTLK_AND)
    {
        r = arg[0]->satStates(reach) * arg[1]->satStates(reach);
    }
    else if (oper == CTLK_OR)
    {
        r = arg[0]->satStates(reach) + arg[1]->satStates(reach);
    }
    else if (oper == CTLK_IMPL)
    {
        r = (reach * !arg[0]->satStates(reach)) + arg[1]->satStates(reach);
    }
    else if (oper == CTLK_NOT)
    {
        r = reach * !arg[0]->satStates(reach);
    }
    else if (oper == CTLK_EX)
    {
        r = preImage(arg[0]->satStates(reach));
    }
    else if (oper == CTLK_EG)
    {
        r = arg[0]->satStates(reach);
        BDD r_p = cuddMgr->bddZero();
        while (r != r_p)
        {
            r_p = r;
            r = r * preImage(r);
        }
    }
    else if (oper == CTLK_EU)
    {
        r = arg[1]->satStates(reach); //* reach;
        BDD r_p = reach;
        BDD y = arg[0]->satStates(reach);
        while (r != r_p)
        {
            r_p = r;
            r = r + (y * preImage(r));
        }
    }
    else if (oper == CTLK_EF)
    {
        r = arg[0]->satStates(reach); //* reach;
        if (r != cuddMgr->bddZero())
        {
            BDD r_p = reach;
            while (r != r_p)
            {
                r_p = r;
                r = r + (reach * preImage(r));
            }
        }
    }
    else if (oper == CTLK_KD)
    {
        r = (arg[0]->satStates(reach) * reach).ExistAbstract(bddOnlyIth(getAgent())) * reach;
    }
    else if (oper == CTLK_OD)
    {
        assert(0); // placeholder
    }
    else if (oper == CTLK_RT_EG)
    {
        if (right < left || left < 0) return cuddMgr->bddZero();

        r = arg[0]->satStates(reach);
        BDD r_p;
        for (int i = 0; i < right-left; ++i)
        {
            r_p = r;
            r = preImage(r) * r; 
            if (r == r_p) break;
        }
        for (int i = 0; i < left; ++i)
        {
            r_p = r;
            r = preImage(r);
            if (r == r_p) break;
        }
    }
    else if (oper == CTLK_RT_EU)
    {
        if (right < left || left < 0) return cuddMgr->bddZero(); 

        BDD f = arg[0]->satStates(reach);
        BDD g = arg[1]->satStates(reach);
        BDD r_p;
        r = g;
        for (int i = 0; i < right-left; ++i)
        {
            r_p = r;
            r = g + (f * preImage(r));
            if (r == r_p) break;
        }
        for (int i = 0; i < left; ++i)
        {
            r_p = r;
            r = f * preImage(r);
            if (r == r_p) break;
        }
    }
    else if (oper == CTLK_RT_EF)
    {
        if (right < left || left < 0) return cuddMgr->bddZero(); 

        BDD g = arg[0]->satStates(reach);
        BDD r_p;
        r = g;
        for (int i = 0; i < right-left; ++i)
        {
            r_p = r;
            r = g + preImage(r);
            if (r == r_p) break;
        }
        for (int i = 0; i < left; ++i)
        {
            r_p = r;
            r = preImage(r);
            if (r == r_p) break;
        }
    }
    return r;
}

bool
FormCTLK::hasOper(Oper op)
{
    if (oper == op)
        return true;
    else
    {
        bool result = false;
        if (arg[0] != NULL) result = arg[0]->hasOper(op);
        if (!result && arg[1] != NULL) result = arg[1]->hasOper(op);
        return result;
    }
}

void
FormCTLK::encodeAtoms(const SymAutomataNet *net)
{
    if (CTLK_COND_1ARG(oper))
    {
        arg[0]->encodeAtoms(net);
    }
    else if (CTLK_COND_2ARG(oper))
    {
        arg[0]->encodeAtoms(net);
        arg[1]->encodeAtoms(net);
    }
    else if (oper == CTLK_PV)
    {
        bdd = new BDD(net->bddLocation(aut, loc));
    }
}

bool
FormCTLK::isECTLK(void) const
{
    if (oper == CTLK_PV)
    {
        return true;
    }

    if (oper == CTLK_NOT)
    {
        if (arg[0]->getOper() == CTLK_PV || arg[0]->getOper() == CTLK_TF)
            return true;
        else
            return false;
    }

    if (CTLK_COND_1ARG(oper))
    {
        return arg[0]->isECTLK();
    }

    if (CTLK_COND_2ARG(oper))
    {
        return arg[0]->isECTLK() && arg[1]->isECTLK();
    }

    assert(0);

    return false;
}
