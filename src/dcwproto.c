/*
* Copyright (c) 2018 Cable Television Laboratories, Inc. ("CableLabs")
*                    and others.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Created by Jon Dennis (j.dennis@cablelabs.com)
*/




#ifdef WIN32
#define bzero(ptr, size) memset(ptr, 0, size)
#else
#include <config.h>
#include <strings.h>
#endif
#include <dcwproto.h>

#include <string.h>
#include <stdio.h>

#define ELEMENT_COUNT(ARR) (sizeof(ARR) / sizeof(ARR[0]))



/*

  Marshaling functions begin here...

*/
static int
dcwmsg_marshal_sta_join(struct dcwmsg_sta_join * const output, const unsigned char * const buf, const unsigned buf_len) {
  unsigned copy_size;

  if (buf_len < 1) return 0; /* marshal failed */

  output->data_macaddr_count = (unsigned)buf[0];
  if (output->data_macaddr_count > ELEMENT_COUNT(output->data_macaddrs)) {
    return 0; /* data_macaddr_count too big */
  }

  copy_size = output->data_macaddr_count * 6;
  if (buf_len < copy_size) {
    return 0; /* not enought input data... */
  }
  memcpy(output->data_macaddrs, &buf[1], copy_size);
  
  return 1; /* success */
}

static int
dcwmsg_marshal_sta_unjoin(struct dcwmsg_sta_unjoin * const output, const unsigned char * const buf, const unsigned buf_len) {
  return dcwmsg_marshal_sta_join((void*)output, buf, buf_len); /* this is ugly, but DRY... */
}

static int
dcwmsg_marshal_sta_ack(struct dcwmsg_sta_ack * const output, const unsigned char * buf, unsigned buf_len) {
  unsigned copy_size;
  unsigned i;

  /* read in the bonded data channel count */
  if (buf_len < 1) return 0;
  output->bonded_data_channel_count = (unsigned)*buf;
  buf_len--;
  buf++;
  if (output->bonded_data_channel_count > ELEMENT_COUNT(output->bonded_data_channels)) {
    return 0; /* bonded_data_channel_count too big */
  }

  for (i = 0; i < output->bonded_data_channel_count; i++) {
    /* read in the data mac address */
    if (buf_len < sizeof(output->bonded_data_channels[i].macaddr)) return 0;
    memcpy(output->bonded_data_channels[i].macaddr, buf, sizeof(output->bonded_data_channels[i].macaddr));
    buf_len -= sizeof(output->bonded_data_channels[i].macaddr);
    buf += sizeof(output->bonded_data_channels[i].macaddr);

    /* read in the data ssid string length */
    if (buf_len < 1) return 0;
    copy_size = (unsigned)*buf;
    buf_len--;
    buf++;
    if (copy_size > sizeof(output->bonded_data_channels[i].ssid)) return 0;

    /* copy in the data ssid string bytes */
    if (buf_len < copy_size) return 0;
    bzero(output->bonded_data_channels[i].ssid, sizeof(output->bonded_data_channels[i].ssid));
    memcpy(output->bonded_data_channels[i].ssid, buf, copy_size);
    buf_len -= copy_size;
    buf += copy_size;
  }

  return 1; /* success */
}

static int
dcwmsg_marshal_sta_nack(struct dcwmsg_sta_nack * const output, const unsigned char * const buf, const unsigned buf_len) {
  return dcwmsg_marshal_sta_join((void*)output, buf, buf_len); /* this is ugly, but DRY... */
}

