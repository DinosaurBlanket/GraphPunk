
module *loadModule(uint32_t moduleId); // also loads parents and needed branches
void saveLoadInit(void);
module *getLastModule(void);
void save(void);
void saveLoadExit(void);
