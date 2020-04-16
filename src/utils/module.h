/** \file Defines modules, isolated units of behaviour with lifecycle hooks.
  */

#ifndef UEL_MODULE_H
#define UEL_MODULE_H

struct uel_application;
struct uel_module;

/** \brief Defines a module hook to be called during the module lifecycle
  */
typedef void (*uel_module_hook_t)(struct uel_module *);

/** \brief A module is an isolated unit of behaviour with lifecycle hooks
  *
  * Modules can serve as a variety of purposes:
  *
  * - They can act as bridges to static data, such as SFRs;
  * - They can be object factories, meant to distribute and recycle objects to
  *   other modules;
  * - They can act as services, background processes that interact with other
  *   parts of the application in a sattelite-fashion.
  *
  * Modules are meant to be singletons and user extendable.
  */
typedef struct uel_module uel_module_t;
struct uel_module {
    /** The config hook should be used for any setup of the module dependencies.
      * It fires after the module has been fully initialised, but may still be
      * in an inconsistent state.
      */
    uel_module_hook_t config;
    /** The launch hook is used to start a module. The application will only fire
      * it once all loaded modules are both initialised and configurated
      * (*i.e.* `config` hook has been called).
      */
    uel_module_hook_t launch;
    //! Keeps a reference to the application onto which the module is loaded
    struct uel_application *app;
};

/** \brief Initialised a module
  *
  * \param module The module to be initialised
  * \param config A module hook to be invoked during the configuration phase
  * \param launch A module hook to be invoked during the launching phase
  * \param app The application instante onto which this module will be loaded
  */
void uel_module_init(
    uel_module_t *module,
    uel_module_hook_t config,
    uel_module_hook_t launch,
    struct uel_application *app
);

/** \brief Calls the configuration hook of a module
  *
  * \param module The modules to config
  */
void uel_module_config(uel_module_t *module);

/** \brief Calls the launch hook of a module
  *
  * \param module The modules to launch
  */
void uel_module_launch(uel_module_t *module);

#endif /* end of include guard: UEL_MODULE_H */