static int
dcwmsg_marshal_ap_accept_sta(struct dcwmsg_ap_accept_sta * const output, const unsigned char * buf, unsigned buf_len) {
  unsigned copy_size;
  unsigned i;

  /* read in the data ssid count */
  if (buf_len < 1) return 0;
  output->data_ssid_count = (unsigned)*buf;
  buf_len--;
  buf++;
  if (output->data_ssid_count > ELEMENT_COUNT(output->data_ssids)) {
    return 0; /* data_ssid_count too big */
  }

  /* copy in the data SSIDs one-by-one */
  for (i = 0; i < output->data_ssid_count; i++) {
    /* read in the data ssid string length */
    if (buf_len < 1) return 0;
    copy_size = (unsigned)*buf;
    buf_len--;
    buf++;
    if (copy_size > sizeof(output->data_ssids[i])) return 0;

    /* copy in the data ssid string bytes */
    if (buf_len < copy_size) return 0;
    bzero(output->data_ssids[i], sizeof(output->data_ssids[i]));
    memcpy(output->data_ssids[i], buf, copy_size);
    buf_len -= copy_size;
    buf += copy_size;
  }

  return 1; /* success */
}

static int
dcwmsg_marshal_ap_reject_sta(struct dcwmsg_ap_reject_sta * const output, const unsigned char *buf, unsigned buf_len) {
  unsigned copy_size;

  /* read in the data mac address count */
  if (buf_len < 1) return 0;
  output->data_macaddr_count = (unsigned)*buf;
  buf_len--;
  buf++;
  if (output->data_macaddr_count > ELEMENT_COUNT(output->data_macaddrs)) {
    return 0; /* data_macaddr_count too big */
  }

  /* copy in all data mac addresses */
  copy_size = output->data_macaddr_count * 6;
  if (buf_len < copy_size) return 0;
  memcpy(output->data_macaddrs, buf, copy_size);
  buf_len -= copy_size;
  buf += copy_size;

  return 1; /* success */
}

int WIN32_EXPORT
dcwmsg_marshal(struct dcwmsg * const output, const unsigned char * const buf, const unsigned buf_len) {
  if (buf_len < 1) return 0; /* marshal failed */
  switch( output->id = (enum dcwmsg_id)buf[0] ) {
  case DCWMSG_STA_JOIN:           return dcwmsg_marshal_sta_join(&output->sta_join, buf + 1, buf_len - 1);
  case DCWMSG_STA_UNJOIN:         return dcwmsg_marshal_sta_unjoin(&output->sta_unjoin, buf + 1, buf_len - 1);
  case DCWMSG_STA_ACK:            return dcwmsg_marshal_sta_ack(&output->sta_ack, buf + 1, buf_len - 1);
  case DCWMSG_STA_NACK:           return dcwmsg_marshal_sta_nack(&output->sta_nack, buf + 1, buf_len - 1);
  case DCWMSG_AP_ACCEPT_STA:      return dcwmsg_marshal_ap_accept_sta(&output->ap_accept_sta, buf + 1, buf_len - 1);
  case DCWMSG_AP_REJECT_STA:      return dcwmsg_marshal_ap_reject_sta(&output->ap_reject_sta, buf + 1, buf_len - 1);
  case DCWMSG_AP_ACK_DISCONNECT:  return 1; /* this message is ID only */
  case DCWMSG_AP_QUIT:            return 1; /* this message is ID only */
  default:
    return 0; /* marshal failed */
  }
  return 0; /* shouldnt get here... */
}








/*

  Serialization functions begin here...

*/
static unsigned
dcwmsg_serialize_sta_join(unsigned char * const buf, const struct dcwmsg_sta_join * const input, const unsigned buf_len) {
  unsigned bytes_serialized;

  /* validate input fields */
  if (input->data_macaddr_count > ELEMENT_COUNT(input->data_macaddrs)) {
    return 0;
  }

  /* validate output size... */
  bytes_serialized = (input->data_macaddr_count * 6) + 1;
  if (buf_len < bytes_serialized) {
    return 0;
  }

  /* write to the output buffer... */
  buf[0] = (unsigned)input->data_macaddr_count;
  memcpy(&buf[1], input->data_macaddrs, bytes_serialized - 1);

  return bytes_serialized;
}

