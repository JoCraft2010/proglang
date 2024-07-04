#ifndef DEBUG
  #ifdef VERBOSE
    #define DEBUG(msg) std::cout << msg
  #else
    #define DEBUG(msg)
  #endif
#endif

#ifndef ERROR
  #define ERROR(msg) std::cerr << "\033[1;31m" << msg << "\033[0m"
#endif
