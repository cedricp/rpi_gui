/*
 * wiringPiI2C.c:
 *	Simplified I2C access routines
 *	Copyright (c) 2013 Gordon Henderson
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with wiringPi.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

/*
 * Notes:
 *	The Linux I2C code is actually the same (almost) as the SMBus code.
 *	SMBus is System Management Bus - and in essentially I2C with some
 *	additional functionality added, and stricter controls on the electrical
 *	specifications, etc. however I2C does work well with it and the
 *	protocols work over both.
 *
 *	I'm directly including the SMBus functions here as some Linux distros
 *	lack the correct header files, and also some header files are GPLv2
 *	rather than the LGPL that wiringPi is released under - presumably because
 *	originally no-one expected I2C/SMBus to be used outside the kernel -
 *	however enter the Raspberry Pi with people now taking directly to I2C
 *	devices without going via the kernel...
 *
 *	This may ultimately reduce the flexibility of this code, but it won't be
 *	hard to maintain it and keep it current, should things change.
 *
 *	Information here gained from: kernel/Documentation/i2c/dev-interface
 *	as well as other online resources.
 *********************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "wiringPiI2C.h"

// I2C definitions

#define I2C_SLAVE	0x0703
#define I2C_SMBUS	0x0720	/* SMBus-level access */

#define I2C_SMBUS_READ	1
#define I2C_SMBUS_WRITE	0

// SMBus transaction types

#define I2C_SMBUS_QUICK		    0
#define I2C_SMBUS_BYTE		    1
#define I2C_SMBUS_BYTE_DATA	    2 
#define I2C_SMBUS_WORD_DATA	    3
#define I2C_SMBUS_PROC_CALL	    4
#define I2C_SMBUS_BLOCK_DATA	    5
#define I2C_SMBUS_I2C_BLOCK_BROKEN  6
#define I2C_SMBUS_BLOCK_PROC_CALL   7		/* SMBus 2.0 */
#define I2C_SMBUS_I2C_BLOCK_DATA    8

// SMBus messages

#define I2C_SMBUS_BLOCK_MAX	32	/* As specified in SMBus standard */	
#define I2C_SMBUS_I2C_BLOCK_MAX	32	/* Not specified but we use same structure */

// Structures used in the ioctl() calls

union i2c_smbus_data
{
  uint8_t  byte ;
  uint16_t word ;
  uint8_t  block [I2C_SMBUS_BLOCK_MAX + 2] ;	// block [0] is used for length + one more for PEC
} ;

struct i2c_smbus_ioctl_data
{
  char read_write ;
  uint8_t command ;
  int size ;
  union i2c_smbus_data *data ;
} ;

static int piModel2 = 0;

static void piBoardRevOops (const char *why)
{
  fprintf (stderr, "piBoardRev: Unable to determine board revision from /proc/cpuinfo\n") ;
  fprintf (stderr, " -> %s\n", why) ;
  fprintf (stderr, " ->  You may want to check:\n") ;
  fprintf (stderr, " ->  http://www.raspberrypi.org/phpBB3/viewtopic.php?p=184410#p184410\n") ;
  exit (EXIT_FAILURE) ;
}

int piBoardRev (void)
{
  FILE *cpuFd ;
  char line [120] ;
  char *c ;
  static int  boardRev = -1 ;

  if (boardRev != -1)	// No point checking twice
    return boardRev ;

  if ((cpuFd = fopen ("/proc/cpuinfo", "r")) == NULL)
    piBoardRevOops ("Unable to open /proc/cpuinfo") ;

// Start by looking for the Architecture to make sure we're really running
//	on a Pi. I'm getting fed-up with people whinging at me because
//	they can't get it to work on weirdFruitPi boards...

  while (fgets (line, 120, cpuFd) != NULL)
    if (strncmp (line, "Hardware", 8) == 0)
      break ;

  if (strncmp (line, "Hardware", 8) != 0)
    piBoardRevOops ("No hardware line") ;


// See if it's BCM2708 or BCM2709

  if (strstr (line, "BCM2709") != NULL)	// Pi v2 - no point doing anything more at this point
  {
    piModel2 = 1;
    fclose (cpuFd) ;
    return boardRev = 2 ;
  }
  else if (strstr (line, "BCM2708") == NULL)
  {
    fprintf (stderr, "Unable to determine hardware version. I see: %s,\n", line) ;
    fprintf (stderr, " - expecting BCM2708 or BCM2709.\n") ;
    fprintf (stderr, "If this is a genuine Raspberry Pi then please report this\n") ;
    fprintf (stderr, "to projects@drogon.net. If this is not a Raspberry Pi then you\n") ;
    fprintf (stderr, "are on your own as wiringPi is designed to support the\n") ;
    fprintf (stderr, "Raspberry Pi ONLY.\n") ;
    exit (EXIT_FAILURE) ;
  }

// Now do the rest of it as before - we just need to see if it's an older
//	Rev 1 as anything else is rev 2.

// Isolate the Revision line

  rewind (cpuFd) ;
  while (fgets (line, 120, cpuFd) != NULL)
    if (strncmp (line, "Revision", 8) == 0)
      break ;

  fclose (cpuFd) ;

  if (strncmp (line, "Revision", 8) != 0)
    piBoardRevOops ("No \"Revision\" line") ;

// Chomp trailing CR/NL

  for (c = &line [strlen (line) - 1] ; (*c == '\n') || (*c == '\r') ; --c)
    *c = 0 ;


// Scan to the first character of the revision number

  for (c = line ; *c ; ++c)
    if (*c == ':')
      break ;

  if (*c != ':')
    piBoardRevOops ("Bogus \"Revision\" line (no colon)") ;

// Chomp spaces

  ++c ;
  while (isspace (*c))
    ++c ;

  if (!isxdigit (*c))
    piBoardRevOops ("Bogus \"Revision\" line (no hex digit at start of revision)") ;

// Make sure its long enough

  if (strlen (c) < 4)
    piBoardRevOops ("Bogus revision line (too small)") ;

// If you have overvolted the Pi, then it appears that the revision
//	has 100000 added to it!
// The actual condition for it being set is:
//	 (force_turbo || current_limit_override || temp_limit>85) && over_voltage>0


// This test is not correct for the new encoding scheme, so we'll remove it here as
//	we don't really need it at this point.

/********************
  if (wiringPiDebug)
    if (strlen (c) != 4)
      printf ("piboardRev: This Pi has/is (force_turbo || current_limit_override || temp_limit>85) && over_voltage>0\n") ;
*******************/

// Isolate  last 4 characters:

  c = c + strlen (c) - 4 ;


  if ( (strcmp (c, "0002") == 0) || (strcmp (c, "0003") == 0))
    boardRev = 1 ;
  else
    boardRev = 2 ;	// Covers everything else from the B revision 2 to the B+, the Pi v2 and CM's.


  return boardRev ;
}