static unsigned
dcwmsg_serialize_sta_unjoin(unsigned char * const buf, const struct dcwmsg_sta_unjoin * const input, const unsigned buf_len) {
  return dcwmsg_serialize_sta_join(buf, (const void *)input, buf_len); /* this is ugly, but DRY... */
}

static unsigned
dcwmsg_serialize_sta_ack(unsigned char *buf, const struct dcwmsg_sta_ack * const input, const unsigned buf_len) {
  unsigned copy_size;
  unsigned buf_remaining;
  unsigned i;

  buf_remaining = buf_len;

  /* validate input fields */
  if (input->bonded_data_channel_count > ELEMENT_COUNT(input->bonded_data_channels)) {
    return 0;
  }

  /* copy out the bondeded data channel count */
  if (buf_remaining < 1) return 0;
  (*buf) = input->bonded_data_channel_count;
  buf_remaining--;
  buf++;

  for (i = 0; i < input->bonded_data_channel_count; i++) {
    /* copy out the data mac address */
    if (buf_remaining < sizeof(input->bonded_data_channels[i].macaddr)) return 0;
    memcpy(buf, input->bonded_data_channels[i].macaddr, sizeof(input->bonded_data_channels[i].macaddr));
    buf_remaining -= sizeof(input->bonded_data_channels[i].macaddr);
    buf += sizeof(input->bonded_data_channels[i].macaddr);

    /* copy out the SSID string length */
    if (buf_remaining < 1) return 0;
    copy_size = strnlen(input->bonded_data_channels[i].ssid, sizeof(input->bonded_data_channels[i].ssid));
    (*buf) = copy_size;
    buf_remaining--;
    buf++;

    /* copy out the SSID string contents */
    if (buf_remaining < copy_size) return 0;
    memcpy(buf, input->bonded_data_channels[i].ssid, copy_size);
    buf_remaining -= copy_size;
    buf += copy_size;
  }

  return buf_len - buf_remaining;
}

static unsigned
dcwmsg_serialize_sta_nack(unsigned char * const buf, const struct dcwmsg_sta_nack * const input, const unsigned buf_len) {
  return dcwmsg_serialize_sta_join(buf, (const void *)input, buf_len); /* this is ugly, but DRY... */
}

static unsigned
dcwmsg_serialize_ap_accept_sta(unsigned char *buf, const struct dcwmsg_ap_accept_sta * const input, const unsigned buf_len) {
  unsigned copy_size;
  unsigned buf_remaining;
  unsigned i;

  buf_remaining = buf_len;

  /* validate input fields */
  if (input->data_ssid_count > ELEMENT_COUNT(input->data_ssids)) {
    return 0;
  }

  /* copy out the data ssid count */
  if (buf_remaining < 1) return 0;
  (*buf) = input->data_ssid_count;
  buf_remaining--;
  buf++;

  for (i = 0; i < input->data_ssid_count; i++) {
    /* copy out the SSID string length */
    if (buf_remaining < 1) return 0;
    copy_size = strnlen(input->data_ssids[i], sizeof(input->data_ssids[i]));
    (*buf) = copy_size;
    buf_remaining--;
    buf++;

    /* copy out the SSID string contents */
    if (buf_remaining < copy_size) return 0;
    memcpy(buf, input->data_ssids[i], copy_size);
    buf_remaining -= copy_size;
    buf += copy_size;
  }

  return buf_len - buf_remaining;
}

static unsigned
dcwmsg_serialize_ap_reject_sta(unsigned char *buf, const struct dcwmsg_ap_reject_sta * const input, const unsigned buf_len) {
  unsigned copy_size;
  unsigned buf_remaining;

  buf_remaining = buf_len;

  /* validate input fields */
  if (input->data_macaddr_count > ELEMENT_COUNT(input->data_macaddrs)) {
    return 0;
  }

  /* copy out the data mac address count */
  if (buf_remaining < 1) return 0;
  (*buf) = input->data_macaddr_count;
  buf_remaining--;
  buf++;

  /* copy out the data mac addresses */
  copy_size = input->data_macaddr_count * 6;
  if (buf_remaining < copy_size) return 0;
  memcpy(buf, input->data_macaddrs, copy_size);
  buf_remaining -= copy_size;
  buf++;

  return buf_len - buf_remaining;
}

