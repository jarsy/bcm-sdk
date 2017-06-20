/*
 * $Id: README.TXT,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

Diagnostic Tests

Notes
-----

Diag tests are not expected to clean up after themselves.
However, each test is expected to initialize the chip properly before
performing its work.  Many of the tests, such as port loopbacks, do this
by calling soc_reset() or by running rc.soc prior to the test.
