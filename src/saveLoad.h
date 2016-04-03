
module *loadModules(uint32_t id);
module *saveLoadInit(void);
void save(module *curModule);
void saveLoadExit(module *curModule);

extern module root;
