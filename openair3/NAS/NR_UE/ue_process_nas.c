#include <openair2/LAYER2/NR_MAC_gNB/nr_mac_gNB.h>
#include <openair3/NAS/NR_UE/nr_user_def.h>


void SGSabortNet(void *msg, nr_user_nas_t *UE) {
}

void nas_schedule(void) {
}

/*
 *Message reception
 */

void SGSauthenticationReq(void *msg, nr_user_nas_t *UE) {
  authenticationrequestHeader_t *amsg=(authenticationrequestHeader_t *) msg;
  arrayCpy(UE->uicc->rand,amsg->RAND);
  arrayCpy(UE->uicc->autn,amsg->AUTN);
  nas_schedule();
}

void SGSidentityReq(void *msg, nr_user_nas_t *UE) {
  Identityrequest_t *idmsg=(Identityrequest_t *) msg;

  if (idmsg->it == SUCI ) {
    LOG_I(NAS,"Received Identity request, scheduling answer\n");
    nas_schedule();
  } else
    LOG_E(NAS,"Not developped: identity request for %d\n", idmsg->it);
}

void SGSsecurityModeCommand(void *msg, nr_user_nas_t *UE) {
}


void UEprocessNAS(void *msg,nr_user_nas_t *UE) {
  SGScommonHeader_t *header=(SGScommonHeader_t *) msg;

  if ( header->sh > 4 )
    SGSabortNet(msg, UE);
  else {
    switch  (header->epd) {
      case SGSmobilitymanagementmessages:
        switch (header->mt) {
          case Authenticationrequest:
            SGSauthenticationReq(msg, UE);
            break;

          case Identityrequest:
            SGSidentityReq(msg, UE);
            break;

          case Securitymodecommand:
            SGSsecurityModeCommand(msg, UE);
            break;

          default:
            SGSabortNet(msg, UE);
        }

        break;

      case SGSsessionmanagementmessages:
        SGSabortNet(msg, UE);
        break;

      default:
        SGSabortNet(msg, UE);
    }
  }
}

/*
 * Messages emission
 */

int identityResponse(void **msg, nr_user_nas_t *UE) {
  if (UE->uicc == NULL)
    // config file section hardcoded as "uicc", nevertheless it opens to manage several UEs or a multi SIM UE
    UE->uicc=init_uicc("uicc");

  // TS 24.501 9.11.3.4
  int imsiL=strlen(UE->uicc->imsiStr);
  int msinL=imsiL-3-UE->uicc->nmc_size;
  int respSize=sizeof(IdentityresponseIMSI_t) + (msinL+1)/2;
  IdentityresponseIMSI_t *resp=(IdentityresponseIMSI_t *) calloc(respSize,1);
  resp->common.epd=SGSmobilitymanagementmessages;
  resp->common.sh=0;
  resp->common.mt=Identityresponse;
  resp->common.len=htons(respSize-sizeof(Identityresponse_t));
  resp->mi=SUCI;
  resp->mcc1=UE->uicc->imsiStr[0]-'0';
  resp->mcc2=UE->uicc->imsiStr[1]-'0';
  resp->mcc3=UE->uicc->imsiStr[2]-'0';
  resp->mnc1=UE->uicc->imsiStr[3]-'0';
  resp->mnc2=UE->uicc->imsiStr[4]-'0';
  resp->mnc3=UE->uicc->nmc_size==2? 0xF : UE->uicc->imsiStr[3]-'0';
  // TBD: routing to fill (FF ?)
  char *out=(char *)(resp+1);
  char *ptr=UE->uicc->imsiStr + 3 + UE->uicc->nmc_size;

  while ( ptr < UE->uicc->imsiStr+strlen(UE->uicc->imsiStr) ) {
    *out=((*(ptr+1)-'0')<<4) | (*(ptr) -'0');
    out++;
    ptr+=2;
  }

  if (msinL%2 == 1)
    *out=((*(ptr-1)-'0')) | 0xF0;

  *msg=resp;
  return respSize;
}

int authenticationResponse(void **msg,nr_user_nas_t *UE) {
  if (UE->uicc == NULL)
    // config file section hardcoded as "uicc", nevertheless it opens to manage several UEs or a multi SIM UE
    UE->uicc=init_uicc("uicc");

  myCalloc(resp, authenticationresponse_t);
  resp->epd=SGSmobilitymanagementmessages;
  resp->sh=0;
  resp->mt=Authenticationresponse;
  resp->iei=IEI_AuthenticationResponse;
  resp->RESlen=sizeof(resp->RES);
  // Verify the AUTN
  uint8_t ik[16], ck[16], res[8], AUTN[16];
  milenage_generate(UE->uicc->opc, UE->uicc->amf, UE->uicc->key,
                    UE->uicc->sqn, UE->uicc->rand, AUTN, ik, ck, res);

  if ( memcmp(UE-uicc->autn, AUTN ) {
} else {
}
*msg=resp;
     return sizeof(authenticationresponse_t);
}

int securityModeComplete(void **msg, nr_user_nas_t *UE) {
  return -1;
}

int registrationComplete(void **msg, nr_user_nas_t *UE) {
  return -1;
}
