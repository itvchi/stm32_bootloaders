#include "stm32f4xx_hal.h"
#include "usart.h"
#include "crc.h"
#include "crc32.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FLASH_APP_ADDRESS 0x08008000

#define BL_RX_LEN 200
uint8_t bl_rx_buffer[BL_RX_LEN];

#define ADDR_VALID		0
#define ADDR_INVALID	1

#define SRAM1_SIZE		112*1024
#define SRAM1_END		(SRAM1_BASE + SRAM1_SIZE)
#define SRAM2_SIZE		16*1024
#define SRAM2_END		(SRAM2_BASE + SRAM2_SIZE)
#define SRAM3_SIZE		64*1024
#define SRAM3_END		(SRAM3_BASE + SRAM3_SIZE)
#define SYS_MEM_BASE	0x1FFF0000
#define SYS_MEM_END		0x1FFF77FF

typedef enum {
	BL_GET_VER = 0x51,
	BL_GET_HELP,
	BL_GET_CID,
	BL_GET_RDP_STATUS,
	BL_GO_TO_ADDR,
	BL_FLASH_ERASE,
	BL_MEM_WRITE,
	BL_ENDIS_RW_PROTECT,
	BL_MEM_READ,
	BL_READ_SECTOR_STATUS,
	BL_OPT_READ,
	BL_ACK = 0xA5,
	BL_NACK = 0x7F
} command_t;

command_t supported_commands[] = {
	BL_GET_VER,
	BL_GET_HELP,
	BL_GET_CID,
	BL_GET_RDP_STATUS,
	BL_GO_TO_ADDR,
	BL_FLASH_ERASE,
	BL_MEM_WRITE,
	BL_ENDIS_RW_PROTECT,
	BL_MEM_READ,
	BL_READ_SECTOR_STATUS,
	BL_OPT_READ
};

typedef enum {
	CRC_OK,
	CRC_FAIL
} crc_t;

crc_t bootloader_verify_crc(uint8_t *pData, uint32_t len, uint32_t crc_host)
{
	crc_t rc = CRC_OK;
	uint32_t uwCRCvalue;

	uwCRCvalue = CRC32_ForBytes(pData, len);

	if(uwCRCvalue != crc_host)
		rc = CRC_FAIL;

	printf("BL_DBG_MSG: CRC - host 0x%08lx calculated 0x%08lx\n", crc_host, uwCRCvalue);

	return rc;
}

void bootloader_send_ack(uint8_t command_code, uint8_t follow_length)
{
	uint8_t ack_buf[2] = { BL_ACK, follow_length };
	HAL_UART_Transmit(&huart1, ack_buf, 2, HAL_MAX_DELAY);
}

void bootloader_send_nack()
{
	uint8_t nack = BL_NACK;
	HAL_UART_Transmit(&huart1, &nack, 1, HAL_MAX_DELAY);
}

uint8_t get_bootloader_version()
{
	return 0x11;
}

uint16_t get_mcu_id()
{
	uint16_t cid;
	cid = (uint16_t)(DBGMCU->IDCODE) & 0x0FFF;
	return cid;
}

uint8_t get_flash_rdp_level()
{
	uint8_t rdp;
	rdp = *(uint8_t *)(0x1FFFC001); /* Bits 15:8 of 0x1FFFC000 */
	return rdp;
}

void bootloader_uart_write_data(uint8_t *data, uint32_t length)
{
	HAL_UART_Transmit(&huart1, data, length, HAL_MAX_DELAY);
}

void bootloader_handle_getver_cmd(uint8_t *buff)
{
	uint8_t bl_version;
	uint32_t command_packet_len = buff[0] + 1;
	uint32_t crc_host = *((uint32_t *)(buff + command_packet_len - 4));

	printf("BL_DBG_MSG: %s\n", __FUNCTION__);
	if(bootloader_verify_crc(buff, command_packet_len - 4, crc_host) == CRC_OK)
	{
		printf("BL_DBG_MSG: checksum correct\n");
		bootloader_send_ack(buff[0], 1);
		bl_version = get_bootloader_version();
		printf("BL_DBG_MSG: BL_VER = %d\n", bl_version);
		bootloader_uart_write_data(&bl_version, 1);
	}
	else
	{
		printf("BL_DBG_MSG: checksum incorrect\n");
		bootloader_send_nack();
	}
}