unsigned WIN32_EXPORT
dcwmsg_serialize(unsigned char * const buf, const struct dcwmsg * const input, const unsigned buf_len) {
  unsigned rv;

  if (buf_len < 1) return 0; /* serialize failed */
  buf[0] = (unsigned char)(unsigned)input->id;

  switch(input->id) {
  case DCWMSG_STA_JOIN:
    rv = dcwmsg_serialize_sta_join(buf + 1, &input->sta_join, buf_len - 1);
    break;
  case DCWMSG_STA_UNJOIN:
    rv = dcwmsg_serialize_sta_unjoin(buf + 1, &input->sta_unjoin, buf_len - 1);
    break;
  case DCWMSG_STA_ACK:
    rv = dcwmsg_serialize_sta_ack(buf + 1, &input->sta_ack, buf_len - 1);
    break;
  case DCWMSG_STA_NACK:
    rv = dcwmsg_serialize_sta_nack(buf + 1, &input->sta_nack, buf_len - 1);
    break;
  case DCWMSG_AP_ACCEPT_STA:
    rv = dcwmsg_serialize_ap_accept_sta(buf + 1, &input->ap_accept_sta, buf_len - 1);
    break;
  case DCWMSG_AP_REJECT_STA:
    rv = dcwmsg_serialize_ap_reject_sta(buf + 1, &input->ap_reject_sta, buf_len - 1);
    break;
  case DCWMSG_AP_ACK_DISCONNECT:
    return 1; /* this message is id only */
  case DCWMSG_AP_QUIT:
    return 1; /* this message is id only */
  default:
    rv = 0;
    break;
  }

  return (rv == 0) ? 0 : (rv + 1);
}







/*

  Diagnostic functions begin here...

*/
#include <stdio.h>
static inline void
fprintf_macaddr(FILE * const f, const unsigned char * const macaddr) {
  fprintf(f, "%02X-%02X-%02X-%02X-%02X-%02X",
          (unsigned)macaddr[0],
          (unsigned)macaddr[1],
          (unsigned)macaddr[2],
          (unsigned)macaddr[3],
          (unsigned)macaddr[4],
          (unsigned)macaddr[5]
  );
}

