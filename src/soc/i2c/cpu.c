/*
 * $Id: cpu.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MPC8548 cpu I2C Device Driver.
 *
 */

#include <sal/types.h>
#include <shared/bsl.h>
#include <soc/drv.h>
#include <soc/i2c.h>

#if defined(__DUNE_GTO_BCM_CPU__) || defined(__DUNE_WRX_BCM_CPU__) || defined (BCM_CALADAN3_SVK)

#ifdef __VXWORKS__
#include <sal/appl/io.h>
#include <sal/appl/i2c.h>
#endif /* __VXWORKS__ */

#if (defined(LINUX) || defined(UNIX))


#ifndef __KERNEL__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>          /* open */
#include <unistd.h>         /* exit */
#include <sys/ioctl.h>      /* ioctl */
#include <linux/i2c-dev.h>
#ifndef I2C_SMBUS_READ
  #include <linux/i2c.h>
#endif

typedef struct {
  int   file_desc;
  int   alen;       /* CPU_I2C_ALEN_* */
  char  chip;       /* 7 bit address of the device */
  int   addr;
  int   data;
} cpu_i2c_access_t;


int cpu_i2c_init(int* p_fd, char dev_num)
{
  int ret = 0;
  char dev_file_name[CPU_I2C_MAX_NAME_LENGTH];

  sprintf(dev_file_name, CPU_I2C_DEVICE_FILE_NAME_PREFIX"%d", (int)dev_num);
  
  /* Try to open the given i2c dev num */
  if ( ((*p_fd) = open(dev_file_name, O_RDWR)) < 0)
  {
      LOG_INFO(BSL_LS_SOC_I2C,
               (BSL_META("ERROR in %s: Failed to open() I2C device num (%d)\n"),
                FUNCTION_NAME(), (int)dev_num));
    ret = 1;
    goto exit;
  }

exit:
  return ret;
}

int cpu_i2c_deinit(int fd)
{
  int ret = 0;
  
  /* close the I2c device */
  if (fd > 0)
  {
    close(fd);
  }
  
  return ret;
}

