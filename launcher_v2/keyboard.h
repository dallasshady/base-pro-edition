
// keyboard helpers (for direct input)

#pragma once

#define INVALID_CODE 0xFFFFFFFF

unsigned int getNumCodes(void);
unsigned int getCode(unsigned int index);
unsigned int getCodeIndex(unsigned int code);
const char* getCodeHint(unsigned int index);