static inline int i2c_smbus_access (int fd, char rw, uint8_t command, int size, union i2c_smbus_data *data)
{
  struct i2c_smbus_ioctl_data args ;

  args.read_write = rw ;
  args.command    = command ;
  args.size       = size ;
  args.data       = data ;
  return ioctl (fd, I2C_SMBUS, &args) ;
}


/*
 * wiringPiI2CRead:
 *	Simple device read
 *********************************************************************************
 */

int wiringPiI2CRead (int fd)
{
  union i2c_smbus_data data ;

  if (i2c_smbus_access (fd, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data))
    return -1 ;
  else
    return data.byte & 0xFF ;
}


/*
 * wiringPiI2CReadReg8: wiringPiI2CReadReg16:
 *	Read an 8 or 16-bit value from a register on the device
 *********************************************************************************
 */

int wiringPiI2CReadReg8 (int fd, int reg)
{
  union i2c_smbus_data data;

  if (i2c_smbus_access (fd, I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, &data))
    return -1 ;
  else
    return data.byte & 0xFF ;
}

int wiringPiI2CReadReg16 (int fd, int reg)
{
  union i2c_smbus_data data;

  if (i2c_smbus_access (fd, I2C_SMBUS_READ, reg, I2C_SMBUS_WORD_DATA, &data))
    return -1 ;
  else
    return data.word & 0xFFFF ;
}


/*
 * wiringPiI2CWrite:
 *	Simple device write
 *********************************************************************************
 */

int wiringPiI2CWrite (int fd, int data)
{
  return i2c_smbus_access (fd, I2C_SMBUS_WRITE, data, I2C_SMBUS_BYTE, NULL) ;
}


/*
 * wiringPiI2CWriteReg8: wiringPiI2CWriteReg16:
 *	Write an 8 or 16-bit value to the given register
 *********************************************************************************
 */

int wiringPiI2CWriteReg8 (int fd, int reg, int value)
{
  union i2c_smbus_data data ;

  data.byte = value ;
  return i2c_smbus_access (fd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BYTE_DATA, &data) ;
}

int wiringPiI2CWriteReg16 (int fd, int reg, int value)
{
  union i2c_smbus_data data ;

  data.word = value ;
  return i2c_smbus_access (fd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_WORD_DATA, &data) ;
}


/*
 * wiringPiI2CSetupInterface:
 *	Undocumented access to set the interface explicitly - might be used
 *	for the Pi's 2nd I2C interface...
 *********************************************************************************
 */

int wiringPiI2CSetupInterface (const char *device, int devId)
{
  int fd ;

  if ((fd = open (device, O_RDWR)) < 0)
    return -1;

  if (ioctl (fd, I2C_SLAVE, devId) < 0)
    return -2;

  return fd ;
}


/*
 * wiringPiI2CSetup:
 *	Open the I2C device, and regsiter the target device
 *********************************************************************************
 */

int wiringPiI2CSetup (const int devId)
{
  int rev ;
  const char *device ;

  rev = piBoardRev () ;

  if (rev == 1)
    device = "/dev/i2c-0" ;
  else
    device = "/dev/i2c-1" ;

  return wiringPiI2CSetupInterface (device, devId) ;
}