int cpu_i2c_read_int(cpu_i2c_access_t* p_i2c_access)
{
  int ret = 0;
  char result[4], addr[4];
  struct i2c_rdwr_ioctl_data rw_data;
  struct i2c_msg i2c_rw_msg[2];

  /* init the i2c access struct */
  rw_data.msgs  = &i2c_rw_msg[0];
  rw_data.nmsgs = 2;                 /* WRITE + READ */
  
  i2c_rw_msg[0].addr    = i2c_rw_msg[1].addr = p_i2c_access->chip;
  i2c_rw_msg[0].flags   = 0;                                            /* I2C_M_WR */
  i2c_rw_msg[0].buf     = (unsigned char*)&addr[0];
  i2c_rw_msg[1].flags   = I2C_M_RD;
  i2c_rw_msg[1].buf     = (unsigned char*)&result[0];


  switch(p_i2c_access->alen)
  {
  case CPU_I2C_ALEN_NONE_DLEN_WORD: /* No internal adress */
	  i2c_rw_msg[0].flags = I2C_M_RD;
	  i2c_rw_msg[0].len = 2;
	  i2c_rw_msg[0].buf = (unsigned char*)&result[0];

	  rw_data.nmsgs = 1;

	  while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
		  LOG_INFO(BSL_LS_SOC_I2C,
                           (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                     "- error reading from chip 0x%02x, "
                                     "address 0x%02x, error = %d\n"),
                            __FILE__, __LINE__, FUNCTION_NAME(),
                            p_i2c_access->chip, p_i2c_access->addr, errno));
		  if (errno != EINTR) {
			  ret = 1;
			  goto exit;
		  }
	  }
          p_i2c_access->data = (result[1] & 0xff) | ((result[0] & 0xff) << 8);
	  break;

  case CPU_I2C_ALEN_NONE_DLEN_BYTE: /* No internal adress */
	  i2c_rw_msg[0].flags = I2C_M_RD;
	  i2c_rw_msg[0].len = 1;
	  i2c_rw_msg[0].buf = (unsigned char*)&result[0];

	  rw_data.nmsgs = 1;

	  while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
		  LOG_INFO(BSL_LS_SOC_I2C,
                           (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                     "- error reading from chip 0x%02x, "
                                     "address 0x%02x, error = %d\n"),
                            __FILE__, __LINE__, FUNCTION_NAME(),
                            p_i2c_access->chip, p_i2c_access->addr, errno));
		  if (errno != EINTR) {
			  ret = 1;
			  goto exit;
		  }
	  }

	  p_i2c_access->data = result[0] & 0xff;

	  break;

  case CPU_I2C_ALEN_BYTE_DLEN_BYTE: /* address is 1 byte long */

          /* take only 1 byte of the input address */
          addr[0] = p_i2c_access->addr & 0xff; 
      
          i2c_rw_msg[0].len = i2c_rw_msg[1].len = 1;

          while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
              LOG_INFO(BSL_LS_SOC_I2C,
                       (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                 "- error reading from chip 0x%02x, "
                                 "address 0x%02x, error = %d\n"),
                        __FILE__, __LINE__, FUNCTION_NAME(),
                        p_i2c_access->chip, p_i2c_access->addr, errno));
              if (errno != EINTR) {
                  ret = 1;
                  goto exit;
              }
          }

          /* Data is 1 char long*/
          p_i2c_access->data = result[0] & 0xff;
          break;
        
      case CPU_I2C_ALEN_WORD_DLEN_WORD: /* address is word (2 bytes) long */
          
          /* build address according to endianess */
          addr[0] = (p_i2c_access->addr >> 8)& 0xff;
          addr[1] = p_i2c_access->addr & 0xff;
          
          i2c_rw_msg[0].len = i2c_rw_msg[1].len = 2;

          while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
              LOG_INFO(BSL_LS_SOC_I2C,
                       (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                 "- error reading from chip 0x%02x, "
                                 "address 0x%02x, error = %d\n"),
                        __FILE__, __LINE__, FUNCTION_NAME(),
                        p_i2c_access->chip, p_i2c_access->addr, errno));
              if (errno != EINTR) {
                  ret = 1;
                  goto exit;
              }
          }

          /* data is word (2 bytes) long */
          p_i2c_access->data = (result[1] & 0xff) | ((result[0] & 0xff) << 8);
          break;

  case CPU_I2C_ALEN_BYTE_DLEN_LONG: /* address is 1 bytes long, data is 4 byte long*/
	  /* build address according to endianess */
	  addr[0] = p_i2c_access->addr & 0xff;
	  
	  i2c_rw_msg[0].len = 1;
	  i2c_rw_msg[1].len = 4;

	  while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
		  LOG_INFO(BSL_LS_SOC_I2C,
                           (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                     "- error reading from chip 0x%x, "
                                     "address 0x%x, error = %d\n"),
                            __FILE__, __LINE__, FUNCTION_NAME(),
                            p_i2c_access->chip, p_i2c_access->addr, errno));
		  if (errno != EINTR) {
			  ret = 1;
			  goto exit;
		  }
	  }
	  p_i2c_access->data = (*((int*)&result[0]));
	  break;

    case CPU_I2C_ALEN_WORD_DLEN_LONG: /* address is 1 bytes long, data is 4 byte long*/
	  /* build address according to endianess */
      addr[0] = (p_i2c_access->addr >> 8)& 0xff;
      addr[1] = p_i2c_access->addr & 0xff;
	  
	  i2c_rw_msg[0].len = 2;
	  i2c_rw_msg[1].len = 4;

	  while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
		  LOG_INFO(BSL_LS_SOC_I2C,
                           (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                     "- error reading from chip 0x%x, "
                                     "address 0x%x, error = %d\n"),
                            __FILE__, __LINE__, FUNCTION_NAME(),
                            p_i2c_access->chip, p_i2c_access->addr, errno));
		  if (errno != EINTR) {
			  ret = 1;
			  goto exit;
		  }
	  }
	  p_i2c_access->data = (*((int*)&result[0]));
	  break;

  case CPU_I2C_ALEN_LONG_DLEN_LONG: /* address is 4 bytes long */

          /* build address according to endianess */
          addr[0] = (p_i2c_access->addr >> 24)& 0xff;
          addr[1] = (p_i2c_access->addr >> 16)& 0xff;
          addr[2] = (p_i2c_access->addr >> 8)& 0xff;
          addr[3] = p_i2c_access->addr & 0xff;
          
          i2c_rw_msg[0].len = i2c_rw_msg[1].len = 4;

          while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
              LOG_INFO(BSL_LS_SOC_I2C,
                       (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                 "- error reading from chip 0x%x, "
                                 "address 0x%x, error = %d\n"),
                        __FILE__, __LINE__, FUNCTION_NAME(),
                        p_i2c_access->chip, p_i2c_access->addr, errno));
              if (errno != EINTR) {
                  ret = 1;
                  goto exit;
              }
          }

          p_i2c_access->data = (*((int*)&result[0]));
          break;

      case CPU_I2C_ALEN_BYTE_DLEN_WORD: /* address is 1 bytes long, data is 2 bytes long */

          addr[0] = p_i2c_access->addr & 0xff;
          
          i2c_rw_msg[0].len = 1;
          i2c_rw_msg[1].len = 2;

          while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
              LOG_INFO(BSL_LS_SOC_I2C,
                       (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                 "- error reading from chip 0x%x, "
                                 "address 0x%x, error = %d\n"),
                        __FILE__, __LINE__, FUNCTION_NAME(),
                        p_i2c_access->chip, p_i2c_access->addr, errno));
              if (errno != EINTR) {
                  ret = 1;
                  goto exit;
              }
          }
          
          p_i2c_access->data = (result[1] & 0xff) | ((result[0] & 0xff) << 8);
          break;

      default:

          LOG_INFO(BSL_LS_SOC_I2C,
                   (BSL_META("error - unsupported address length given = %d"),
                    p_i2c_access->alen));
          ret = 1;
          goto exit;
          break;
    } /* switch*/
  
