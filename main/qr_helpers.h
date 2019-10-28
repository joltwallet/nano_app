/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_NANO_QR_HELPERS_H__
#define __JOLT_NANO_QR_HELPERS_H__

#include "mbedtls/bignum.h"
#include "nano_lib.h"
#include "qrcode.h"

/* Creates a receive string at the public key for the specified amount of raw */
jolt_err_t receive_url_create( char *buf, size_t buf_len, const uint256_t public_key, const mbedtls_mpi *amount );

#endif
