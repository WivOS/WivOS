#include "compositor.h"
#include "packetfs.h"

size_t compositor_client_send_wait(FILE *client, compositor_header_t *header, compositor_response_header_t **response, size_t size) {
	fwrite((const void *)header, size, 1, client);

	*response = (compositor_response_header_t *)malloc(1024);
    return server_client_read(client, (void *)*response, 1024);
}