void bootloader_handle_gethelp_cmd(uint8_t *buff)
{
	uint32_t command_packet_len = buff[0] + 1;
	uint32_t crc_host = *((uint32_t *)(buff + command_packet_len - 4));

	printf("BL_DBG_MSG: %s\n", __FUNCTION__);
	if(bootloader_verify_crc(buff, command_packet_len - 4, crc_host) == CRC_OK)
	{
		printf("BL_DBG_MSG: checksum correct\n");
		bootloader_send_ack(buff[0], sizeof(supported_commands));
		bootloader_uart_write_data(supported_commands, sizeof(supported_commands));
	}
	else
	{
		printf("BL_DBG_MSG: checksum incorrect\n");
		bootloader_send_nack();
	}
}

void bootloader_handle_getcid_cmd(uint8_t *buff)
{
	uint16_t cid;
	uint32_t command_packet_len = buff[0] + 1;
	uint32_t crc_host = *((uint32_t *)(buff + command_packet_len - 4));

	printf("BL_DBG_MSG: %s\n", __FUNCTION__);
	if(bootloader_verify_crc(buff, command_packet_len - 4, crc_host) == CRC_OK)
	{
		printf("BL_DBG_MSG: checksum correct\n");
		bootloader_send_ack(buff[0], 2);
		cid = get_mcu_id();
		printf("BL_DBG_MSG: BL_CID = 0x%04x\n", cid);
		bootloader_uart_write_data((uint8_t *)&cid, 2);
	}
	else
	{
		printf("BL_DBG_MSG: checksum incorrect\n");
		bootloader_send_nack();
	}
}

void bootloader_handle_getrdp_cmd(uint8_t *buff)
{
	uint8_t rdp;
	uint32_t command_packet_len = buff[0] + 1;
	uint32_t crc_host = *((uint32_t *)(buff + command_packet_len - 4));

	printf("BL_DBG_MSG: %s\n", __FUNCTION__);
	if(bootloader_verify_crc(buff, command_packet_len - 4, crc_host) == CRC_OK)
	{
		printf("BL_DBG_MSG: checksum correct\n");
		bootloader_send_ack(buff[0], 1);
		rdp = get_flash_rdp_level();
		printf("BL_DBG_MSG: BL_RDP = 0x%02x\n", rdp);
		bootloader_uart_write_data(&rdp, 1);
	}
	else
	{
		printf("BL_DBG_MSG: checksum incorrect\n");
		bootloader_send_nack();
	}
}

uint8_t verify_addr(uint32_t go_addr)
{
	uint8_t rc;

	/* Valid addresses:
	 * flash memory?		YES
	 * system memory? 		YES
	 * sram1?				YES
	 * sram2?				YES
	 * backup sram?			YES
	 * peripheral memory?	NO
	 * external memory?		YES
	 */

	if((go_addr >= FLASH_BASE) && (go_addr < FLASH_END))
		return ADDR_VALID;
	else if((go_addr >= SYS_MEM_BASE) && (go_addr < SYS_MEM_END))
		return ADDR_VALID;
	else if((go_addr >= SRAM1_BASE) && (go_addr < SRAM1_END))
			return ADDR_VALID;
	else if((go_addr >= SRAM2_BASE) && (go_addr < SRAM2_END))
			return ADDR_VALID;
	else if((go_addr >= SRAM3_BASE) && (go_addr < SRAM3_END))
				return ADDR_VALID;
	else if((go_addr >= CCMDATARAM_BASE) && (go_addr < CCMDATARAM_END))
			return ADDR_VALID;
	else
		return ADDR_INVALID;

	return rc;
}

