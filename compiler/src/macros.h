#ifndef DEBUG
  #ifdef VERBOSE
    #define DEBUG(msg) std::cout << msg
  #else
    #define DEBUG(msg)
  #endif
#endif

#ifndef INFO
  #define INFO(msg) std::cerr << "\033[1;34m" << msg << "\033[0m"
#endif

#ifndef ERROR
  #define ERROR(msg) std::cerr << "\033[1;31m" << msg << "\033[0m"
#endif