exit:
  return ret;
}

int cpu_i2c_write_int(cpu_i2c_access_t* p_i2c_access)
{
    char addr_n_data[8];
    int ret = 0;
    struct i2c_rdwr_ioctl_data rw_data;
    struct i2c_msg i2c_rw_msg;
    
    /* LOG_INFO(BSL_LS_SOC_I2C,
                (BSL_META("%s(): p_i2c_access->alen=%x, p_i2c_access->chip=%x, "
                          "p_i2c_access->addr=%x, p_i2c_access->data=%x\n"),
                 FUNCTION_NAME(), p_i2c_access->alen, p_i2c_access->chip,
                 p_i2c_access->addr, p_i2c_access->data)); */
    
    /* init the i2c access struct */
    rw_data.msgs = &i2c_rw_msg;
    rw_data.nmsgs = 1; /* 1 WRITE */
    
    i2c_rw_msg.addr = p_i2c_access->chip;
    i2c_rw_msg.flags = 0; /* I2C_M_WR */
    i2c_rw_msg.buf = (unsigned char*)&addr_n_data[0];
  
    switch (p_i2c_access->alen)
    {
	case CPU_I2C_ALEN_NONE_DLEN_BYTE: /* No internal adress */
		i2c_rw_msg.len = 1;
		addr_n_data[0] = p_i2c_access->data & 0xff;

		while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
			LOG_INFO(BSL_LS_SOC_I2C,
                                 (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                           "- error to write to chip 0x%x, "
                                           "address 0x%x, error = %d\n"),
                                  __FILE__, __LINE__, FUNCTION_NAME(),
                                  p_i2c_access->chip, p_i2c_access->addr, errno));
			if (errno != EINTR) {
				ret = 1;
				goto exit;
			}
		}

		break;

	case CPU_I2C_ALEN_BYTE_DLEN_BYTE: /* address is 1 byte long */
		
          /* 1 byte for address + 1 byte for data */
          i2c_rw_msg.len = 2; 
          addr_n_data[0] = p_i2c_access->addr & 0xff;
          addr_n_data[1] = p_i2c_access->data & 0xff;

          while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
              LOG_INFO(BSL_LS_SOC_I2C,
                       (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                 "- error to write to chip 0x%x, "
                                 "address 0x%x, error = %d\n"),
                        __FILE__, __LINE__, FUNCTION_NAME(),
                        p_i2c_access->chip, p_i2c_access->addr, errno));
              if (errno != EINTR) {
                  ret = 1;
                  goto exit;
              }
          }

          break;

        case CPU_I2C_ALEN_WORD_DLEN_WORD: /* address is 2 bytes long */

            i2c_rw_msg.len = 4; /* 2 bytes for address + 2 bytes for data */
            /* insert the address bytes followed by the data bytes to the addr_n_data byte array */
            addr_n_data[0] = (p_i2c_access->addr >> 8)& 0xff;
            addr_n_data[1] = p_i2c_access->addr & 0xff;
            addr_n_data[2] = (p_i2c_access->data >> 8)& 0xff;
            addr_n_data[3] = p_i2c_access->data & 0xff;

            while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
                LOG_INFO(BSL_LS_SOC_I2C,
                         (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                   "- error to write to chip 0x%x, "
                                   "address 0x%x, error = %d\n"),
                          __FILE__, __LINE__, FUNCTION_NAME(),
                          p_i2c_access->chip, p_i2c_access->addr, errno));
                if (errno != EINTR) {
                    ret = 1;
                    goto exit;
                }
            }

            break;

	case CPU_I2C_ALEN_BYTE_DLEN_LONG: /* address is 1 bytes long, data is 4 byte long */
		i2c_rw_msg.len = 5;

		/* insert the address bytes followed by the data bytes to the addr_n_data byte array */
		addr_n_data[0] = p_i2c_access->addr & 0xff;
		addr_n_data[1] = (p_i2c_access->data >> 24)& 0xff;
		addr_n_data[2] = (p_i2c_access->data >> 16)& 0xff;
		addr_n_data[3] = (p_i2c_access->data >> 8)& 0xff;
		addr_n_data[4] = p_i2c_access->data & 0xff;

		while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
			LOG_INFO(BSL_LS_SOC_I2C,
                                 (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                           "- error to write to chip 0x%x, "
                                           "address 0x%x, error = %d\n"),
                                  __FILE__, __LINE__, FUNCTION_NAME(),
                                  p_i2c_access->chip, p_i2c_access->addr, errno));
			if (errno != EINTR) {
				ret = 1;
				goto exit;
			}
		}
		break;

	case CPU_I2C_ALEN_LONG_DLEN_LONG: /* address is 4 bytes long */

            i2c_rw_msg.len = 8; /* 4 byte for address + 4 byte for data */
            
            /* insert the address bytes followed by the data bytes to the addr_n_data byte array */
            addr_n_data[0] = (p_i2c_access->addr >> 24)& 0xff;
            addr_n_data[1] = (p_i2c_access->addr >> 16)& 0xff;
            addr_n_data[2] = (p_i2c_access->addr >> 8)& 0xff;
            addr_n_data[3] = p_i2c_access->addr & 0xff;
            addr_n_data[4] = (p_i2c_access->data >> 24)& 0xff;
            addr_n_data[5] = (p_i2c_access->data >> 16)& 0xff;
            addr_n_data[6] = (p_i2c_access->data >> 8)& 0xff;
            addr_n_data[7] = p_i2c_access->data & 0xff;

            while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
              LOG_INFO(BSL_LS_SOC_I2C,
                       (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                 "- error to write to chip 0x%x, "
                                 "address 0x%x, error = %d\n"),
                        __FILE__, __LINE__, FUNCTION_NAME(),
                        p_i2c_access->chip, p_i2c_access->addr, errno));
              if (errno != EINTR) {
                  ret = 1;
                  goto exit;
              }
            
            }
            break;

        case CPU_I2C_ALEN_WORD_DLEN_LONG: /* address is 2 bytes long, data is 4 bytes long */
            i2c_rw_msg.len = 6;
            /* insert the address bytes followed by the data bytes to the addr_n_data byte array */
            addr_n_data[0] = (p_i2c_access->addr >> 8)& 0xff;
            addr_n_data[1] = p_i2c_access->addr & 0xff;
            addr_n_data[2] = (p_i2c_access->data >> 24)& 0xff;
            addr_n_data[3] = (p_i2c_access->data >> 16)& 0xff;
            addr_n_data[4] = (p_i2c_access->data >> 8)& 0xff;
            addr_n_data[5] = p_i2c_access->data & 0xff;
