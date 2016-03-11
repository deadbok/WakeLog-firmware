/** @file FlasLog.h
 * 
 * @brief Persistent circular log buffer using flash.
 * 
 * @copyright
 * Copyright 2015 Martin Bo Kristensen Grønholdt <oblivion@@ace2>
 * 
 * @license
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#ifndef FLASHLOG_H
#define FLASHLOG_H

/**
 * @brief Ring buffer in ESP system parameter area.
 */
template <typename T> class FlashLog
{
    public:
        FlashLog(uint32_t addr)
        {
			debugf("Creating ring buffer.");
			this->addr = addr;
			//Read from flash.
			if (SPI_FLASH_RESULT_OK != spi_flash_read(this->addr << 0xc, (uint32_t *)this->data, this->bytes))
			{
				SYSTEM_ERROR("ÉRROR reading flash.");
			}

			this->count = (uint16_t *)this->data;
			this->head = (uint16_t *)(this->data + sizeof(uint16_t));
			this->tail = (uint16_t *)(this->data + (sizeof(uint16_t) * 2));
			this->sig = (uint16_t *)(this->data + (sizeof(uint16_t) * 3));
			debugf(" Signature: 0x%x.", *this->sig);
			
			if ((*this->sig) != this->header_sig)
			{
				debugf(" Signature mismatch (0x%x), initialising data.", (*this->sig));
				debugf(" Items: %d.", *this->count);
				debugf(" Head: %d.", *this->head);
				debugf(" Tail: %d.", *this->tail);
				memset(this->data, 0, this->bytes);
				(*this->head) = this->header_bytes;
				(*this->tail) = this->header_bytes;
				(*this->sig) = this->header_sig;
				debugf(" Signature 0x%x.", (*this->sig));
				debugf(" Erasing sector 0x%x.", this->addr);
				if (SPI_FLASH_RESULT_OK != spi_flash_erase_sector(this->addr))
				{
					SYSTEM_ERROR("Erasing flash.");
				}
				debugf(" Saving to flash at 0x%x.", this->addr << 0xc);
				if (SPI_FLASH_RESULT_OK != spi_flash_write(this->addr << 0xc, (uint32_t *)this->data, this->bytes))
				{
					SYSTEM_ERROR("Writing flash.");
				}
			}
				
			debugf("Ring buffer created.");
			debugf(" Capacity: %d.", this->capacity);
		    debugf(" Items: %d.", *this->count);
			debugf(" Head: %d.", *this->head);
			debugf(" Tail: %d.", *this->tail);
		};
		unsigned short getCount(void)
		{
			return(*this->count);
		};
		/**
		 * @brief Add a new item at the end of the buffer.
		 * 
		 * @param time Time to store.
		 */
		bool pushBack(T time)
		{
			uint16_t new_tail;
			
			debugf("Adding item to ring buffer.");
			
			//Check for overflow.
			if (*this->count == this->capacity)
			{
				debugf(" Ring buffer is full.");					
			}
			else
			{
				(*this->count)++;
			}

			new_tail = *this->tail + this->item_size;
			//Handle wrap around.
			if (new_tail >= this->bytes)
			{
				debugf(" Reached the end of the buffer array.");
				//Go back to the start (remember to skip over the header).
				new_tail = this->header_bytes;
			}
			//Are we overwriting old entries.
			if (new_tail == *this->head)
			{
				//Advance head.
				(*this->head) = new_tail + this->item_size;
				//Handle wrap around.
				if ((*this->head) >= this->bytes)
				{
					*this->head = this->header_bytes;
				}
				debugf(" Overwriting head entry.");
			}
			debugf(" Adding at: %d.", *this->tail);
			os_memcpy(this->data + *this->tail, &time, this->item_size);
			*this->tail = new_tail;

			debugf(" Erasing sector 0x%x.", this->addr);
			if (SPI_FLASH_RESULT_OK != spi_flash_erase_sector(this->addr))
			{
				SYSTEM_ERROR("Erasing flash.");
			}
			debugf(" Saving to flash at 0x%x.", this->addr << 0xc);
			if (SPI_FLASH_RESULT_OK != spi_flash_write(this->addr << 0xc, (uint32_t *)this->data, this->bytes))
			{
				SYSTEM_ERROR("Writing flash.");
			}
			debugf(" Capacity: %d.", this->capacity);
		    debugf(" Items: %d.", *this->count);
			debugf(" Head: %d.", *this->head);
			debugf(" Tail: %d.", *this->tail);

			return(true);
		};
		/**
		 * @brief Get the next item in the buffer, and free it.
		 * 
		 * @return Time stored in the buffer.
		 */
		T popFront(void)
		{
			uint16_t new_head;
			T ret;
			
			Serial.println("Getting next item in ring buffer.");
			//Check for underflow.
			if (!*this->count)
			{
				Serial.println(" Buffer is empty.");
				return(0);
			}

			//Handle wrap around.
			new_head = (*this->head) + this->item_size;
			if (new_head > this->bytes)
			{
				Serial.println(" Reached the end of the buffer array.");
				//Go back to the start.
				new_head = this->header_bytes;
			}
			(*this->count)--;
			
			os_memcpy(&ret, this->data + (*this->head), this->item_size);
			//Point head at next item.
			(*this->head) = new_head;
			
			debugf(" Erasing sector 0x%x.", this->addr);
			if (SPI_FLASH_RESULT_OK != spi_flash_erase_sector(this->addr))
			{
				SYSTEM_ERROR("Erasing flash.");
			}
			debugf(" Saving to flash at 0x%x.", this->addr << 0xc);
			if (SPI_FLASH_RESULT_OK != spi_flash_write(this->addr << 0xc, (uint32_t *)this->data, this->bytes))
			{
				SYSTEM_ERROR("Writing flash.");
			}
			debugf(" Capacity: %d.", this->capacity);
		    debugf(" Items: %d.", *this->count);
			debugf(" Head: %d.", *this->head);
			debugf(" Tail: %d.", *this->tail);
			
			return(ret);
		}
	private:
		/**
		 * @brief Current number of items in the ring buffer.
		 */		
		uint16_t *count;
		/**
		 * @brief Pointer to the head index in the array.
		 */
		uint16_t *head;
		/**
		 * @brief Pointer to the tail index in the array.
		 */
		uint16_t *tail;
		/**
		 * @brief Size of an item.
		 */		
		static const uint16_t item_size = sizeof(T);
		/**
		 * @brief Capacity in bytes of the buffer.
		 */
		static const uint16_t bytes = 4096;
		/**
		 * @brief Bytes reserved for the header. 
		 */
		static const uint16_t header_bytes = sizeof(uint16_t) * 4;
		/**
		 * @brief Bytes left for data.
		 */
		static const uint16_t data_bytes = FlashLog::bytes - FlashLog::header_bytes;
		/**
		 * @brief Max number of items in the ring buffer.
		 */
		static const int16_t capacity = FlashLog::data_bytes / FlashLog::item_size;
		/**
		 * @brief Header signature, provides minimal safety that the data in the flash are not corrupted.
		 */
		static const uint16_t header_sig = 0xDBBD;
				/**
		 * @brief Pointer to the header signature.
		 */
		uint16_t *sig;
		/**
		 * @brief Actual data buffer.
		 * 
		 * *Must be 4 byte aligned for the SDk flash functions to work.*
		 */
		__attribute__((__aligned__(4))) uint8_t data[FlashLog::bytes];
		/**
		 * @brief Address of data in flash.
		 */
		uint32_t addr;
};

#endif //FLASHLOG_H