void bootloader_handle_go_cmd(uint8_t *buff)
{
	uint32_t go_addr;
	uint8_t addr_valid = ADDR_VALID;
	uint8_t addr_invalid = ADDR_INVALID;
	uint32_t command_packet_len = buff[0] + 1;
	uint32_t crc_host = *((uint32_t *)(buff + command_packet_len - 4));

	printf("BL_DBG_MSG: %s\n", __FUNCTION__);
	if(bootloader_verify_crc(buff, command_packet_len - 4, crc_host) == CRC_OK)
	{
		printf("BL_DBG_MSG: checksum correct\n");
		bootloader_send_ack(buff[0], 1);

		go_addr = *(uint32_t *)(&buff[2]);
		printf("BL_DBG_MSG: go to address: 0x%08lx\n", go_addr);

		if(verify_addr(go_addr) == ADDR_VALID)
		{
			bootloader_uart_write_data(&addr_valid, 1);

			go_addr += 1; /* Setting T-bit of the address to stay in Thumb mode */
			void (*runApp)(void) = (void *)go_addr;

			printf("BL_DBG_MSG: jumping to the app\n");
			runApp();
		}
		else
		{
			printf("BL_DBG_MSG: invalid address\n");
			bootloader_uart_write_data(&addr_invalid, 1);
		}
	}
	else
	{
		printf("BL_DBG_MSG: checksum incorrect\n");
		bootloader_send_nack();
	}
}

void bootloader_handle_flash_erase_cmd(uint8_t *buff)
{

}

void bootloader_handle_mem_write_cmd(uint8_t *buff)
{

}

void bootloader_handle_endis_rw_protect_cmd(uint8_t *buff)
{

}

void bootloader_handle_mem_read_cmd(uint8_t *buff)
{

}

void bootloader_handle_read_sector_status_cmd(uint8_t *buff)
{

}

void bootloader_handle_opt_read_cmd(uint8_t *buff)
{

}

void bootloader_uart_read_data()
{
	uint8_t rcv_len = 0;

	while(1)
	{
		memset(bl_rx_buffer, 0, BL_RX_LEN);
		/* Receive 1 byte that is "length to follow" field of incomming packet */
		HAL_UART_Receive(&huart1, bl_rx_buffer, 1, HAL_MAX_DELAY);
		rcv_len = bl_rx_buffer[0];
		HAL_UART_Receive(&huart1, &bl_rx_buffer[1], rcv_len, HAL_MAX_DELAY);
		switch(bl_rx_buffer[1])
		{
		case BL_GET_VER:
			bootloader_handle_getver_cmd(bl_rx_buffer);
			break;
		case BL_GET_HELP:
			bootloader_handle_gethelp_cmd(bl_rx_buffer);
			break;
		case BL_GET_CID:
			bootloader_handle_getcid_cmd(bl_rx_buffer);
			break;
		case BL_GET_RDP_STATUS:
			bootloader_handle_getrdp_cmd(bl_rx_buffer);
			break;
		case BL_GO_TO_ADDR:
			bootloader_handle_go_cmd(bl_rx_buffer);
			break;
		case BL_FLASH_ERASE:
			bootloader_handle_flash_erase_cmd(bl_rx_buffer);
			break;
		case BL_MEM_WRITE:
			bootloader_handle_mem_write_cmd(bl_rx_buffer);
			break;
		case BL_ENDIS_RW_PROTECT:
			bootloader_handle_endis_rw_protect_cmd(bl_rx_buffer);
			break;
		case BL_MEM_READ:
			bootloader_handle_mem_read_cmd(bl_rx_buffer);
			break;
		case BL_READ_SECTOR_STATUS:
			bootloader_handle_read_sector_status_cmd(bl_rx_buffer);
			break;
		case BL_OPT_READ:
			bootloader_handle_opt_read_cmd(bl_rx_buffer);
			break;
		default:
			printf("BL_DBG_MSG: Invalid command code.\n");
			break;
		}
	}
}

void bootloader_jump_to_user_app()
{
	void (*app_reset_handler)(void); //function pointer to app reset handler

	/* Configure the MSP value */
	uint32_t msp = *(volatile uint32_t *)FLASH_APP_ADDRESS;
	__set_MSP(msp);

	/* Fetch the rest handler of the application */
	uint32_t reset_handler = *(volatile uint32_t *)(FLASH_APP_ADDRESS + 4);
	app_reset_handler = (void *)(reset_handler);

	/* Jump to reset handler of the application */
	app_reset_handler();
}
