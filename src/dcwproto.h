#ifndef DCWPROTO_H_INCLUDED
#define DCWPROTO_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char dcwmsg_macaddr_t[6];
typedef char          dcwmsg_ssid_t[32];

struct dcwmsg_sta_join {
  unsigned            data_macaddr_count;
  dcwmsg_macaddr_t    data_macaddrs[32];
};

struct dcwmsg_sta_unjoin {
  unsigned            data_macaddr_count;
  dcwmsg_macaddr_t    data_macaddrs[32];
};


struct dcwmsg_sta_ack {
  unsigned bonded_data_channel_count;
  struct {
    dcwmsg_macaddr_t macaddr;
    dcwmsg_ssid_t    ssid;
  } bonded_data_channels[32];
};

struct dcwmsg_sta_nack {
  unsigned            data_macaddr_count;
  dcwmsg_macaddr_t    data_macaddrs[32];
};



struct dcwmsg_ap_accept_sta {
  unsigned            data_ssid_count;
  dcwmsg_ssid_t       data_ssids[32];
};

struct dcwmsg_ap_reject_sta {
  unsigned            data_macaddr_count;
  dcwmsg_macaddr_t    data_macaddrs[32];
};




enum dcwmsg_id {
  DCWMSG_STA_JOIN          = 0x01,
  DCWMSG_STA_UNJOIN        = 0x02,
  DCWMSG_STA_ACK           = 0x11,
  DCWMSG_STA_NACK          = 0x12,
  DCWMSG_AP_ACCEPT_STA     = 0x21,
  DCWMSG_AP_REJECT_STA     = 0x22,
  DCWMSG_AP_ACK_DISCONNECT = 0x41,
  DCWMSG_AP_QUIT           = 0x99,
};


struct dcwmsg {
  enum dcwmsg_id id;
  union {
    struct dcwmsg_sta_join        sta_join;
    struct dcwmsg_sta_unjoin      sta_unjoin;
    struct dcwmsg_sta_ack         sta_ack;
    struct dcwmsg_sta_nack        sta_nack;
    struct dcwmsg_ap_accept_sta   ap_accept_sta;
    struct dcwmsg_ap_reject_sta   ap_reject_sta;
    /* DCWMSG_AP_ACK_DISCONNECT is empty */
    /* DCWMSG_AP_QUIT is empty */
  };
};

int dcwmsg_marshal(struct dcwmsg * const /* output */, const unsigned char * const /* buf */, const unsigned /* buf_len */);
unsigned dcwmsg_serialize(unsigned char * const /* buf */, const struct dcwmsg * const /* input */, const unsigned /* buf_len */);
void dcwmsg_dbgdump(const struct dcwmsg * const /* msg */);


#ifdef __cplusplus
}; //extern "C" {
#endif


#endif /* #ifndef DCWPROTO_H_INCLUDED */
