/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "qr_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jolttypes.h"
#include "mbedtls/bignum.h"
#include "nano_lib.h"

/* Creates a receive string at the public key for the specified amount of raw */
jolt_err_t receive_url_create( char *buf, size_t buf_len, const uint256_t public_key, const mbedtls_mpi *amount )
{
    char address[ADDRESS_BUF_LEN];
    char amount_str[BALANCE_DEC_BUF_LEN];
    size_t olen;
    jolt_err_t err;

    /* Translate the public key to an address string */
    err = nl_public_to_address( address, sizeof( address ), public_key );
    if( err != E_SUCCESS ) { return err; }

    /* Translate the numerical amount to a string */
    if( NULL == amount ) { strcpy( amount_str, "0" ); }
    else {
        mbedtls_mpi_write_string( amount, 10, amount_str, sizeof( amount_str ), &olen );
    }

#if CONFIG_JOLT_QR_TYPE_SIMPLE
    strcpy( buf, address );
    strlwr( buf );
#endif

#if CONFIG_JOLT_QR_TYPE_STANDARD
    char *buf_moving = buf;
    strncpy( buf, CONFIG_NANO_LIB_ADDRESS_PREFIX, strlen( CONFIG_NANO_LIB_ADDRESS_PREFIX ) - 1 );
    buf_moving += strlen( prefix );

    *buf_moving = ':';
    buf_moving++;

    strcpy( buf_moving, address );
    buf_moving += strlen( address );

    strcpy( buf_moving, "?amount=" );
    buf_moving += 8;

    strcpy( buf_moving, amount_str );
    buf_moving += strlen( str_amount );
#endif

#if CONFIG_JOLT_QR_TYPE_SHORT
    /* Can be represeented using a more compact QR type */
    strcpy( buf, toupper( address ) );
    if( buf[0] == 'x' || buf[0] == 'X' ) { buf[3] = '-'; }
    else {
        buf[4] = '-';
    }
#endif

    return E_SUCCESS;
}
