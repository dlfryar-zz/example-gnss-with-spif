// Here's an example using the MX25R SPI flash device on the K82F
#include "mbed.h"
#include "SPIFBlockDevice.h"

// Create flash device on SPI bus with PTE5 as chip select
SPIFBlockDevice spif(MBED_CONF_SPIF_DRIVER_SPI_MOSI, MBED_CONF_SPIF_DRIVER_SPI_MISO, MBED_CONF_SPIF_DRIVER_SPI_CLK, MBED_CONF_SPIF_DRIVER_SPI_CS);

#define SLEEP_TIME 1

void thread_spi()
{
    printf("SPI: starting the filesystem thread\r\n");

    // Initialize the SPI flash device and print the memory layout
    spif.init();
    printf("SPI: spif size: %llu\r\n", spif.size());
    printf("SPI: spif read size: %llu\r\n", spif.get_read_size());
    printf("SPI: spif program size: %llu\r\n", spif.get_program_size());
    printf("SPI: spif erase size: %llu\r\n", spif.get_erase_size());

    // Write "Hello World!" to the first block
    char *buffer = (char *)malloc(spif.get_erase_size());
    sprintf(buffer, "Hello World!\r\n");
    spif.erase(0, spif.get_erase_size());
    spif.program(buffer, 0, spif.get_erase_size());

    while (true)
    {
        // Read back what was stored
        spif.read(buffer, 0, spif.get_erase_size());
        printf("SPI: %s", buffer);
        printf("SPI: sleeping for %d seconds\r\n", SLEEP_TIME);
        wait(SLEEP_TIME);
    }

    printf("SPI: THIS SHOULD NEVER HAPPEN!\r\n");
    // Deinitialize the device
    spif.deinit();
}
