/**
 *  Copyright Notice:
 *  Copyright 2021-2022 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/spdm-emu/blob/main/LICENSE.md
 **/

#include "spdm_responder_emu.h"

uint32_t m_command;

SOCKET m_server_socket;

extern void *m_spdm_context;
#if LIBSPDM_FIPS_MODE
extern void *m_fips_selftest_context;
#endif /*LIBSPDM_FIPS_MODE*/
extern void *m_scratch_buffer;

void *spdm_server_init(void);


bool platform_server(const SOCKET socket)
{
    libspdm_return_t status;

    while (true) {
        status = libspdm_responder_dispatch_message(m_spdm_context);
        if (status != LIBSPDM_STATUS_UNSUPPORTED_CAP) {
            continue;
        }
    }
}

bool platform_server_routine_mctp_kernel(uint16_t port_number)
{
    SOCKET responder_socket;
    bool result;
    bool continue_serving;

    if (m_use_eid == 0 || m_use_net == 0) {
        printf("Platform MCTP kernel require to have valid EID and NET. eid = %d, net = %d\n",
               m_use_eid, m_use_net);
        return false;
    }

    result = create_socket_mctp_kernel(&responder_socket, m_use_eid, m_use_net);
    if (!result) {
        printf("Create platform service socket fail\n");
        return false;
    }
    m_server_socket = responder_socket;

    do {
        continue_serving = platform_server(m_server_socket);
        closesocket(m_server_socket);
    } while (continue_serving);

    closesocket(responder_socket);
    return true;
}

int main(int argc, char *argv[])
{
    bool result;

    printf("%s version 0.1\n", "spdm_responder_emu");
    srand((unsigned int)time(NULL));

    process_args("spdm_responder_emu", argc, argv);

    m_spdm_context = spdm_server_init();
    if (m_spdm_context == NULL) {
        return 1;
    }


    result = platform_server_routine_mctp_kernel(0);

    if (m_spdm_context != NULL) {
#if LIBSPDM_FIPS_MODE
        if (!libspdm_export_fips_selftest_context_from_spdm_context(
            m_spdm_context, m_fips_selftest_context,
            libspdm_get_fips_selftest_context_size())) {
            return 1;
        }
#endif /*LIBSPDM_FIPS_MODE*/
        libspdm_deinit_context(m_spdm_context);
        free(m_spdm_context);
        free(m_scratch_buffer);
    }

    printf("Server stopped\n");

    close_pcap_packet_file();
    return (!result);
}