void WIN32_EXPORT
dcwmsg_dbgdump(const struct dcwmsg * const msg) {

  FILE *f;
  unsigned i;

  f = stderr;

  fprintf(f, "DCW Message Dump:\n");

  if (msg == NULL) {
    fprintf(f, "  (NULL)\n");
    return;
  }

  switch (msg->id) {
  case DCWMSG_STA_JOIN:
    fprintf(f, "  Type: Station Join\n");
    fprintf(f, "  Data Channel MAC Address Count: %u\n", msg->sta_join.data_macaddr_count);
    if (msg->sta_join.data_macaddr_count > ELEMENT_COUNT(msg->sta_join.data_macaddrs)) {
      fprintf(f, "  (Count too high)\n");
      return;
    }
    fprintf(f, "  Data Channel MAC Address:\n");
    for (i = 0; i < msg->sta_join.data_macaddr_count; i++) {
      fprintf(f, "    . ");
      fprintf_macaddr(f, msg->sta_join.data_macaddrs[i]);
      fprintf(f, "\n");
    }
    break;

  case DCWMSG_STA_UNJOIN:
    fprintf(f, "  Type: Station Unjoin\n");
    fprintf(f, "  Data Channel MAC Address Count: %u\n", msg->sta_unjoin.data_macaddr_count);
    if (msg->sta_unjoin.data_macaddr_count > ELEMENT_COUNT(msg->sta_unjoin.data_macaddrs)) {
      fprintf(f, "  (Count too high)\n");
      return;
    }
    fprintf(f, "  Data Channel MAC Address:\n");
    for (i = 0; i < msg->sta_unjoin.data_macaddr_count; i++) {
      fprintf(f, "    . ");
      fprintf_macaddr(f, msg->sta_unjoin.data_macaddrs[i]);
      fprintf(f, "\n");
    }
    break;

  case DCWMSG_STA_ACK:
    fprintf(f, "  Type: Station Acknowledgement\n");
    fprintf(f, "  Bonded Data Channel Count: %u\n", msg->sta_ack.bonded_data_channel_count);
    if (msg->sta_ack.bonded_data_channel_count > ELEMENT_COUNT(msg->sta_ack.bonded_data_channels)) {
      fprintf(f, "  (Count too high)\n");
      return;
    }
    fprintf(f, "  Bonded Data Channels:\n");
    for (i = 0; i < msg->sta_ack.bonded_data_channel_count; i++) {
      fprintf(f, "    . ");
      fprintf_macaddr(f, msg->sta_ack.bonded_data_channels[i].macaddr);
      fprintf(f, " -> '%.*s'\n", (int)sizeof(msg->sta_ack.bonded_data_channels[i].ssid), msg->sta_ack.bonded_data_channels[i].ssid);
    }
    break;

  case DCWMSG_STA_NACK:
    fprintf(f, "  Type: Station Negative Acknowledgement\n");
    fprintf(f, "  Data Channel MAC Address Count: %u\n", msg->sta_nack.data_macaddr_count);
    if (msg->sta_nack.data_macaddr_count > ELEMENT_COUNT(msg->sta_nack.data_macaddrs)) {
      fprintf(f, "  (Count too high)\n");
      return;
    }
    fprintf(f, "  Data Channel MAC Address:\n");
    for (i = 0; i < msg->sta_nack.data_macaddr_count; i++) {
      fprintf(f, "    . ");
      fprintf_macaddr(f, msg->sta_nack.data_macaddrs[i]);
      fprintf(f, "\n");
    }
    break;

  case DCWMSG_AP_ACCEPT_STA:
    fprintf(f, "  Type: AP Accept Station\n");
    fprintf(f, "  Data Channel SSID Count: %u\n", msg->ap_accept_sta.data_ssid_count);
    if (msg->ap_accept_sta.data_ssid_count > ELEMENT_COUNT(msg->ap_accept_sta.data_ssids)) {
      fprintf(f, "  (Count too high)\n");
      return;
    }
    fprintf(f, "  Data Channel SSIDs:\n");
    for (i = 0; i < msg->ap_accept_sta.data_ssid_count; i++) {
      fprintf(f, "    . %.*s\n", (int)sizeof(msg->ap_accept_sta.data_ssids[i]), msg->ap_accept_sta.data_ssids[i]);
    }
    break;

  case DCWMSG_AP_REJECT_STA:
    fprintf(f, "  Type: AP Reject Station\n");
    fprintf(f, "  Data Channel MAC Address Count: %u\n", msg->ap_reject_sta.data_macaddr_count);
    if (msg->ap_reject_sta.data_macaddr_count > ELEMENT_COUNT(msg->ap_reject_sta.data_macaddrs)) {
      fprintf(f, "  (Count too high)\n");
      return;
    }
    fprintf(f, "  Data Channel MAC Addresses:\n");
    for (i = 0; i < msg->ap_reject_sta.data_macaddr_count; i++) {
      fprintf(f, "    . ");
      fprintf_macaddr(f, msg->ap_reject_sta.data_macaddrs[i]);
      fprintf(f, "\n");
    }
    break;

    case DCWMSG_AP_ACK_DISCONNECT:
      fprintf(f, "  Type: AP ACK Station Disconnect\n");
      break;

    case DCWMSG_AP_QUIT:
      fprintf(f, "  Type: AP Quit\n");
      break;


  default:
    fprintf(f, "  Type: Unknown\n");
    break;
  }
}

