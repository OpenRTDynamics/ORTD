//

#ifdef __ORTD_TARGET_LINUX
  #define __ORTD_PLUGINS_ENABLED

  void *ortd_load_plugin(struct dynlib_simulation_t *sim, char *name);
#endif