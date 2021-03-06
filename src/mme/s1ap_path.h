#ifndef __S1AP_PATH_H__
#define __S1AP_PATH_H__

#include "core_pkbuf.h"
#include "core_net.h"

#include "mme_context.h"
#include "s1ap_message.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CORE_DECLARE(status_t) s1ap_open();
CORE_DECLARE(status_t) s1ap_close();

CORE_DECLARE(status_t) s1ap_sctp_close(net_sock_t *sock);

CORE_DECLARE(status_t) s1ap_final();

#if 0 /* depreciated */
CORE_DECLARE(status_t) s1ap_send(net_sock_t *s, pkbuf_t *pkb);
#endif
CORE_DECLARE(status_t) s1ap_sendto(net_sock_t *s, pkbuf_t *pkb,
        c_uint32_t addr, c_uint16_t port);
CORE_DECLARE(status_t) s1ap_send_to_enb(mme_enb_t *enb, pkbuf_t *pkb);
CORE_DECLARE(status_t) s1ap_delayed_send_to_enb(mme_enb_t *enb,
        pkbuf_t *pkbuf, c_uint32_t duration);
CORE_DECLARE(status_t) s1ap_send_to_nas(
        enb_ue_t *enb_ue, S1ap_NAS_PDU_t *nasPdu);
CORE_DECLARE(status_t) s1ap_send_to_esm(mme_ue_t *mme_ue, pkbuf_t *esmbuf);

CORE_DECLARE(status_t) s1ap_send_initial_context_setup_request(
        mme_ue_t *mme_ue);
CORE_DECLARE(status_t) s1ap_send_ue_context_release_commmand(
        enb_ue_t *enb_ue, S1ap_Cause_t *cause, c_uint32_t delay);

CORE_DECLARE(status_t) s1ap_send_path_switch_ack(mme_ue_t *mme_ue);
CORE_DECLARE(status_t) s1ap_send_path_switch_failure(mme_enb_t *enb,
    c_uint32_t enb_ue_s1ap_id, c_uint32_t mme_ue_s1ap_id, S1ap_Cause_t *cause);

CORE_DECLARE(status_t) s1ap_send_handover_command(enb_ue_t *source_ue);
CORE_DECLARE(status_t) s1ap_send_handover_preparation_failure(
        enb_ue_t *source_ue, S1ap_Cause_t *cause);

CORE_DECLARE(status_t) s1ap_send_handover_request(
        mme_ue_t *mme_ue, S1ap_HandoverRequiredIEs_t *required);

CORE_DECLARE(status_t) s1ap_send_handover_cancel_ack(enb_ue_t *source_ue);

CORE_DECLARE(status_t) s1ap_send_mme_status_transfer(
        enb_ue_t *target_ue, S1ap_ENBStatusTransferIEs_t *ies);

int s1ap_recv_cb(net_sock_t *net_sock, void *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__S1_PATH_H__ */