/*         
            while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
                LOG_INFO(BSL_LS_SOC_I2C,
                         (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                   "- error to write to chip 0x%x, "
                                   "address 0x%x, error = %d\n"),
                          __FILE__, __LINE__, FUNCTION_NAME(),
                          p_i2c_access->chip, p_i2c_access->addr, errno));
                if (errno != EINTR) {
                    ret = 1;
                    goto exit;
                }
            }
*/
            ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data);
            break;
        case CPU_I2C_ALEN_BYTE_DLEN_WORD: /* address is 1 bytes long, data is 2 bytes long */


            i2c_rw_msg.len = 3;
            /* insert the address bytes followed by the data bytes to the addr_n_data byte array */
            addr_n_data[0] = (p_i2c_access->addr)& 0xff;
            addr_n_data[1] = (p_i2c_access->data >> 8)& 0xff;
            addr_n_data[2] = p_i2c_access->data & 0xff;

            while (ioctl(p_i2c_access->file_desc, I2C_RDWR, &rw_data) < 0) {
                LOG_INFO(BSL_LS_SOC_I2C,
                         (BSL_META("ioctl I2C_RDWR failed in %s, %d, %s "
                                   "- error to write to chip 0x%x, "
                                   "address 0x%x, error = %d\n"),
                          __FILE__, __LINE__, FUNCTION_NAME(),
                          p_i2c_access->chip, p_i2c_access->addr, errno));
                if (errno != EINTR) {
                    ret = 1;
                    goto exit;
                }
            }
   
            break;
        default:
            LOG_INFO(BSL_LS_SOC_I2C,
                     (BSL_META("error - unsupported address length given = %d"),
                      p_i2c_access->alen));
            ret = 1;
            goto exit;
            break;
    }
  
