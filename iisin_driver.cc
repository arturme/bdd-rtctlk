#include "iisin_driver.hh"
#include "iisin_parser.hh"

iisin_driver::iisin_driver(SymAutomataNet *net)
  : trace_scanning(false), trace_parsing(false)
{
  this->ctlkform = NULL;
  this->net = net;
  nosys = false;
  cur_aut = new Automaton;
}

iisin_driver::~iisin_driver ()
{
  delete cur_aut;
}

int iisin_driver::parse(const std::string &f)
{
  file = f;
  scan_begin();
  yy::iisin_parser parser(*this);
  parser.set_debug_level(trace_parsing);
  int res = parser.parse();
  scan_end();
  return res;
}

void iisin_driver::error(const yy::location &l, const std::string &m)
{
  std::cerr << l << ": " << m << std::endl;
}

void iisin_driver::error(const std::string &m)
{
  std::cerr << m << std::endl;
}

void iisin_driver::pushAutomaton(const std::string &name)
{
  cur_aut->setName(name);
  net->addAutomaton(*cur_aut);
  delete cur_aut;
  cur_aut = new Automaton;
}


FormCTLK *iisin_driver::getFormCTLK(void)
{
  if (ctlkform == NULL) {
    FERROR("CTL formula was not supplied!");
  }

  return ctlkform;
}


