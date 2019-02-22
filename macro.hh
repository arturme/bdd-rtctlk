//
//  macro.hh
//  Created by Artur Meski on 2010-10-14.
//  Copyright 2010 VerICS Team. All rights reserved.
//

#define LINENUM std::cout << __FILE__ <<  " (function " << __func__ << "), line " << __LINE__ << std::endl;

/* Fatal error */
#define FERROR(s) \
{ \
  std::cout <<  __FILE__ << " (function " << __func__ << "), line " << __LINE__ << ": " << s << std::endl; \
  exit(1); \
}

#define VERB(s) \
if (opts.verbose > 0) { \
  std::cerr << "(i) " << s << std::endl;    \
}

#define VERB_L2(s) \
if (opts.verbose > 1) { \
  std::cerr << "(i) " << s << std::endl;    \
}

#define VERB_L3(s) \
if (opts.verbose > 2) { \
  std::cerr << "(i) " << s << std::endl;    \
}

/****************************************************************************/

#define DEF_AGENT(a,b) Agents (a); (a).push_back((b));

