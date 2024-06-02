#ifndef __KB_H
#define __KB_H

void kb_install();
int register_kb_observer(void *fn);
void deregister_kb_observer(void *fn);

#endif
