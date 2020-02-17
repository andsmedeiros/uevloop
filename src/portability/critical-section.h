/** \file critical-section.h
  * \brief Contains macros for delimitating critical sections.
  */

#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#ifndef UEVLOOP_CRITICAL_ENTER
/** \brief Enters a critical section.
  *
  * This is a no-op meant to be overridden by the programmer, according to the
  * synchronisation methods available on the target platform.
  */
#define UEVLOOP_CRITICAL_ENTER
#endif /* UEVLOOP_CRITICAL_ENTER */

#ifndef UEVLOOP_CRITICAL_EXIT
/** \brief Exits a critical section..
  *
  * This is a no-op meant to be overridden by the programmer, according to the
  * synchronisation methods available on the target platform.
  */
#define UEVLOOP_CRITICAL_EXIT
#endif /* UEVLOOP_CRITICAL_EXIT */

#ifdef UEVLOOP_CRITICAL_SECTION_OBJ_TYPE
/** \brief The global critical section object.
*
* Should the programmer need a critical section object to manage the system lock,
* the `UEVLOOP_CRITICAL_SECTION_OBJ_TYPE` must be defined with the desired
* type of such object.
*
* An object of type `UEVLOOP_CRITICAL_SECTION_OBJ_TYPE` will be then declared as
* an external global under the symbol `uevloop_critical_section`.
* It is the programmer's responsability to actually allocate such object. It will
* then be available in all critical sections.
*/
extern UEVLOOP_CRITICAL_SECTION_OBJ_TYPE uevloop_critical_section;
#endif /* UEVLOOP_CRITICAL_SECTION_OBJ_TYPE */

#endif /* end of include guard: CRITICAL_SECTION_H */
