#ifndef NANO_CMDS_H__
#define NANO_CMDS_H__

int nano_count(int argc, char**argv);

/**
 * @brief Print the Addresses of given index
 * Optionally takes a second argument to print the inclusive range.
 */
int nano_address(int argc, char ** argv);

#endif
