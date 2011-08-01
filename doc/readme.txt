A rework of the SW's original with the following goals:
 - remove dependencies on Perl and a built Boost (Regex) by moving to Boost.Xpressive and later to Boost.Spirit
 - further improve performance (e.g. use low level C IO and memory mapped files instead of C++ streams)
 - convert to a single standalone binary
 - move to CMake and support for creating profiling targets in user projects/solutions.

Hopefully, (Lib)Clang will once provide this functionality 'natively' making this tool obsolete.
 
 (...work in progress...)