exit:
  return ret;
}


int cpu_i2c_write(int chip, int addr, CPU_I2C_BUS_LEN alen, int val)
{
  int ret = 0;
  int dev_num;
  cpu_i2c_access_t i2c_access;

  /* get the i2c dev number */
  dev_num = CPU_I2C_DEV_NUM_DEFAULT;

  /* open the given i2c dev num */
  if (cpu_i2c_init(&i2c_access.file_desc, dev_num) != 0)
  {
      LOG_INFO(BSL_LS_SOC_I2C,
               (BSL_META("ERROR in %s: cpu_i2c_init() returned with error.\n"),
                FUNCTION_NAME()));
    return 1;
  }
  LOG_INFO(BSL_LS_SOC_I2C,
           (BSL_META("cpu_i2c_write: bus:%d dev:@%02x reg:%0x data:%x\n"),
            dev_num, chip, addr, val));


  i2c_access.chip = chip;
  i2c_access.alen = alen;
  i2c_access.addr = addr;
  i2c_access.data = val;

  ret = cpu_i2c_write_int(&i2c_access);
  if (ret)
  {
      LOG_INFO(BSL_LS_SOC_I2C,
               (BSL_META("Error in %s: cpu_i2c_write() returned with error.\n"),
                FUNCTION_NAME()));
    ret = 1;
    goto exit;
  }

exit:
  cpu_i2c_deinit(i2c_access.file_desc);
  return ret;
}

