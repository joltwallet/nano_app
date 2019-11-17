#ifndef NANO_CMDS_H__
#define NANO_CMDS_H__

int nano_cmd_count( int argc, char **argv );

/**
 * @brief Print the Addresses of given index
 * If index is not specified, defaults to last used in Jolt GUI.
 *
 * Optionally takes a second argument to print the inclusive range.
 *
 * Examples:
 *
 *
 *     jolt> Nano address
 *     {"addresses":[{"index":0,"address":nano_3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz}]}
 *
 *     jolt> Nano address 0
 *     {"addresses":[{"index":0,"address":nano_3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz}]}
 *     jolt> Nano address 1 4
 *     {"addresses":[{"index":1,"address":"nano_3nxuasbi7ohysb41fgoomisepnrbbmpjt5m7rzjmfj1yw1erdrxcj7nq1eq1"},{"index":2,"address":"nano_3nyfu94d1b4hukrku4145ijtug6d5sngz7n96ea837pfx7c8s6rrr53q7pej"},{"index":3,"address":"nano_1umgo1godwfma7ik3t1qjkgko6x4w6x1ipt5soqx8txi9n346c1uzffxnq6q"},{"index":4,"address":"nano_3zw77prqkmo3ccpei78et95due7ioshh3hfogs8coqqhosccjnqs5r73sitx"}]}
 *
 */
int nano_cmd_address( int argc, char **argv );

/**
 * @brief Modify the contactbook
 *
 * Print the entire contact book ([Name] - [Address]):
 *     contact print [lower] [upper]
 * If lower is provided, it will print from that address to the end.
 * If upper is provided, it will print from lower to upper (inclusive).
 *
 * Remove a contact at index:
 *     contact delete [index]
 *
 * Add a contact to the end of the contact book
 *     contact add [name] [address]
 *
 * Insert a contact at index
 *     contact insert [index] [name] [address]
 *
 * Replace a contact at index
 *     contact update [index] [name] [address]
 *
 * Clear the entire contact book:
 *     contact clear
 *
 * @return 0 on success
 *
 */
int nano_cmd_contact( int argc, char **argv );

/**
 * @brief Sign a block
 *
 * block_sign account_index frontier_block new_block
 *
 * account_index  - derivation index of account to use
 * frontier_block - escaped json representing the account's frontier block
 * new_block      - escaped json representing the block to sign
 *
 * Example command;
 * Nano block_sign 0 "{\"account\":
 * \"xrb_1n3hccnsf5tz4pbuftk69p7jobuh6afcriqqzbkd5pooyyr5be6fc3khfmbi\",\"amount\":
 * \"1000000000000000000000000000\",  \"type\": \"state\",  \"representative\":
 * \"xrb_1cwswatjifmjnmtu5toepkwca64m7qtuukizyjxsghujtpdr9466wjmn89d8\",
 * \"previous\":
 * \"4635A79A0474ACD84C42DC847163E15867E547CAEF3367E7FAA45187E965B88D\",
 * \"work\": \"9152b3d12cc0196b\",  \"signature\":
 * \"A70818F4A9F533CFC52EFE8454F5243C8E5B6801DC82E613112891D58825AE084D6E154AF88F1A85CF7DCDB6E4FA92205829474548064FABB33858D7020BA80C\",
 * \"link\":
 * \"3BC45C628A4269D841272B48C67E89927B2B35A4B01BD3A71F9FBD3523D492C6\",
 * \"balance\": \"10000000000000000000000000000\"}" "{\"account\":
 * \"xrb_1n3hccnsf5tz4pbuftk69p7jobuh6afcriqqzbkd5pooyyr5be6fc3khfmbi\",
 * \"type\": \"state\", \"representative\":
 * \"xrb_1cwswatjifmjnmtu5toepkwca64m7qtuukizyjxsghujtpdr9466wjmn89d8\",
 * \"previous\":
 * \"D09342477E592B53A0C031A1D529476732DACEFE03CCD3867294D2DF0995789D\",
 * \"link\":
 * \"261F4B979009FD5665331AC72A95C8906285DCCAAACE2FD0F07AA39D738A1216\",
 * \"balance\": \"9000000000000000000000000000\"}"
 */
int nano_cmd_block_sign( int argc, char **argv );

#endif
