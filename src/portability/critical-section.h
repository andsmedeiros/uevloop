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
  *
  * \note Critical sections are meant to be nested.
  */
#define UEVLOOP_CRITICAL_ENTER
#endif /* UEVLOOP_CRITICAL_ENTER */

#ifndef UEVLOOP_CRITICAL_EXIT
/** \brief Exits a critical section..
  *
  * This is a no-op meant to be overridden by the programmer, according to the
  * synchronisation methods available on the target platform.
  *
  * \note Critical sections are meant to be nested.
  */
#define UEVLOOP_CRITICAL_EXIT
#endif /* UEVLOOP_CRITICAL_EXIT */

#ifndef UEVLOOP_CRITICAL_OBJECT
/** \brief Defines the type of critical section objects.
  *
  * This macro should be defined with the intended type of critical section
  * objects.
  */
#define UEVLOOP_CRITICAL_OBJECT unsigned char
#endif /* UEVLOOP_CRITICAL_OBJECT */

/** \brief The global critical section object.
  *
  * This should be declared globally in the application body.
  * It is meant to store the current nesting level of critical sections.
  */
extern UEVLOOP_CRITICAL_OBJECT uevloop_critical_section;
#endif /* end of include guard: CRITICAL_SECTION_H */
