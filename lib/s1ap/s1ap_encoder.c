#define TRACE_MODULE _s1ap_send

#include "core_debug.h"
#include "core_param.h"

#include "types.h"
#include "s1ap_message.h"

static inline int s1ap_encode_initiating_message(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_successfull_outcome(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_unsuccessfull_outcome(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);

static inline int s1ap_encode_s1setup_request(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_s1setup_response(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_s1setup_failure(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_downlink_nas_transport(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_initial_context_setup_request(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_e_rab_setup_request(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_e_rab_release_command(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_ue_context_release_command(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_paging(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_path_switch_ack(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_path_switch_failure(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_handover_request(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_handover_command(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_handover_cancel_ack(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_handover_preparation_failure(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);
static inline int s1ap_encode_mme_status_transfer(
    s1ap_message_t *message_p, pkbuf_t *pkbuf);

static void s1ap_encode_xer_print_message(
    asn_enc_rval_t (*func)(asn_app_consume_bytes_f *cb,
    void *app_key, s1ap_message_t *message_p), 
    asn_app_consume_bytes_f *cb, s1ap_message_t *message_p);

int s1ap_encode_pdu(pkbuf_t **pkb, s1ap_message_t *message_p)
{
    int encoded = -1;

    d_assert (message_p, return -1, "Null param");

    *pkb = pkbuf_alloc(0, MAX_SDU_LEN);
    d_assert(*pkb, return -1, "Null Param");

    switch (message_p->direction) 
    {
        case S1AP_PDU_PR_initiatingMessage:
            encoded = s1ap_encode_initiating_message(message_p, *pkb);
            break;

        case S1AP_PDU_PR_successfulOutcome:
            encoded = s1ap_encode_successfull_outcome(message_p, *pkb);
            break;

        case S1AP_PDU_PR_unsuccessfulOutcome:
            encoded = s1ap_encode_unsuccessfull_outcome(message_p, *pkb);
            break;

        default:
            d_warn("Unknown message outcome (%d) or not implemented", 
                    (int)message_p->direction);
            pkbuf_free(*pkb);
            return -1;
    }

    if (encoded < 0)
    {
        pkbuf_free(*pkb);
        return -1;
    }

    (*pkb)->len = (encoded >> 3);

    return encoded;
}

static inline int s1ap_encode_initiating_message(
    s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    int ret = -1;
    switch (message_p->procedureCode) 
    {
        case S1ap_ProcedureCode_id_S1Setup:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_s1setuprequest, 
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_s1setup_request(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_downlinkNASTransport:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_downlinknastransport, 
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_downlink_nas_transport(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_InitialContextSetup:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_initialcontextsetuprequest, 
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_initial_context_setup_request(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_E_RABSetup:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_e_rabsetuprequest, 
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_e_rab_setup_request(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_E_RABRelease:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_e_rabreleasecommand, 
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_e_rab_release_command(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_UEContextRelease:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_uecontextreleasecommand, 
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_ue_context_release_command(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_Paging:
            s1ap_encode_xer_print_message(s1ap_xer_print_s1ap_paging,
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_paging(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_HandoverResourceAllocation:
            s1ap_encode_xer_print_message(s1ap_xer_print_s1ap_handoverrequest,
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_handover_request(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_MMEStatusTransfer:
            s1ap_encode_xer_print_message(s1ap_xer_print_s1ap_mmestatustransfer,
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_mme_status_transfer(message_p, pkbuf);
            break;

        default:
            d_warn("Unknown procedure ID (%d) for initiating message_p\n", 
                    (int)message_p->procedureCode);
            break;
    }

    return ret;
}

static inline int s1ap_encode_successfull_outcome(
    s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    int ret = -1;
    switch (message_p->procedureCode) 
    {
        case S1ap_ProcedureCode_id_S1Setup:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_s1setupresponse, 
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_s1setup_response(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_PathSwitchRequest:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_pathswitchrequestacknowledge,
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_path_switch_ack(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_HandoverPreparation:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_handovercommand,
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_handover_command(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_HandoverCancel:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_handovercancelacknowledge,
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_handover_cancel_ack(message_p, pkbuf);
            break;

        default:
            d_warn("Unknown procedure ID (%d) for successfull "
                    "outcome message\n", (int)message_p->procedureCode);
        break;
    }

    return ret;
}

static inline int s1ap_encode_unsuccessfull_outcome(
    s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    int ret = -1;
    switch (message_p->procedureCode) 
    {
        case S1ap_ProcedureCode_id_S1Setup:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_s1setupfailure, 
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_s1setup_failure(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_PathSwitchRequest:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_pathswitchrequestfailure,
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_path_switch_failure(message_p, pkbuf);
            break;

        case S1ap_ProcedureCode_id_HandoverPreparation:
            s1ap_encode_xer_print_message(
                    s1ap_xer_print_s1ap_handoverpreparationfailure,
                    s1ap_xer__print2sp, message_p);
            ret = s1ap_encode_handover_preparation_failure(message_p, pkbuf);
            break;

        default:
            d_warn("Unknown procedure ID (%d) for unsuccessfull "
                    "outcome message\n", (int)message_p->procedureCode);
        break;
    }

    return ret;
}

static inline int s1ap_encode_s1setup_request(
        s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_S1SetupRequest_t s1SetupRequest;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_S1SetupRequest;

    memset(&s1SetupRequest, 0, sizeof(s1SetupRequest));
    if (s1ap_encode_s1ap_s1setuprequesties(
            &s1SetupRequest, &message_p->s1ap_S1SetupRequestIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = message_p->procedureCode;
    pdu.choice.initiatingMessage.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.initiatingMessage.value, td, &s1SetupRequest);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &s1SetupRequest);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_s1setup_response(
        s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_S1SetupResponse_t s1SetupResponse;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_S1SetupResponse;

    memset(&s1SetupResponse, 0, sizeof (S1ap_S1SetupResponse_t));
    if (s1ap_encode_s1ap_s1setupresponseies(
            &s1SetupResponse, &message_p->s1ap_S1SetupResponseIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_successfulOutcome;
    pdu.choice.successfulOutcome.procedureCode = message_p->procedureCode;
    pdu.choice.successfulOutcome.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.successfulOutcome.value, 
            td, &s1SetupResponse);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &s1SetupResponse);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_s1setup_failure(
    s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_S1SetupFailure_t s1SetupFailure;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_S1SetupFailure;

    memset(&s1SetupFailure, 0, sizeof (S1ap_S1SetupFailure_t));
    if (s1ap_encode_s1ap_s1setupfailureies(
            &s1SetupFailure, &message_p->s1ap_S1SetupFailureIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_unsuccessfulOutcome;
    pdu.choice.unsuccessfulOutcome.procedureCode = message_p->procedureCode;
    pdu.choice.unsuccessfulOutcome.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.unsuccessfulOutcome.value, 
            td, &s1SetupFailure);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &s1SetupFailure);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_downlink_nas_transport(
        s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_DownlinkNASTransport_t downlinkNasTransport;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_DownlinkNASTransport;

    memset(&downlinkNasTransport, 0, sizeof(S1ap_DownlinkNASTransport_t));
    if (s1ap_encode_s1ap_downlinknastransport_ies(&downlinkNasTransport, 
            &message_p->s1ap_DownlinkNASTransport_IEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = message_p->procedureCode;
    pdu.choice.initiatingMessage.criticality = S1ap_Criticality_ignore;
    ANY_fromType_aper(&pdu.choice.initiatingMessage.value, 
            td, &downlinkNasTransport);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &downlinkNasTransport);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_initial_context_setup_request(
        s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_InitialContextSetupRequest_t initialContextSetupRequest;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_InitialContextSetupRequest;

    memset(&initialContextSetupRequest, 0, 
            sizeof(S1ap_InitialContextSetupRequest_t));
    if (s1ap_encode_s1ap_initialcontextsetuprequesties(
            &initialContextSetupRequest, 
            &message_p->s1ap_InitialContextSetupRequestIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = message_p->procedureCode;
    pdu.choice.initiatingMessage.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.initiatingMessage.value, 
            td, &initialContextSetupRequest);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &initialContextSetupRequest);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_e_rab_setup_request(
        s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_E_RABSetupRequest_t req;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_E_RABSetupRequest;

    memset(&req, 0, sizeof(S1ap_E_RABSetupRequest_t));
    if (s1ap_encode_s1ap_e_rabsetuprequesties(
            &req, &message_p->s1ap_E_RABSetupRequestIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = message_p->procedureCode;
    pdu.choice.initiatingMessage.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.initiatingMessage.value, td, &req);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &req);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_e_rab_release_command(
        s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_E_RABReleaseCommand_t req;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_E_RABReleaseCommand;

    memset(&req, 0, sizeof(S1ap_E_RABReleaseCommand_t));
    if (s1ap_encode_s1ap_e_rabreleasecommandies(
            &req, &message_p->s1ap_E_RABReleaseCommandIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = message_p->procedureCode;
    pdu.choice.initiatingMessage.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.initiatingMessage.value, td, &req);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &req);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_ue_context_release_command(
      s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_UEContextReleaseCommand_t ueContextReleaseCommand;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_UEContextReleaseCommand;

    memset(&ueContextReleaseCommand, 0, 
            sizeof(S1ap_UEContextReleaseCommand_t));
    if (s1ap_encode_s1ap_uecontextreleasecommand_ies(
            &ueContextReleaseCommand,
            &message_p->s1ap_UEContextReleaseCommand_IEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = message_p->procedureCode;
    pdu.choice.initiatingMessage.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.initiatingMessage.value, 
            td, &ueContextReleaseCommand);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &ueContextReleaseCommand);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_paging(s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_Paging_t paging;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_Paging;

    memset(&paging, 0, sizeof(S1ap_Paging_t));
    if (s1ap_encode_s1ap_pagingies(&paging, &message_p->s1ap_PagingIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = message_p->procedureCode;
    pdu.choice.initiatingMessage.criticality = S1ap_Criticality_ignore;
    ANY_fromType_aper(&pdu.choice.initiatingMessage.value, td, &paging);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &paging);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_path_switch_ack(
    s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_PathSwitchRequestAcknowledge_t ack;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_PathSwitchRequestAcknowledge;

    memset(&ack, 0, sizeof(S1ap_PathSwitchRequestAcknowledge_t));
    if (s1ap_encode_s1ap_pathswitchrequestacknowledgeies(
            &ack, &message_p->s1ap_PathSwitchRequestAcknowledgeIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_successfulOutcome;
    pdu.choice.successfulOutcome.procedureCode = message_p->procedureCode;
    pdu.choice.successfulOutcome.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.successfulOutcome.value, td, &ack);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &ack);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_path_switch_failure(
    s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_PathSwitchRequestFailure_t failure;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_PathSwitchRequestFailure;

    memset(&failure, 0, sizeof(S1ap_PathSwitchRequestFailure_t));
    if (s1ap_encode_s1ap_pathswitchrequestfailureies(
            &failure, &message_p->s1ap_PathSwitchRequestFailureIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_unsuccessfulOutcome;
    pdu.choice.unsuccessfulOutcome.procedureCode = message_p->procedureCode;
    pdu.choice.unsuccessfulOutcome.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.unsuccessfulOutcome.value, td, &failure);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &failure);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_handover_request(
    s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_HandoverRequest_t request;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_HandoverRequest;

    memset(&request, 0, sizeof(S1ap_HandoverRequest_t));
    if (s1ap_encode_s1ap_handoverrequesties(
                &request, &message_p->s1ap_HandoverRequestIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = message_p->procedureCode;
    pdu.choice.initiatingMessage.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.initiatingMessage.value, td, &request);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &request);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_handover_command(
  s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_HandoverCommand_t command;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_HandoverCommand;

    memset(&command, 0, sizeof(S1ap_HandoverCommand_t));
    if (s1ap_encode_s1ap_handovercommandies(
            &command, &message_p->s1ap_HandoverCommandIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_successfulOutcome;
    pdu.choice.successfulOutcome.procedureCode = message_p->procedureCode;
    pdu.choice.successfulOutcome.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.successfulOutcome.value, td, &command);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &command);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_handover_preparation_failure(
  s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_HandoverPreparationFailure_t failure;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_HandoverPreparationFailure;

    memset(&failure, 0, sizeof(S1ap_HandoverPreparationFailure_t));
    if (s1ap_encode_s1ap_handoverpreparationfailureies(
            &failure, &message_p->s1ap_HandoverPreparationFailureIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_unsuccessfulOutcome;
    pdu.choice.unsuccessfulOutcome.procedureCode = message_p->procedureCode;
    pdu.choice.unsuccessfulOutcome.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.unsuccessfulOutcome.value, td, &failure);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &failure);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_handover_cancel_ack(
  s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_HandoverCancelAcknowledge_t ack;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_HandoverCommand;

    memset(&ack, 0, sizeof(S1ap_HandoverCommand_t));
    if (s1ap_encode_s1ap_handovercancelacknowledgeies(
            &ack, &message_p->s1ap_HandoverCancelAcknowledgeIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_successfulOutcome;
    pdu.choice.successfulOutcome.procedureCode = message_p->procedureCode;
    pdu.choice.successfulOutcome.criticality = S1ap_Criticality_reject;
    ANY_fromType_aper(&pdu.choice.successfulOutcome.value, td, &ack);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &ack);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static inline int s1ap_encode_mme_status_transfer(
    s1ap_message_t *message_p, pkbuf_t *pkbuf)
{
    asn_enc_rval_t enc_ret = {0};

    S1AP_PDU_t pdu;
    S1ap_MMEStatusTransfer_t status;
    asn_TYPE_descriptor_t *td = &asn_DEF_S1ap_MMEStatusTransfer;

    memset(&status, 0, sizeof(S1ap_MMEStatusTransfer_t));
    if (s1ap_encode_s1ap_mmestatustransferies(
                &status, &message_p->s1ap_MMEStatusTransferIEs) < 0) 
    {
        d_error("Encoding of %s failed", td->name);
        return -1;
    }

    memset(&pdu, 0, sizeof (S1AP_PDU_t));
    pdu.present = S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = message_p->procedureCode;
    pdu.choice.initiatingMessage.criticality = S1ap_Criticality_ignore;
    ANY_fromType_aper(&pdu.choice.initiatingMessage.value, td, &status);

    enc_ret = aper_encode_to_buffer(&asn_DEF_S1AP_PDU, 
                    &pdu, pkbuf->payload, MAX_SDU_LEN);

    ASN_STRUCT_FREE_CONTENTS_ONLY(*td, &status);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_S1AP_PDU, &pdu);

    if (enc_ret.encoded < 0)
    {
        d_error("Encoding of %s failed", td->name);
    }

    return enc_ret.encoded;
}

static void s1ap_encode_xer_print_message(
    asn_enc_rval_t (*func)(asn_app_consume_bytes_f *cb,
    void *app_key, s1ap_message_t *message_p), 
    asn_app_consume_bytes_f *cb, s1ap_message_t *message_p)
{
    if (g_trace_mask && TRACE_MODULE >= 5)
    {
        char *message_string = core_calloc(HUGE_STRING_LEN, sizeof(c_uint8_t));
        s1ap_string_total_size = 0;

        func(cb, message_string, message_p);

        printf("%s\n", message_string);

        core_free(message_string);
    }
}

