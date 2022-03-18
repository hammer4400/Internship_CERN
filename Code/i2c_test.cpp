/*
 * main.cpp
 *
 *
 * Date: 31/05/2021
 *
 * Description:
 * This code is build upon a code from Paris Maschovakos.
 *
 * The TE0808 carrier board is equipped with a Silicon Labs I²C programmable quad PLL
 * clock generator Si5345 (U35). It's output frequencies can be programmed by using
 * the I²C bus with address 0x69. With the I²C slave device connected to 8-channel
 * I²C Swith U16.
 *
 * Once running, the frequency and other parameters can be changed by programming the
 * device using the I²C-bus connected through I²C switch U16 between the
 * Zynq module (master) and reference clock signal generator (slave).
 *
 *
 * Info + table I2C-interfaces of device slave addresses:
 * https://wiki.trenz-electronic.de/display/PD/TEBF0808+TRM
 *
 * I2C switch datasheet:
 * https://www.ti.com/lit/ds/symlink/tca9548a.pdf?ts=1643262427715&ref_url
 *
 * Si5345 Manual:
 * https://www.skyworksinc.com/-/media/Skyworks/SL/documents/public/reference-manuals/Si5345-44-42-D-RM.pdf
 *

 */

#include <stdio.h>
#include <unistd.h>
#include "i2c.hpp"

#include <string.h>
#include <errno.h>
#include <iostream>
#include <string>



#include <linux/i2c-dev.h>
#include <sys/ioctl.h>


#include <fcntl.h>
#include <stdlib.h>


int main()
{
	int j = 0;
	int k = 0;
	int page = 0;
	int page_val = 0;

	int rx;
	int rt;
	int tx;
	int **data_from_emp_silabs;

	/* Fetching data from Silabs design report to an matrix */
	data_from_emp_silabs = extractData();

	/* Displays data in matrix */
	// show_data(data_from_emp_silabs);

	struct I2cDevice dev;

	/* Set the I2C bus filename and slave address,
	 * "/dev/i2c-13" works! I dont know why and not ex. "/dev/i2c-12"! */
	dev.filename = (char *)"/dev/i2c-13";

	/* On-module Quad programmable PLL clock generator Si5345 (TE0808) [U35] has slave address 0x69 */
	dev.addr = 0x69;

	i2c_start(&dev);

	/* Calculate size of array from EMP_Silabs */
	size_t array_size = size_of_array() - 1;

	for (int i = 0; i < array_size; ++i)
	{

		/* Check if device ready between reading/writing
		 * - This register is repeated on every page*/
		rx = i2c_read_reg(&dev, 0xFE);
		if (rx != 0x0F) {
			printf("[Failure] Device not ready!\n");
			return -1;
		}

		if (i == 0)
		{
			char message[] = "Start configuration preamble";
			section(message);
		}

		/* Delay 1 sec (should be at least 300 msec)
		 * Delay is worst case time for device to complete any calibration that is running
		 * due to device state change previous to this script being processed. */
		if (i == 3)
		{
			sleep(1);
			char message[] = "Start configuration registers";
			section(message);
		}

		if (i == array_size - 5)
		{
			char message[] = "Start configuration postamble";
			section(message);
		}

		/* Fetch data from matrix */
		uint8_t page = data_from_emp_silabs[i][0];
		uint8_t reg = data_from_emp_silabs[i][1];
		uint8_t value = data_from_emp_silabs[i][2];


		/* This switch case selects the page to be written/read to, and allows us to exceed
		 * the slave address 255. Each page gets selected via 'i2c_write_reg(&dev, 0x0<n>01, 0);'
		 * where 'n' is the page we want to select.
		 * Page 0 starts from slave adresss 0. Page 1 from 256. Page 2 from 512 and so on.
		 */
		switch (page)
		{
		case 0:
			i2c_write_reg(&dev, 0x01, 0x00);
			page_val = 1;
			break;
		case 1:
			i2c_write_reg(&dev, 0x01, 0x01);
			page_val = 1;
			break;
		case 2:
			i2c_write_reg(&dev, 0x01, 0x02);
			page_val = 2;
			break;
		case 3:
			i2c_write_reg(&dev, 0x01, 0x03);
			page_val = 3;
			break;
		case 4:
			i2c_write_reg(&dev, 0x01, 0x04);
			page_val = 4;
			break;
		case 5:
			i2c_write_reg(&dev, 0x01, 0x05);
			page_val = 5;
			break;
		case 6:
			i2c_write_reg(&dev, 0x01, 0x06);
			page_val = 6;
			break;
		case 7:
			i2c_write_reg(&dev, 0x01, 0x07);
			page_val = 7;
			break;
		case 8:
			i2c_write_reg(&dev, 0x01, 0x08);
			page_val = 8;
			break;
		case 9:
			i2c_write_reg(&dev, 0x01, 0x09);
			page_val = 9;
			break;
		case 10:
			i2c_write_reg(&dev, 0x01, 0x0A);
			page_val = 10;
			break;
		case 11:
			i2c_write_reg(&dev, 0x01, 0x0B);
			page_val = 11;
			break;
		default:
			printf("[Failure] page not found! \n");
			return -1;

		}

		rx = i2c_read_reg(&dev, reg);

		if (rx == value)
		{
			printf("[Overwrote]\t %d\t 0x%X\t\t 0x%X\t   0x%X\t\t", page_val, reg, rx, value);
			tx = i2c_write_reg(&dev, reg, value);
			rx = i2c_read_reg(&dev, reg);
			if (rx == value)
			{
				printf("[Success]\n");
			}
			else
			{
				printf("[Failed]\n");
			}
			k++;
		}
		else
		{
			printf("  [Wrote]\t %d\t\t0x%X\t\t 0x%X\t   0x%X\t\t", page_val, reg, rx, value);
			tx = i2c_write_reg(&dev, reg, value);
			rx = i2c_read_reg(&dev, reg);
			if (rx == value)
			{
				printf("[Success]\n");
			}
			else
			{
				printf("[Failed]\n");
			}
			j++;
		}
	}

	printf("\n\nTotal registers written to is: %d\n", j);
	printf("Total overwritten registers is: %d\n", k);
	printf("\nfinished writing to registers.\n");

	i2c_stop(&dev);
	return 0;

}