int cpu_i2c_read(int chip, int addr, CPU_I2C_BUS_LEN alen, int* p_val)
{
  int ret = 0;
  int dev_num;
  cpu_i2c_access_t i2c_access;

  /* get the i2c dev number */
  dev_num = CPU_I2C_DEV_NUM_DEFAULT;

  /* open the given i2c dev num */
  if (cpu_i2c_init(&i2c_access.file_desc, dev_num) != 0)
  {
      LOG_INFO(BSL_LS_SOC_I2C,
               (BSL_META("ERROR in %s: cpu_i2c_init() returned with error.\n"),
                FUNCTION_NAME()));
    return 1;
  }

  i2c_access.chip = chip;
  i2c_access.alen = alen;
  i2c_access.addr = addr;
  /* LOG_INFO(BSL_LS_SOC_I2C,
              (BSL_META("\nRead %x addr %x len %d"), chip, addr, alen));*/
  LOG_INFO(BSL_LS_SOC_I2C,
           (BSL_META("cpu_i2c_read: bus:%d dev:@%02x reg:%0x\n"),
            dev_num, chip, addr));

  ret = cpu_i2c_read_int(&i2c_access);
  if (ret)
  {
      LOG_INFO(BSL_LS_SOC_I2C,
               (BSL_META("Error in %s: cpu_i2c_read() returned with error.\n"),
                FUNCTION_NAME()));
    ret = 1;
    goto exit;
  }
  *p_val = i2c_access.data;
  LOG_INFO(BSL_LS_SOC_I2C,
           (BSL_META("cpu_i2c_read: bus:%d dev:@%02x reg:%0x data:%x\n"),
            dev_num, chip, addr, *p_val));

exit:
  cpu_i2c_deinit(i2c_access.file_desc);
  return ret;
}

/*
 * Function: cpu_i2c_device_present
 * Purpose: Probe the I2C bus using saddr, report if a device responds.
 *          The I2C bus is released upon return.
 *          No further action is taken.
 *
 * Parameters:
 *    saddr - I2C slave address
 *
 * Return:
 *     SOC_E_NONE - An I2C device responds at the indicated saddr.
 *     otherwise  - No I2C response at the indicated saddr, or I/O error.
 */
int
cpu_i2c_device_present(int bus, i2c_saddr_t saddr) {

  struct i2c_smbus_ioctl_data smbus_data;
  int dev_num, ret;
  cpu_i2c_access_t i2c_access;

  dev_num = CPU_I2C_DEV_NUM_DEFAULT;

  /* open the given i2c dev num */
  if (cpu_i2c_init(&i2c_access.file_desc, dev_num) != 0)
  {
      LOG_INFO(BSL_LS_SOC_I2C,
               (BSL_META("ERROR in %s: cpu_i2c_init() returned with error.\n"),
                FUNCTION_NAME()));
    return SOC_E_CONFIG;
  }

  if (ioctl(i2c_access.file_desc, I2C_SLAVE, saddr) < 0) {
      /* ERROR HANDLING; you can check errno to see what went wrong */
      ret = SOC_E_NOT_FOUND;
      goto exit;
  }

  sal_memset(&smbus_data, 0, sizeof(smbus_data));
  smbus_data.read_write = I2C_SMBUS_QUICK;
  smbus_data.size = I2C_SMBUS_QUICK;

  if (ioctl(i2c_access.file_desc, I2C_SMBUS, &smbus_data) < 0) {
      /* ERROR HANDLING; you can check errno to see what went wrong */
     ret = SOC_E_NOT_FOUND;
  } else {
     ret = SOC_E_NONE;
  }
exit:
  cpu_i2c_deinit(i2c_access.file_desc);
  return ret;
}

/*
 * Function: cpu_i2c_block_read
 * Purpose: Read a block of data from I2c device
 *
 * Parameters:
 *    bus  - cpu controller to use
 *    saddr  - slave address
 *    addr -  interal starting address, only 8bit addressing as of now
 *    data  - pointer to buffer
 *    len   - len of data to be read
 * Return:
 *     SOC_E_NONE - An I2C device responds at the indicated saddr.
 *     otherwise  - No I2C response at the indicated saddr, or I/O error.
 */
