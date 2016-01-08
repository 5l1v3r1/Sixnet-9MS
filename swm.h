#include "mii.h"
int registerDump(void* switchblob);

void* findSwm(); // no idea what this function returns. assume it sets up globals

int mdio_read(void* switchblob, int phyaddr, int regnum);
