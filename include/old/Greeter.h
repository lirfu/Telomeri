/* NOTE: ifdefs are necasary since C compiler is used to compile .c object
 *  files which implement functions listed here, but C++ linker is used. */
#ifdef __cplusplus
extern "C" {
#endif

void old_greet();

#ifdef __cplusplus
}
#endif
