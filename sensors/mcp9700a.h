#ifndef MCP9700A_H
#define MCP9700A_H


#define VREF 2048

extern void mcp9700Init(void);
extern unsigned long getMCP9700_C(void);
extern float getMCP9700_F(void);
extern char *getMCP9700TxtTemp(float tmp, short CF);


#endif