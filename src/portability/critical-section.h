/** \file critical-section.h
  * \brief Contains macros for delimitating critical sections.
  */

#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#ifndef UEL_CRITICAL_ENTER
/** \brief Enters a critical section.
  *
  * This is a no-op meant to be overridden by the programmer, according to the
  * synchronisation methods available on the target platform.
  */
#define UEL_CRITICAL_ENTER
#endif /* UEL_CRITICAL_ENTER */

#ifndef UEL_CRITICAL_EXIT
/** \brief Exits a critical section..
  *
  * This is a no-op meant to be overridden by the programmer, according to the
  * synchronisation methods available on the target platform.
  */
#define UEL_CRITICAL_EXIT
#endif /* UEL_CRITICAL_EXIT */

#ifdef UEL_CRITICAL_SECTION_OBJ_TYPE
/** \brief The global critical section object.
*
* Should the programmer need a critical section object to manage the system lock,
* the `UEL_CRITICAL_SECTION_OBJ_TYPE` must be defined with the desired
* type of such object.
*
* An object of type `UEL_CRITICAL_SECTION_OBJ_TYPE` will be then declared as
* an external global under the symbol `uel_critical_section`.
* It is the programmer's responsability to actually allocate such object. It will
* then be available in all critical sections.
*/
extern UEL_CRITICAL_SECTION_OBJ_TYPE uel_critical_section;
#endif /* UEL_CRITICAL_SECTION_OBJ_TYPE */

#endif /* end of include guard: CRITICAL_SECTION_H */
