/** \file critical-section.h
  * \brief Contains a default no-op macro for providing critical sections unless
  * already provided.
  */

  #ifndef CRITIAL_SECTION_H
  #define CRITIAL_SECTION_H

  #ifndef UEVLOOP_CRITICAL_SECTION
  //!
  /** \brief Default critical section implementation.
    *
    * This is a no-op meant to be overridden by the programmer, according to the
    * synchronisation methods available on the target platform.
    *
    * \param body The protected code section
    */
  #define UEVLOOP_CRITICAL_SECTION(body) do{ body }while(0);
  #endif

  #endif /* end of include guard: CRITIAL_SECTION_H */
