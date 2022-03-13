/*
 * Copyright 2016 Sony Corporation
 * File added and changed on 2016-01-20
 */
/*
 *  icx_usbtrns.h
 *
 *  Copyright 2011 Sony Corporation
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  version 2 of the  License.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#ifndef ICX_USBTRNS_H
#define ICX_USBTRNS_H

#include <asm/ioctl.h>

/* Command Code */
#define MSC_TRNS_IOC_MAGIC  'g'
#define MSC_GET_LAPSED_TIME _IOR(MSC_TRNS_IOC_MAGIC, 1, int)
#define MSC_FSYNC           _IO (MSC_TRNS_IOC_MAGIC, 2)
#define MSC_TRNS_IOC_MAXNR  2

#define MSC_MAX_JIFF_TIME 0xFFFFFFFF        /* jiffies max value */


struct trns_to_msc_func {
    void    (*trns_start)(void);
};

struct msc_to_trns_func{
    int     (*get_lapsed_time)(unsigned long *);
    void    (*fsync_all)(void);
};

#endif /* ICX_USBTRNS_H */