int
cpu_i2c_block_read(int bus, i2c_saddr_t saddr, 
                   uint8 reg, uint8 *data, uint8 *len)
{

    int fd, rv;
    struct i2c_rdwr_ioctl_data rw_data;
    struct i2c_msg i2c_rw_msg[2];

    /* Open the device */
    if (cpu_i2c_init(&fd, bus) != 0)
    {
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("ERROR in %s: cpu_i2c_init() returned with error.\n"),
                  FUNCTION_NAME()));
        return SOC_E_CONFIG;
    }
    if (*len == 0) {
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("ERROR in cpu_i2c_block_read: invalid len specified (%d)"),
                  *len));
        return SOC_E_PARAM;
    }
  
    /* init the i2c access struct */
    rw_data.msgs = i2c_rw_msg;
    rw_data.nmsgs = 2; 
    
    i2c_rw_msg[0].addr = saddr;
    i2c_rw_msg[0].flags = 0; 
    i2c_rw_msg[0].buf = &reg;
    i2c_rw_msg[0].len = 1;
  
    i2c_rw_msg[1].addr = saddr;
    i2c_rw_msg[1].flags = I2C_M_RD; 
    i2c_rw_msg[1].buf = data;
    i2c_rw_msg[1].len = *len;

    rv = ioctl(fd, I2C_RDWR, &rw_data);

    cpu_i2c_deinit(fd);

    if (rv < 0) {
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("\nFailed reading %d bytes data from %x at device saddr %x"),
                  *len, reg, saddr));
        return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

/*
 * Function: cpu_i2c_block_write
 * Purpose: Write a block of data from I2c device
 *
 * Parameters:
 *    bus  - cpu controller to use
 *    saddr  - slave address
 *    addr -  interal starting address, only 8bit addressing as of now
 *    data  - pointer to buffer
 *    len   - len of data to be read
 * Return:
 *     SOC_E_NONE - An I2C device responds at the indicated saddr.
 *     otherwise  - No I2C response at the indicated saddr, or I/O error.
 */
int
cpu_i2c_block_write(int bus, i2c_saddr_t saddr, 
                   uint8 reg, uint8 *data, uint8 len)
{

    int fd, rv;
    struct i2c_rdwr_ioctl_data rw_data;
    struct i2c_msg i2c_rw_msg[2];

    /* Open the device */
    if (cpu_i2c_init(&fd, bus) != 0)
    {
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("ERROR in %s: cpu_i2c_init() returned with error.\n"),
                  FUNCTION_NAME()));
        return SOC_E_CONFIG;
    }
  
    /* init the i2c access struct */
    rw_data.msgs = i2c_rw_msg;
    rw_data.nmsgs = 2; 
    
    i2c_rw_msg[0].addr = saddr;
    i2c_rw_msg[0].flags = 0; 
    i2c_rw_msg[0].buf = &reg;
    i2c_rw_msg[0].len = 1;
  
    i2c_rw_msg[1].addr = saddr;
    i2c_rw_msg[1].flags = I2C_M_NOSTART; 
    i2c_rw_msg[1].buf = data;
    i2c_rw_msg[1].len = len;

    rv = ioctl(fd, I2C_RDWR, &rw_data);

    cpu_i2c_deinit(fd);

    if (rv < 0) {
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("\nFailed reading %d bytes data from %x at device saddr %x"),
                  len, reg, saddr));
        return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

#else
int cpu_i2c_write(int chip, int addr, CPU_I2C_BUS_LEN alen, int val)
{
    
    LOG_INFO(BSL_LS_SOC_I2C,
             (BSL_META("%s: not implemented for KERNEL mode \n"),
              FUNCTION_NAME()));
    return -1;
}

int cpu_i2c_read(int chip, int addr, CPU_I2C_BUS_LEN alen, int* p_val)
{
    
    LOG_INFO(BSL_LS_SOC_I2C,
             (BSL_META("%s: not implemented for KERNEL mode \n"),
              FUNCTION_NAME()));
    return -1;
}
#endif /* __KERNEL__ */
#endif /* LINUX */

#ifdef __VXWORKS__

#define I2C_CTRL 1 /* I2C Controller number */

int cpu_i2c_write(int chip, int addr, CPU_I2C_BUS_LEN alen, int val)
{
    int     rv;
    uint8   addr_len, buf_len;
    uint8   buf[4];

    LOG_VERBOSE(BSL_LS_SOC_I2C,
                (BSL_META("chip:%d addr:%x alen:%d val:%d \n"), 
                 chip, addr, alen, val));

    switch (alen) {
    case CPU_I2C_ALEN_BYTE_DLEN_BYTE:
        addr_len = buf_len = 1;
        break;
    case CPU_I2C_ALEN_LONG_DLEN_LONG:
        addr_len = buf_len = 4;
        break;
    case CPU_I2C_ALEN_BYTE_DLEN_LONG:
        addr_len = 1;
        buf_len = 4;
        break;
    default:
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("%s: alen:%x is not supported\n"),
                  FUNCTION_NAME(), alen));
        return SOC_E_INTERNAL;
    }
    switch (buf_len) {
    case 1:
        buf[0] = val & 0xff;
        break;
    case 4:
        buf[0] = (val >> 24) & 0xff;
        buf[1] = (val >> 16) & 0xff;
        buf[2] = (val >> 8) & 0xff;
        buf[3] = val & 0xff;
        break;
    default:
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("%s: buf_len:%x not supported\n"),
                  FUNCTION_NAME(), 
                  buf_len));
        return SOC_E_INTERNAL;
    }

    rv = sal_i2c_write(I2C_CTRL, (UINT32) chip, (unsigned int) addr, addr_len,
                       buf, buf_len);
    if (rv) {
        sal_printf("%s: failed. rv:%d \n", FUNCTION_NAME(), rv);
        return SOC_E_INTERNAL;
    }
    LOG_VERBOSE(BSL_LS_SOC_I2C,
                (BSL_META("success. wrote val:%x to addr:%x\n"),
                 val, addr));
    return SOC_E_NONE;
}

int cpu_i2c_read(int chip, int addr, CPU_I2C_BUS_LEN alen, int* p_val)
{
    int     rv;
    uint8   addr_len, buf_len;
    uint8   buf[4];
    
    LOG_VERBOSE(BSL_LS_SOC_I2C,
                (BSL_META("chip:%d addr:%x alen:%d \n"),
                 chip, addr, alen));
    if (p_val == NULL) {
        return SOC_E_PARAM;
    }
    *p_val = 0;

    switch (alen) {
    case CPU_I2C_ALEN_BYTE_DLEN_BYTE:
        addr_len = buf_len = 1;
        break;
    case CPU_I2C_ALEN_LONG_DLEN_LONG:
        addr_len = buf_len = 4;
        break;
    case CPU_I2C_ALEN_BYTE_DLEN_LONG:
        addr_len = 1;
        buf_len = 4;
        break;
    default:
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("%s: alen:%x is not supported\n"),
                  FUNCTION_NAME(), alen));
        return SOC_E_INTERNAL;
    }
    rv = sal_i2c_read(I2C_CTRL, (UINT32) chip, (unsigned int)addr, addr_len, 
                      buf, buf_len);
    if (rv) {
        sal_printf("%s: failed. rv:%d \n", FUNCTION_NAME(), rv);
        return SOC_E_INTERNAL;
    }       

    switch (buf_len) {
    case 1:
        *p_val = buf[0];
        break;
    case 4:
        *p_val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
        break;
    default:
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META("%s: buf_len:%x not supported\n"),
                  FUNCTION_NAME(), buf_len));
        return SOC_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_SOC_I2C,
                (BSL_META("success chip:%d addr:%x alen:%d read-val:0x%0x \n"), 
                 chip, addr, alen, *p_val));
    return SOC_E_NONE;
}

int
cpu_i2c_device_present(int bus, i2c_saddr_t saddr) 
{
    return SOC_E_UNAVAIL;
}

int
cpu_i2c_block_write(int bus, i2c_saddr_t saddr, 
                   uint8 reg, uint8 *data, uint8 len)
{
    return SOC_E_UNAVAIL;
}
int
cpu_i2c_block_read(int bus, i2c_saddr_t saddr, 
                   uint8 reg, uint8 *data, uint8 *len)
{
    return SOC_E_UNAVAIL;
}
#endif /* __VXWORKS__ */

#else
int _src_soc_i2c_cpu_c_not_empty;
#endif  /* __DUNE_GTO_BCM_CPU_ */
